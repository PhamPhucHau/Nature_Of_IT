/*
 * heap_stack_model.c  —  JVM Heap, Stack, and Memory Regions in C
 *
 * Build:  gcc -Wall -Wextra -o heap_stack_model heap_stack_model.c
 * Run:    ./heap_stack_model
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   The JVM memory layout: Heap (Young+Old), Stack (per-thread),
 *   and Metaspace — modeled entirely in C using structs and regions.
 *
 *   KEY TRUTH:
 *     Everything in C is explicit:    malloc() / free() / &variable
 *     Everything in Java is implicit: new Object() / GC / reference
 *     But they map 1:1 under the hood.
 *
 *   MAPPING TABLE:
 *     Java: new String("hi")         → C: malloc(sizeof(StringObj))
 *     Java: String s = "hi"          → C: StringObj *s = ptr  (on stack)
 *     Java: s goes out of scope      → C: ptr becomes unreachable
 *     Java: GC collects the object   → C: free(ptr)  (GC does this for you)
 *     Java: stack frame              → C: function stack frame (automatic)
 *     Java: int x = 5               → C: int x = 5  (same — on stack)
 *
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ═══════════════════════════════════════════════════════════
 *  JVM HEAP SIMULATION
 *
 *  Real JVM heap regions:
 *    Eden       → new objects born here (fast bump-pointer allocation)
 *    Survivor0  → survives first Minor GC
 *    Survivor1  → ping-pong with Survivor0
 *    Old Gen    → survived >15 GCs (tenured)
 *
 *  We simulate this as fixed-size memory pools.
 *  Real JVM uses a contiguous virtual memory range with
 *  a "top" pointer (eden) that bumps on each allocation.
 * ═══════════════════════════════════════════════════════════ */

#define EDEN_SIZE       (1024 * 64)    /* 64 KB Eden */
#define SURVIVOR_SIZE   (1024 * 8)     /*  8 KB each Survivor */
#define OLD_GEN_SIZE    (1024 * 128)   /* 128 KB Old Generation */
#define MAX_GC_AGE      15             /* promote to old after 15 GCs */

/* Simulated heap regions as byte arrays */
static unsigned char eden_space[EDEN_SIZE];
static unsigned char survivor0[SURVIVOR_SIZE];
static unsigned char survivor1[SURVIVOR_SIZE];
static unsigned char old_gen[OLD_GEN_SIZE];

/* "Top" pointer for bump-pointer allocation in Eden
 * Real JVM: Thread-Local Allocation Buffer (TLAB) per thread */
static size_t eden_top     = 0;
static size_t old_gen_top  = 0;
static int    minor_gc_count = 0;

/* ─────────────────────────────────────────────────────────────
 *  OBJECT HEADER — every Java object has this prefix
 *
 *  Real JVM object header (64-bit, compressed oops):
 *    Mark Word  (8 bytes): GC age, lock state, identity hash, GC mark
 *    Klass ptr  (4 bytes): points to class metadata in Metaspace
 *
 *  We simulate a simplified version.
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    unsigned int  mark_word;   /* GC age (bits 0-3), lock bits, mark bit */
    unsigned int  klass_id;    /* which class this object belongs to      */
    int           gc_age;      /* how many GCs this object survived        */
    int           reachable;   /* 1 = referenced, 0 = can be collected     */
    size_t        size;        /* total object size in bytes               */
    char          class_name[32]; /* debug: class name                     */
} ObjectHeader;

/* A "Java object" in our simulation = header + data */
typedef struct {
    ObjectHeader hdr;
    char         data[64];     /* simplified: all objects have 64-byte payload */
} SimObject;

/* ─────────────────────────────────────────────────────────────
 *  Eden allocation — bump pointer (fast, no lock needed per thread)
 *
 *  Java: new MyObject()
 *  JVM:  TLAB top += sizeof(MyObject); return old_top;
 *  C:    eden_space[eden_top .. eden_top+size] = new object
 *
 *  If Eden is full → trigger Minor GC
 * ──────────────────────────────────────────────────────────── */
