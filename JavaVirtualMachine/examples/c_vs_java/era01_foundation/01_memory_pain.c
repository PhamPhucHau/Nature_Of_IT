/*
 * 01_memory_pain.c — The C pain that drove Java's GC
 *
 * Build: gcc -Wall -Wextra -o 01_memory_pain 01_memory_pain.c
 * Run:   ./01_memory_pain
 *
 * This file demonstrates every memory bug Java's GC eliminates:
 *  1. Memory leak  — forgot to free
 *  2. Double free  — freed twice
 *  3. Use-after-free — read after freed
 *  4. Buffer overflow — wrote past the end
 *
 * Java's answer: allocate on GC heap, never call free().
 * The GC runs mark-and-sweep to find and collect unreachable objects.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * BUG 1: Memory Leak — the most common C bug
 * Java fix: GC automatically frees when no references remain
 * ============================================================ */
void demonstrate_memory_leak(void) {
    printf("=== Bug 1: Memory Leak ===\n");

    /* Allocate memory */
    char *name = malloc(100);
    if (!name) { perror("malloc"); return; }

    strcpy(name, "James Gosling");
    printf("  Name: %s\n", name);

    /* FORGOT to free(name) here! */
    /* name just leaked — memory stays allocated until process exits */

    printf("  [LEAK] Forgot free(name) — 100 bytes lost!\n");
    printf("  [JAVA] String name = \"James Gosling\"; — GC handles it\n\n");

    /* CORRECT version: */
    char *name2 = malloc(100);
    if (!name2) { perror("malloc"); return; }
    strcpy(name2, "Fixed!");
    printf("  Correct: %s\n", name2);
    free(name2);  /* properly freed */
    printf("  [OK] free(name2) called\n\n");
}

/* ============================================================
 * BUG 2: Double Free — crash on second free()
 * Java fix: No free() — GC owns all memory
 * ============================================================ */
void demonstrate_double_free(void) {
    printf("=== Bug 2: Double Free (simulated — not actually run) ===\n");

    /*
     * DANGER: Running this crashes the program.
     * Uncomment only to observe the crash.
     *
     *   int *data = malloc(sizeof(int) * 5);
     *   free(data);
     *   free(data);   <- DOUBLE FREE → undefined behavior → crash / heap corruption
     */

    printf("  [DANGER] Code exists but is not executed to avoid crash\n");
    printf("  [JAVA]  No free() exists. GC ensures each object freed exactly once.\n\n");
}

/* ============================================================
 * BUG 3: Use After Free — reads garbage or crashes
 * Java fix: References set to null; GC won't free reachable objects
 * ============================================================ */
void demonstrate_use_after_free(void) {
    printf("=== Bug 3: Use After Free ===\n");

    int *data = (int *)malloc(sizeof(int) * 4);
    if (!data) { perror("malloc"); return; }

    data[0] = 10; data[1] = 20; data[2] = 30; data[3] = 40;
    printf("  Before free: data[0]=%d data[1]=%d\n", data[0], data[1]);

    free(data);  /* freed here */

    /* DANGER: data[0] is now garbage — freed memory */
    /* In practice this may print garbage or crash */
    /* We check the pointer to demonstrate the concept safely */
    printf("  After free:  data pointer still holds address %p\n", (void *)data);
    printf("  [DANGER] Reading data[0] after free() = undefined behavior\n");
    printf("  [JAVA]   obj = null; then obj.field → NullPointerException (safe, controlled)\n\n");

    /* CORRECT: null the pointer after freeing (dangling pointer prevention) */
    data = NULL;
    if (data == NULL) {
        printf("  [OK] Nulled the pointer — safe to check\n\n");
    }
}

/* ============================================================
 * BUG 4: Buffer Overflow — silent data corruption
 * Java fix: Array bounds check → ArrayIndexOutOfBoundsException
 * ============================================================ */
