/*
 * mark_sweep.c  —  Mark-Sweep-Compact Garbage Collector in C
 *
 * Build:  gcc -Wall -Wextra -o mark_sweep mark_sweep.c
 * Run:    ./mark_sweep
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Java's garbage collection — specifically the Mark-Sweep-Compact
 *   algorithm used in Serial GC and as the foundation of all JVM GCs.
 *
 *   The three GC phases:
 *     MARK:    start from GC roots, traverse object graph, mark reachable
 *     SWEEP:   scan heap linearly, free any unmarked (unreachable) objects
 *     COMPACT: slide live objects to one end, fixing fragmentation
 *
 *   In real JVM:
 *     - "Mark" bit is stored in the object's Mark Word (object header)
 *     - "Sweep" walks the heap using object size to step through
 *     - "Compact" updates ALL references in ALL objects (expensive!)
 *
 *   This file implements a complete, working tri-color mark-sweep GC
 *   on a small simulated heap. Every step is commented to map
 *   exactly to the JVM's real behavior.
 *
 * IMPORTANT CONCEPTS DEMONSTRATED:
 *   - GC roots: where object graph traversal starts
 *   - Reachability: objects are only collected if UNREACHABLE from roots
 *   - Memory fragmentation: solved by compaction
 *   - Stop-the-world: entire simulation halts during GC
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  HEAP CONFIGURATION
 * ═══════════════════════════════════════════════════════════ */
#define HEAP_CAPACITY    16         /* max objects our heap can hold     */
#define MAX_REFS_PER_OBJ  4         /* max outgoing references per object */
#define MAX_GC_ROOTS      8         /* max GC roots (thread locals, statics) */

/* ═══════════════════════════════════════════════════════════
 *  TRI-COLOR MARKING STATE
 *  Real JVM GC uses tri-color invariant:
 *    WHITE = not yet visited (may be garbage)
 *    GRAY  = visited but references not yet processed
 *    BLACK = fully processed (definitely reachable)
 *
 *  At end of marking:
 *    BLACK = live (keep)
 *    WHITE = garbage (collect)
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    COLOR_WHITE = 0,   /* not yet reached (potential garbage) */
    COLOR_GRAY  = 1,   /* reached but children not scanned    */
    COLOR_BLACK = 2,   /* fully scanned (definitely live)     */
} GCColor;

static const char* color_name(GCColor c) {
    static const char *names[] = { "WHITE", "GRAY ", "BLACK" };
    return names[(int)c < 3 ? (int)c : 0];
}

/* ═══════════════════════════════════════════════════════════
 *  HEAP OBJECT — every Java object on the heap
 *
 *  Real Java object:
 *    header (16 bytes) + fields
 *
 *  We simulate: id + class name + references to other objects
 * ═══════════════════════════════════════════════════════════ */
typedef struct HeapObject {
    int    id;                           /* unique object ID                 */
    char   class_name[32];              /* Java class name (e.g. "User")    */
    char   value[64];                   /* object field data                 */
    GCColor gc_color;                   /* current GC mark color             */
    int    gc_age;                      /* how many GCs survived             */

    /* References to other objects (= Java fields that hold references)
     * Real Java: field "UserRepository repo;" stores a pointer to another object */
    struct HeapObject *refs[MAX_REFS_PER_OBJ];
    int    ref_count;

    int    free;                        /* 1 = slot is free (not in use)    */
} HeapObject;

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED HEAP
 *  Real JVM: contiguous virtual memory block
 *  We use: fixed-size array of HeapObjects
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    HeapObject objects[HEAP_CAPACITY];  /* the heap "memory"              */
    int        count;                   /* total allocated (ever)          */
    int        live_count;              /* currently live (not collected)  */
    int        gc_cycle;                /* how many GCs have run           */

    /* GC statistics */
    long       total_allocated;
    long       total_collected;
    long       total_promoted;
} Heap;

/* GC roots — the set of "starting points" for marking.
 * In real JVM: local variables in all thread stacks + static fields + JNI */
static HeapObject *gc_roots[MAX_GC_ROOTS];
static int         gc_root_count = 0;

/* Gray objects waiting for their references to be scanned
 * (A work queue for the marking phase) */
static HeapObject *gray_queue[HEAP_CAPACITY * 2];
static int         gray_head = 0, gray_tail = 0;

static void gray_push(HeapObject *obj) {
    gray_queue[gray_tail++ % (HEAP_CAPACITY*2)] = obj;
}
static HeapObject* gray_pop(void) {
    if (gray_head == gray_tail) return NULL;
    return gray_queue[gray_head++ % (HEAP_CAPACITY*2)];
}