static SimObject* eden_alloc(const char *class_name, size_t data_size) {
    size_t total = sizeof(SimObject);
    (void)data_size;  /* simplified: all same size */

    if (eden_top + total > EDEN_SIZE) {
        printf("  [Eden FULL] Would trigger Minor GC\n");
        return NULL;
    }

    SimObject *obj = (SimObject*)(eden_space + eden_top);
    memset(obj, 0, total);

    /* Initialize object header */
    obj->hdr.mark_word  = 0x01;   /* unlocked, age=0 */
    obj->hdr.klass_id   = 1;
    obj->hdr.gc_age     = 0;
    obj->hdr.reachable  = 1;
    obj->hdr.size       = total;
    strncpy(obj->hdr.class_name, class_name, 31);

    /* Write object data */
    snprintf(obj->data, sizeof(obj->data), "instance of %s @ %p",
             class_name, (void*)obj);

    eden_top += total;

    printf("  [Eden] Allocated %-20s @ %p  (eden_used=%zu/%d)\n",
           class_name, (void*)obj, eden_top, EDEN_SIZE);

    return obj;
}

/* ─────────────────────────────────────────────────────────────
 *  Old Gen allocation — for tenured objects
 *  Called when object survives MAX_GC_AGE Minor GCs
 * ──────────────────────────────────────────────────────────── */
static SimObject* old_gen_alloc(SimObject *young_obj) {
    size_t total = sizeof(SimObject);

    if (old_gen_top + total > OLD_GEN_SIZE) {
        printf("  [OldGen FULL] Would trigger FULL GC (stop-the-world)\n");
        return NULL;
    }

    SimObject *old_obj = (SimObject*)(old_gen + old_gen_top);
    *old_obj = *young_obj;   /* copy object to old gen */
    old_obj->hdr.gc_age = MAX_GC_AGE;
    old_gen_top += total;

    printf("  [OldGen] Promoted    %-20s @ %p  (age=%d, old_used=%zu/%d)\n",
           old_obj->hdr.class_name, (void*)old_obj,
           old_obj->hdr.gc_age, old_gen_top, OLD_GEN_SIZE);

    return old_obj;
}

/* ─────────────────────────────────────────────────────────────
 *  Simulate Minor GC (Young Generation collection)
 *  Real JVM G1 Minor GC:
 *    1. Stop all application threads (stop-the-world)
 *    2. Scan GC roots (stack, static fields, JNI)
 *    3. Mark reachable objects
 *    4. Copy live objects: Eden+S0 → S1 (or Old if age >= threshold)
 *    5. Clear Eden + S0
 *    6. Resume threads
 * ──────────────────────────────────────────────────────────── */