void demonstrate_buffer_overflow(void) {
    printf("=== Bug 4: Buffer Overflow ===\n");

    int arr[5] = {1, 2, 3, 4, 5};
    printf("  arr: ");
    for (int i = 0; i < 5; i++) printf("%d ", arr[i]);
    printf("\n");

    /*
     * DANGEROUS: Writing past the array boundary.
     * This may corrupt adjacent memory silently.
     * We only READ out of bounds here to show the concept.
     */
    printf("  arr[5]  = %d  ← out of bounds! (reading garbage)\n", arr[5]);
    printf("  arr[10] = %d  ← way out of bounds! (could be anything)\n", arr[10]);
    printf("  [DANGER] No crash guaranteed — silent data corruption is possible\n");
    printf("  [JAVA]   arr[5] → ArrayIndexOutOfBoundsException (always caught)\n\n");
}

/* ============================================================
 * COMPARISON: Java-style safe string in C
 * Java fixes this with String class + GC
 * ============================================================ */

/* A "safe string" wrapper — mimicking what Java's String class does */
typedef struct {
    char   *data;
    size_t  length;
    size_t  capacity;
    int     immutable;  /* 1 = read-only, like Java String */
} SafeString;

SafeString *safestr_create(const char *s) {
    SafeString *str = malloc(sizeof(SafeString));
    if (!str) return NULL;
    str->length   = strlen(s);
    str->capacity = str->length + 1;
    str->data     = malloc(str->capacity);
    str->immutable = 1;  /* strings are immutable in Java */
    if (!str->data) { free(str); return NULL; }
    memcpy(str->data, s, str->capacity);
    return str;
}

void safestr_print(const SafeString *str) {
    if (!str) { printf("null"); return; }
    printf("\"%.*s\" (len=%zu)", (int)str->length, str->data, str->length);
}

void safestr_free(SafeString *str) {
    if (!str) return;
    free(str->data);
    free(str);
}

SafeString *safestr_concat(const SafeString *a, const SafeString *b) {
    /* Returns a NEW string — never modifies a or b (immutable semantics) */
    size_t total = a->length + b->length;
    SafeString *result = malloc(sizeof(SafeString));
    if (!result) return NULL;
    result->data = malloc(total + 1);
    if (!result->data) { free(result); return NULL; }
    memcpy(result->data, a->data, a->length);
    memcpy(result->data + a->length, b->data, b->length);
    result->data[total] = '\0';
    result->length   = total;
    result->capacity = total + 1;
    result->immutable = 1;
    return result;
}

void demonstrate_safe_string(void) {
    printf("=== Safe String (how Java String works internally) ===\n");

    SafeString *hello = safestr_create("Hello");
    SafeString *world = safestr_create(", World!");
    SafeString *combined = safestr_concat(hello, world);

    printf("  hello:    "); safestr_print(hello);    printf("\n");
    printf("  world:    "); safestr_print(world);    printf("\n");
    printf("  combined: "); safestr_print(combined); printf("\n");

    /* Java equivalent:
     *   String hello    = "Hello";
     *   String world    = ", World!";
     *   String combined = hello + world;  // new String — hello and world unchanged
     */

    safestr_free(hello);
    safestr_free(world);
    safestr_free(combined);
    printf("  [OK] All memory properly freed (GC would do this automatically)\n\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Era 01 — Memory Pain (what Java's GC solves)       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demonstrate_memory_leak();
    demonstrate_double_free();
    demonstrate_use_after_free();
    demonstrate_buffer_overflow();
    demonstrate_safe_string();

    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("SUMMARY: Java's JVM eliminates ALL of the above:\n");
    printf("  - GC:   no malloc/free needed → no leaks, no double-free\n");
    printf("  - Null: NullPointerException instead of silent crash\n");
    printf("  - Arrays: bounds-checked → ArrayIndexOutOfBoundsException\n");
    printf("  - String: immutable value type, GC-managed lifetime\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");

    return 0;
}