/* ─────────────────────────────────────────────────────────────
 *  HEAP ALLOCATION
 *  Java: new MyObject()
 *  JVM:  bump pointer in Eden → return address
 *  Ours: find first free slot in array
 * ──────────────────────────────────────────────────────────── */
static HeapObject* heap_alloc(Heap *h, const char *class_name, const char *value) {
    /* Find a free slot */
    for (int i = 0; i < HEAP_CAPACITY; i++) {
        if (h->objects[i].free) {
            HeapObject *obj = &h->objects[i];
            memset(obj, 0, sizeof(HeapObject));

            obj->id        = ++h->count;
            obj->free      = 0;
            obj->gc_color  = COLOR_WHITE;  /* new objects start WHITE */
            obj->gc_age    = 0;
            obj->ref_count = 0;

            strncpy(obj->class_name, class_name, 31);
            strncpy(obj->value,      value,      63);

            h->live_count++;
            h->total_allocated++;

            printf("  [alloc] #%-2d %-20s \"%s\"\n",
                   obj->id, class_name, value);
            return obj;
        }
    }

    /* Heap is full → OutOfMemoryError */
    fprintf(stderr,
            "  [OOM] Heap full (%d/%d slots used) → OutOfMemoryError: Java heap space\n",
            h->live_count, HEAP_CAPACITY);
    return NULL;
}

/* Add a reference: obj_a.field = obj_b */
static void heap_add_ref(HeapObject *from, HeapObject *to) {
    if (from->ref_count >= MAX_REFS_PER_OBJ || !to) return;
    from->refs[from->ref_count++] = to;
    printf("  [ref]  #%-2d %-15s → #%-2d %-15s\n",
           from->id, from->class_name, to->id, to->class_name);
}

/* Add a GC root (a reference from the stack/static) */
static void gc_add_root(HeapObject *obj) {
    if (gc_root_count >= MAX_GC_ROOTS || !obj) return;
    gc_roots[gc_root_count++] = obj;
}

/* ─────────────────────────────────────────────────────────────
 *  GC PHASE 1: MARK
 *
 *  Algorithm (tri-color BFS marking):
 *    1. All objects start WHITE
 *    2. Push all GC roots → GRAY queue
 *    3. While GRAY queue not empty:
 *       a. Pop one GRAY object → set BLACK
 *       b. For each reference it holds:
 *          if target is WHITE → push to GRAY queue, set GRAY
 *    4. After queue empty:
 *       BLACK = live (reachable)
 *       WHITE = dead (garbage, safe to collect)
 *
 *  Real JVM: concurrent marking runs alongside app threads
 *             with write barriers to handle new references
 * ──────────────────────────────────────────────────────────── */
static void gc_mark(Heap *h) {
    printf("\n  ── Phase 1: MARK ────────────────────────────────────\n");
    printf("  Starting from %d GC root(s)\n\n", gc_root_count);

    /* Reset all objects to WHITE */
    for (int i = 0; i < HEAP_CAPACITY; i++) {
        if (!h->objects[i].free) {
            h->objects[i].gc_color = COLOR_WHITE;
        }
    }

    /* Reset gray queue */
    gray_head = gray_tail = 0;

    /* Push all GC roots to GRAY queue */
    for (int i = 0; i < gc_root_count; i++) {
        HeapObject *root = gc_roots[i];
        if (root && !root->free && root->gc_color == COLOR_WHITE) {
            root->gc_color = COLOR_GRAY;
            gray_push(root);
            printf("  [root]  #%-2d %-15s → GRAY\n",
                   root->id, root->class_name);
        }
    }

    /* BFS traversal: process all GRAY objects */
    HeapObject *obj;
    while ((obj = gray_pop()) != NULL) {
        /* Process this GRAY object → scan its references */
        printf("  [scan]  #%-2d %-15s %s→BLACK, scanning %d ref(s)\n",
               obj->id, obj->class_name, color_name(obj->gc_color),
               obj->ref_count);

        obj->gc_color = COLOR_BLACK;  /* fully processed */

        /* Visit each reference this object holds */
        for (int r = 0; r < obj->ref_count; r++) {
            HeapObject *ref = obj->refs[r];
            if (ref && !ref->free && ref->gc_color == COLOR_WHITE) {
                /* Newly discovered reachable object */
                ref->gc_color = COLOR_GRAY;
                gray_push(ref);
                printf("    [mark] #%-2d %-15s → GRAY (via #%d.ref[%d])\n",
                       ref->id, ref->class_name, obj->id, r);
            }
        }
    }

    printf("\n  Mark complete:\n");
    for (int i = 0; i < HEAP_CAPACITY; i++) {
        if (!h->objects[i].free) {
            printf("    #%-2d %-20s → %s\n",
                   h->objects[i].id,
                   h->objects[i].class_name,
                   color_name(h->objects[i].gc_color));
        }
    }
}