static void simulate_minor_gc(SimObject **roots, int root_count) {
    minor_gc_count++;
    printf("\n  ┌─ Minor GC #%d (stop-the-world) ─────────────────\n",
           minor_gc_count);
    printf("  │  Eden used before: %zu bytes\n", eden_top);

    int collected = 0, survived = 0, promoted = 0;

    /* Phase 1: Mark — walk from GC roots */
    /* (In our simulation, roots array IS the live set) */

    /* Phase 2: Copy live objects, age survivors */
    size_t new_eden_top = 0;

    /* Walk all objects currently in Eden */
    size_t offset = 0;
    while (offset + sizeof(SimObject) <= eden_top) {
        SimObject *obj = (SimObject*)(eden_space + offset);
        offset += sizeof(SimObject);

        /* Check if object is in roots (= reachable) */
        int is_live = 0;
        for (int r = 0; r < root_count; r++) {
            if (roots[r] == obj) { is_live = 1; break; }
        }

        if (!is_live) {
            /* Object is unreachable — it gets collected */
            printf("  │  COLLECT: %-20s (unreachable)\n",
                   obj->hdr.class_name);
            collected++;
            continue;
        }

        obj->hdr.gc_age++;

        if (obj->hdr.gc_age >= MAX_GC_AGE) {
            /* Promote to Old Generation */
            old_gen_alloc(obj);
            promoted++;
        } else {
            /* Copy to survivor space (bump survivor top) */
            /* In real JVM: copy to survivor space using pointer bump */
            survived++;
            printf("  │  SURVIVE: %-20s (age=%d)\n",
                   obj->hdr.class_name, obj->hdr.gc_age);
            (void)new_eden_top;
        }
    }

    /* Phase 3: Reset Eden (bulk free of all Eden bytes) */
    memset(eden_space, 0, eden_top);
    eden_top = 0;

    printf("  │  Collected: %d  Survived: %d  Promoted: %d\n",
           collected, survived, promoted);
    printf("  │  Eden used after:  %zu bytes\n", eden_top);
    printf("  └──────────────────────────────────────────────────\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  JVM STACK SIMULATION
 *
 *  Each thread has its own stack.
 *  Each method call creates a new "Stack Frame".
 *  Stack Frame contains:
 *    - Local variable table  (primitives stored directly, objects as refs)
 *    - Operand stack         (working area for bytecode evaluation)
 *    - Return address
 *    - Reference to class/method
 *
 *  KEY: The stack is where local variables live.
 *       Object references on the stack point to Heap objects.
 *       Primitives (int, double, etc.) live directly on the stack.
 * ═══════════════════════════════════════════════════════════ */

#define MAX_LOCALS 8
#define MAX_STACK_FRAMES 16

typedef union {
    int    int_val;
    double double_val;
    void  *ref_val;    /* reference to heap object */
} LocalVar;

typedef enum {
    VAR_INT, VAR_DOUBLE, VAR_REF, VAR_EMPTY
} VarType;

typedef struct {
    char      method_name[64];
    LocalVar  locals[MAX_LOCALS];
    VarType   local_types[MAX_LOCALS];
    int       local_count;
    int       operand_stack_top;
} StackFrame;

typedef struct {
    StackFrame frames[MAX_STACK_FRAMES];
    int        depth;             /* current frame count */
} ThreadStack;

/* Push a new stack frame (= method call) */
static StackFrame* stack_push_frame(ThreadStack *ts, const char *method) {
    if (ts->depth >= MAX_STACK_FRAMES) {
        printf("  [StackOverflowError] Stack depth exceeded!\n");
        return NULL;
    }
    StackFrame *f = &ts->frames[ts->depth++];
    memset(f, 0, sizeof(StackFrame));
    strncpy(f->method_name, method, 63);
    printf("  [Stack] push frame  → %s  (depth=%d)\n",
           method, ts->depth);
    return f;
}

/* Pop a stack frame (= method return) */
static void stack_pop_frame(ThreadStack *ts) {
    if (ts->depth == 0) return;
    StackFrame *f = &ts->frames[ts->depth - 1];
    printf("  [Stack] pop  frame  ← %s  (depth=%d)\n",
           f->method_name, ts->depth);
    ts->depth--;
}

/* Add a local variable to the current frame */
static void frame_add_int(StackFrame *f, const char *var_name, int val) {
    if (f->local_count >= MAX_LOCALS) return;
    (void)var_name;
    f->locals[f->local_count].int_val   = val;
    f->local_types[f->local_count]      = VAR_INT;
    printf("    [local] int %-10s = %d  (on stack)\n", var_name, val);
    f->local_count++;
}

static void frame_add_ref(StackFrame *f, const char *var_name, void *heap_ptr) {
    if (f->local_count >= MAX_LOCALS) return;
    (void)var_name;
    f->locals[f->local_count].ref_val   = heap_ptr;
    f->local_types[f->local_count]      = VAR_REF;
    printf("    [local] ref %-10s = %p  (on stack, points to heap)\n",
           var_name, heap_ptr);
    f->local_count++;
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_stack_vs_heap(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Stack vs Heap — where variables actually live\n");
    printf("Java:\n");
    printf("  void main() {\n");
    printf("    int x = 42;                   // primitive: lives on stack\n");
    printf("    String s = new String(\"hi\");  // ref on stack, object on heap\n");
    printf("    process(s);                   // new stack frame\n");
    printf("  }\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ThreadStack ts = {0};

    /* main() frame */
    StackFrame *main_frame = stack_push_frame(&ts, "main()");
    frame_add_int(main_frame, "x", 42);            /* int x = 42 → stack */

    /* new String("hi") → allocated on Eden heap */
    SimObject *str_obj = eden_alloc("String", 32);
    snprintf(str_obj->data, sizeof(str_obj->data), "hi");
    frame_add_ref(main_frame, "s", str_obj);       /* ref s → stack, points to heap */

    /* process(s) call → new stack frame */
    StackFrame *proc_frame = stack_push_frame(&ts, "process(String)");
    frame_add_ref(proc_frame, "param", str_obj);   /* param = same heap pointer */

    printf("\n  Note: both 'main.s' and 'process.param' point to the SAME heap object!\n");
    printf("  This is why Java is 'pass by value of reference' — the pointer is copied.\n\n");

    /* Return from process — pop frame */
    stack_pop_frame(&ts);   /* process() returns → frame gone */
    stack_pop_frame(&ts);   /* main() returns → frame gone, str_obj now unreachable */

    printf("  After main() returns: str_obj still at %p on heap.\n", (void*)str_obj);
    printf("  Stack frames are gone — no more references.\n");
    printf("  GC will collect str_obj on next Minor GC.\n\n");
}

void demo_gc_in_action(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Minor GC — collecting short-lived objects\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Allocate several objects */
    SimObject *user   = eden_alloc("User",        64);
    SimObject *order  = eden_alloc("Order",       64);
    SimObject *temp1  = eden_alloc("TempBuffer",  64);
    SimObject *temp2  = eden_alloc("TempString",  64);
    SimObject *config = eden_alloc("Config",      64);

    if (!user || !order || !temp1 || !temp2 || !config) {
        printf("Allocation failed\n"); return;
    }

    printf("\n  Simulating: temp1 and temp2 go out of scope\n");
    printf("  GC roots: user, order, config (still referenced from stack)\n\n");

    /* Only user, order, config are still referenced (in GC roots) */
    SimObject *roots[] = { user, order, config };
    simulate_minor_gc(roots, 3);

    printf("  After GC:\n");
    printf("    user   still valid @ %p\n", (void*)user);
    printf("    order  still valid @ %p\n", (void*)order);
    printf("    config still valid @ %p\n", (void*)config);
    printf("    temp1, temp2: COLLECTED (memory returned to Eden)\n\n");
}

void demo_stack_overflow(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: StackOverflowError — infinite recursion fills stack\n");
    printf("Java: void recurse() { recurse(); } → StackOverflowError\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ThreadStack ts = {0};

    printf("  Simulating infinite recursion (limited to %d frames):\n\n",
           MAX_STACK_FRAMES);

    for (int i = 0; i < MAX_STACK_FRAMES + 2; i++) {
        StackFrame *f = stack_push_frame(&ts, "recurse()");
        if (!f) {
            printf("\n  !! StackOverflowError at depth %d !!\n", i);
            printf("  Java: throws java.lang.StackOverflowError\n");
            printf("  Fix:  -Xss1m  (increase stack size per thread)\n");
            printf("        OR: use iteration instead of recursion\n");
            printf("        OR: use explicit Stack data structure on heap\n\n");
            break;
        }
        frame_add_int(f, "i", i);
    }
}

void demo_object_header_size(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Object Memory Overhead\n");
    printf("Every Java object has a 16-byte header — even new Object()!\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    typedef struct { int x; int y; }       Point2D;
    typedef struct { int x; int y; int z; } Point3D;
    typedef struct { char c; }              CharBox;

    printf("  C sizes (no header overhead):\n");
    printf("    sizeof(Point2D)  = %zu bytes  (just the fields)\n", sizeof(Point2D));
    printf("    sizeof(Point3D)  = %zu bytes\n", sizeof(Point3D));
    printf("    sizeof(CharBox)  = %zu bytes\n", sizeof(CharBox));

    printf("\n  Java equivalent sizes (with 16-byte object header):\n");
    printf("    new Object()                = 16 bytes (header only)\n");
    printf("    new Point(int x, int y)     = 16 + 4 + 4 = 24 bytes\n");
    printf("    new Point(x, y, z)          = 16 + 4 + 4 + 4 = 28 bytes → padded to 32\n");
    printf("    new CharWrapper(char c)     = 16 + 2 = 18 bytes → padded to 24\n");

    printf("\n  ObjectHeader in our simulation:\n");
    printf("    sizeof(ObjectHeader) = %zu bytes\n", sizeof(ObjectHeader));
    printf("    sizeof(SimObject)    = %zu bytes\n\n", sizeof(SimObject));

    printf("  LESSON: Java has significant memory overhead per object.\n");
    printf("          Millions of small objects → significant header overhead.\n");
    printf("          Solution: primitive arrays, value types (JDK 22 Valhalla)\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  heap_stack_model.c — JVM Memory Regions in C       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    printf("Memory region sizes (our simulation):\n");
    printf("  Eden:     %d KB\n", EDEN_SIZE / 1024);
    printf("  Survivor: %d KB x2\n", SURVIVOR_SIZE / 1024);
    printf("  Old Gen:  %d KB\n\n", OLD_GEN_SIZE / 1024);

    demo_stack_vs_heap();
    demo_gc_in_action();
    demo_stack_overflow();
    demo_object_header_size();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Stack = thread-local, method frames, primitives + references\n");
    printf("  Heap  = shared across threads, all objects live here\n");
    printf("  Eden  = fast bump-pointer allocation (no lock needed per-thread)\n");
    printf("  Minor GC = collect Eden + survivors, promote old objects\n");
    printf("  Every Java object has 16-byte header overhead\n");
    printf("  StackOverflowError = stack frames exceed -Xss limit\n");
    return 0;
}