/* ─────────────────────────────────────────────────────────────
 *  GC PHASE 2: SWEEP
 *
 *  Walk the entire heap.
 *  Any WHITE object = unreachable = garbage → free it.
 *  Any BLACK object = reachable = keep it, reset to WHITE for next GC.
 *
 *  Real JVM: uses free lists to track freed chunks
 *            can coalesce adjacent free blocks
 * ──────────────────────────────────────────────────────────── */
static int gc_sweep(Heap *h) {
    printf("\n  ── Phase 2: SWEEP ───────────────────────────────────\n");

    int collected = 0;

    for (int i = 0; i < HEAP_CAPACITY; i++) {
        HeapObject *obj = &h->objects[i];
        if (obj->free) continue;

        if (obj->gc_color == COLOR_WHITE) {
            /* Unreachable — COLLECT IT */
            printf("  [free]  #%-2d %-20s (unreachable → collected)\n",
                   obj->id, obj->class_name);
            obj->free = 1;
            h->live_count--;
            h->total_collected++;
            collected++;
        } else {
            /* Reachable — keep, increment GC age, reset color */
            obj->gc_age++;
            obj->gc_color = COLOR_WHITE;  /* reset for next GC cycle */
        }
    }

    printf("  Collected: %d objects  |  Live: %d objects\n",
           collected, h->live_count);
    return collected;
}

/* ─────────────────────────────────────────────────────────────
 *  GC PHASE 3: COMPACT (optional)
 *
 *  Slide all live objects to the start of the heap.
 *  Eliminates fragmentation.
 *  Cost: must update ALL references to moved objects.
 *
 *  Real JVM (G1): evacuates live objects to new region,
 *                 then reclaims entire old region at once.
 * ──────────────────────────────────────────────────────────── */
static void gc_compact(Heap *h) {
    printf("\n  ── Phase 3: COMPACT ─────────────────────────────────\n");

    int compact_pos = 0;    /* next free slot at start of heap */
    int moves = 0;

    /* Pass 1: move all live objects to front */
    for (int i = 0; i < HEAP_CAPACITY; i++) {
        if (!h->objects[i].free && i != compact_pos) {
            /* Move object from slot i → slot compact_pos */
            h->objects[compact_pos] = h->objects[i];
            h->objects[i].free = 1;   /* old slot is now free */
            printf("  [move]  #%-2d %-15s slot %d → %d\n",
                   h->objects[compact_pos].id,
                   h->objects[compact_pos].class_name,
                   i, compact_pos);
            moves++;
            compact_pos++;
        } else if (!h->objects[i].free) {
            compact_pos++;
        }
    }

    printf("  Compacted: %d object(s) moved, heap now contiguous\n", moves);
    printf("  (In real JVM: all references to moved objects are updated)\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Full GC run
 * ──────────────────────────────────────────────────────────── */
static void gc_run(Heap *h) {
    h->gc_cycle++;
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║  GC Cycle #%d — Stop-The-World                       \n", h->gc_cycle);
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("  Before: %d live objects, %ld total allocated\n\n",
           h->live_count, h->total_allocated);

    gc_mark(h);
    gc_sweep(h);
    gc_compact(h);

    printf("\n  After GC #%d: %d live objects remain\n",
           h->gc_cycle, h->live_count);
    printf("  Total collected (all time): %ld\n\n", h->total_collected);
}

/* ─────────────────────────────────────────────────────────────
 *  Print heap state
 * ──────────────────────────────────────────────────────────── */
static void heap_print(const Heap *h) {
    printf("  Heap state (%d/%d slots, %d live):\n",
           h->live_count, HEAP_CAPACITY, h->live_count);
    for (int i = 0; i < HEAP_CAPACITY; i++) {
        const HeapObject *o = &h->objects[i];
        if (!o->free) {
            printf("    slot[%2d] #%-2d %-20s age=%-2d refs=%d value='%s'\n",
                   i, o->id, o->class_name, o->gc_age, o->ref_count, o->value);
        }
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_basic_gc(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Basic Mark-Sweep GC\n");
    printf("Some objects go out of scope → GC collects them\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Heap h = {0};
    /* Initialize all slots as free */
    for (int i = 0; i < HEAP_CAPACITY; i++) h.objects[i].free = 1;
    gc_root_count = 0;

    printf("── Allocating objects ──────────────────────────────────\n");

    /* Simulates this Java code:
     *   UserService svc = new UserService();        // rooted (static field)
     *   UserRepository repo = new UserRepository(); // svc holds ref
     *   String query = new String("SELECT *");      // repo holds ref
     *   String temp1 = new String("tmp1");          // local, will go out of scope
     *   String temp2 = new String("tmp2");          // local, will go out of scope
     *   Config cfg = new Config();                  // rooted (static)
     */
    HeapObject *svc   = heap_alloc(&h, "UserService",    "business logic");
    HeapObject *repo  = heap_alloc(&h, "UserRepository", "data access");
    HeapObject *query = heap_alloc(&h, "String",         "SELECT * FROM users");
    HeapObject *temp1 = heap_alloc(&h, "String",         "tmp_buffer_1");
    HeapObject *temp2 = heap_alloc(&h, "String",         "tmp_result");
    HeapObject *cfg   = heap_alloc(&h, "Config",         "db.url=localhost");

    printf("\n── Setting up object references ────────────────────────\n");
    heap_add_ref(svc,  repo);    /* userService.repo = repo */
    heap_add_ref(repo, query);   /* repository.lastQuery = query */
    /* temp1 and temp2 have NO references from rooted objects */
    /* cfg has NO outgoing refs */

    printf("\n── Adding GC roots (static fields + thread-stack locals) ─\n");
    gc_add_root(svc);    /* static field: AppContext.userService = svc */
    gc_add_root(cfg);    /* static field: AppContext.config = cfg      */
    /* temp1, temp2 NOT in roots → they are local variables that went out of scope */

    printf("\nHeap before GC:\n");
    heap_print(&h);

    /* Trigger GC */
    gc_run(&h);

    printf("Heap after GC:\n");
    heap_print(&h);

    printf("  RESULT: temp1 and temp2 collected (not reachable from any root)\n");
    printf("  RESULT: svc, repo, query, cfg survived (reachable via root chain)\n\n");
}

void demo_circular_reference(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Circular References — GC handles them, ref-counting cannot\n");
    printf("C++ shared_ptr circular refs = leak. Java GC = no problem.\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    Heap h = {0};
    for (int i = 0; i < HEAP_CAPACITY; i++) h.objects[i].free = 1;
    gc_root_count = 0;

    printf("── Allocating circular reference pair ──────────────────\n");

    HeapObject *a = heap_alloc(&h, "NodeA", "data: a");
    HeapObject *b = heap_alloc(&h, "NodeB", "data: b");
    HeapObject *c = heap_alloc(&h, "NodeC", "data: c");   /* rooted, stays alive */

    if (!a || !b || !c) return;

    /* Circular: A → B → A (neither is in GC roots → both garbage) */
    heap_add_ref(a, b);   /* a.next = b */
    heap_add_ref(b, a);   /* b.prev = a (cycle!) */

    /* C is rooted and has no cycle */
    gc_add_root(c);       /* c is still referenced from stack */

    printf("\n  Circular reference: A → B → A (neither in GC roots)\n");
    printf("  In C++ with shared_ptr: ref count A=1 (from B), B=1 (from A)\n");
    printf("  → Neither ever reaches 0 → LEAK in ref-counting systems\n");
    printf("  In Java GC: mark starts from roots → A and B never reached\n");
    printf("  → Both collected even with circular references\n\n");

    printf("Heap before GC:\n");
    heap_print(&h);

    gc_run(&h);

    printf("Heap after GC:\n");
    heap_print(&h);

    printf("  RESULT: A and B COLLECTED (circular ref is no problem for mark-sweep)\n");
    printf("  RESULT: C survived (reachable from root)\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  mark_sweep.c — Tri-Color Mark-Sweep GC in C        ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_basic_gc();
    demo_circular_reference();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  GC roots = starting points: thread stacks + static fields\n");
    printf("  MARK: BFS from roots → color reachable objects BLACK\n");
    printf("  SWEEP: WHITE objects = garbage → free them\n");
    printf("  COMPACT: slide live objects together → no fragmentation\n");
    printf("  Circular references are NOT a problem for mark-sweep GC\n");
    printf("  Stop-the-world: app PAUSES during GC → tune -Xms/-Xmx\n");
    return 0;
}
