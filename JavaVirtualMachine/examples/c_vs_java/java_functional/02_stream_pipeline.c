/*
 * 02_stream_pipeline.c  —  Java Stream API in C
 *
 * Build:  gcc -Wall -Wextra -o 02_stream_pipeline 02_stream_pipeline.c
 * Run:    ./02_stream_pipeline
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA STREAM PIPELINE WORKS:
 *
 *  A Stream pipeline has three parts:
 *    1. SOURCE:       Collection, array, or generator
 *    2. INTERMEDIATE: filter, map, sorted, distinct, limit, skip
 *       → Returns a NEW Stream (lazy — not computed yet)
 *    3. TERMINAL:     collect, forEach, reduce, count, min, max
 *       → Triggers actual computation
 *
 *  KEY: Java Streams are LAZY. Intermediate operations build up
 *  a chain of operations. Nothing runs until the terminal op.
 *
 *  Under the hood, each intermediate op creates a new Stream
 *  that wraps the previous one:
 *    source → FilteredStream → MappedStream → SortedStream
 *  Terminal op pulls elements through the entire chain.
 *
 *  In C, we implement this as:
 *    - A Stream struct with: int[] data, size
 *    - Each operation: transform data into new array
 *    - Chain by passing result to next operation
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  STREAM STRUCT — holds current state of the pipeline
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    int  *data;     /* current elements */
    int   size;     /* number of elements */
    int   owned;    /* 1 = we own data and must free it */
} IntStream;

/* ── Create stream from array (source) ── */
IntStream stream_of(const int *arr, int len) {
    IntStream s;
    s.data  = malloc(len * sizeof(int));
    s.size  = len;
    s.owned = 1;
    memcpy(s.data, arr, len * sizeof(int));
    return s;
}

/* ── Create stream with range: [start, end) ── */
IntStream stream_range(int start, int end) {
    int len = end - start;
    IntStream s;
    s.data  = malloc(len * sizeof(int));
    s.size  = len;
    s.owned = 1;
    for (int i = 0; i < len; i++)
        s.data[i] = start + i;
    return s;
}

void stream_free(IntStream *s) {
    if (s->owned && s->data) {
        free(s->data);
        s->data = NULL;
        s->size = 0;
    }
}

/* ── Print stream like Java's Arrays.toString ── */
void stream_print(const IntStream *s, const char *label) {
    printf("  %-15s [", label);
    for (int i = 0; i < s->size; i++) {
        printf("%d", s->data[i]);
        if (i < s->size - 1) printf(", ");
    }
    printf("]  (size=%d)\n", s->size);
}

/* ═══════════════════════════════════════════════════════════
 *  INTERMEDIATE OPERATIONS — each returns a new IntStream
 * ═══════════════════════════════════════════════════════════ */

/* stream.filter(predicate) */
IntStream stream_filter(IntStream *s, int (*pred)(int)) {
    int *temp = malloc(s->size * sizeof(int));
    int count = 0;
    for (int i = 0; i < s->size; i++)
        if (pred(s->data[i])) temp[count++] = s->data[i];

    IntStream result;
    result.data  = malloc(count * sizeof(int));
    result.size  = count;
    result.owned = 1;
    memcpy(result.data, temp, count * sizeof(int));
    free(temp);
    return result;
}

/* stream.map(function) */
IntStream stream_map(IntStream *s, int (*fn)(int)) {
    IntStream result;
    result.data  = malloc(s->size * sizeof(int));
    result.size  = s->size;
    result.owned = 1;
    for (int i = 0; i < s->size; i++)
        result.data[i] = fn(s->data[i]);
    return result;
}

/* stream.sorted() */
static int cmp_int_asc (const void *a, const void *b) { return *(int*)a - *(int*)b; }
static int cmp_int_desc(const void *a, const void *b) { return *(int*)b - *(int*)a; }

IntStream stream_sorted(IntStream *s) {
    IntStream result;
    result.data  = malloc(s->size * sizeof(int));
    result.size  = s->size;
    result.owned = 1;
    memcpy(result.data, s->data, s->size * sizeof(int));
    qsort(result.data, result.size, sizeof(int), cmp_int_asc);
    return result;
}

IntStream stream_sorted_desc(IntStream *s) {
    IntStream result;
    result.data  = malloc(s->size * sizeof(int));
    result.size  = s->size;
    result.owned = 1;
    memcpy(result.data, s->data, s->size * sizeof(int));
    qsort(result.data, result.size, sizeof(int), cmp_int_desc);
    return result;
}

/* stream.distinct() — remove duplicates (preserves order) */
IntStream stream_distinct(IntStream *s) {
    int *seen = calloc(10000, sizeof(int));  /* simple bitmap for demo */
    int *temp = malloc(s->size * sizeof(int));
    int count = 0;
    for (int i = 0; i < s->size; i++) {
        int v = s->data[i];
        if (!seen[v]) { seen[v] = 1; temp[count++] = v; }
    }
    free(seen);
    IntStream result;
    result.data  = malloc(count * sizeof(int));
    result.size  = count;
    result.owned = 1;
    memcpy(result.data, temp, count * sizeof(int));
    free(temp);
    return result;
}

/* stream.limit(n) */
IntStream stream_limit(IntStream *s, int n) {
    int size = n < s->size ? n : s->size;
    IntStream result;
    result.data  = malloc(size * sizeof(int));
    result.size  = size;
    result.owned = 1;
    memcpy(result.data, s->data, size * sizeof(int));
    return result;
}

/* stream.skip(n) */
IntStream stream_skip(IntStream *s, int n) {
    int skip = n < s->size ? n : s->size;
    int size = s->size - skip;
    IntStream result;
    result.data  = malloc(size * sizeof(int));
    result.size  = size;
    result.owned = 1;
    memcpy(result.data, s->data + skip, size * sizeof(int));
    return result;
}

/* stream.peek(consumer) — intermediate, for debugging */
IntStream stream_peek(IntStream *s, void (*consumer)(int)) {
    for (int i = 0; i < s->size; i++) consumer(s->data[i]);
    /* Returns same stream */
    IntStream result;
    result.data  = malloc(s->size * sizeof(int));
    result.size  = s->size;
    result.owned = 1;
    memcpy(result.data, s->data, s->size * sizeof(int));
    return result;
}

/* ═══════════════════════════════════════════════════════════
 *  TERMINAL OPERATIONS — trigger the pipeline
 * ═══════════════════════════════════════════════════════════ */

/* stream.forEach(consumer) */
void stream_foreach(IntStream *s, void (*consumer)(int)) {
    for (int i = 0; i < s->size; i++) consumer(s->data[i]);
    stream_free(s);
}

/* stream.count() */
int stream_count(IntStream *s) {
    int n = s->size;
    stream_free(s);
    return n;
}

/* stream.reduce(identity, accumulator) */
int stream_reduce(IntStream *s, int identity, int (*fn)(int,int)) {
    int acc = identity;
    for (int i = 0; i < s->size; i++) acc = fn(acc, s->data[i]);
    stream_free(s);
    return acc;
}

/* stream.sum() — mapToInt().sum() */
int stream_sum(IntStream *s) {
    int total = 0;
    for (int i = 0; i < s->size; i++) total += s->data[i];
    stream_free(s);
    return total;
}

/* stream.min() */
int stream_min(IntStream *s) {
    if (s->size == 0) { stream_free(s); return 0; }
    int m = s->data[0];
    for (int i = 1; i < s->size; i++) if (s->data[i] < m) m = s->data[i];
    stream_free(s);
    return m;
}

/* stream.max() */
int stream_max(IntStream *s) {
    if (s->size == 0) { stream_free(s); return 0; }
    int m = s->data[0];
    for (int i = 1; i < s->size; i++) if (s->data[i] > m) m = s->data[i];
    stream_free(s);
    return m;
}

/* stream.anyMatch(pred) */
int stream_any_match(IntStream *s, int (*pred)(int)) {
    for (int i = 0; i < s->size; i++) if (pred(s->data[i])) { stream_free(s); return 1; }
    stream_free(s);
    return 0;
}

/* stream.allMatch(pred) */
int stream_all_match(IntStream *s, int (*pred)(int)) {
    for (int i = 0; i < s->size; i++) if (!pred(s->data[i])) { stream_free(s); return 0; }
    stream_free(s);
    return 1;
}

/* stream.findFirst() — returns first or -1 */
int stream_find_first(IntStream *s) {
    int v = s->size > 0 ? s->data[0] : -1;
    stream_free(s);
    return v;
}

/* collect to array */
int *stream_collect(IntStream *s, int *out_len) {
    *out_len = s->size;
    int *arr = malloc(s->size * sizeof(int));
    memcpy(arr, s->data, s->size * sizeof(int));
    stream_free(s);
    return arr;
}

/* ═══════════════════════════════════════════════════════════
 *  LAMBDA FUNCTIONS FOR USE WITH STREAM
 * ═══════════════════════════════════════════════════════════ */
int fn_double(int x)     { return x * 2; }
int fn_square(int x)     { return x * x; }
int fn_add_one(int x)    { return x + 1; }
int pred_even(int x)     { return x % 2 == 0; }
int pred_positive(int x) { return x > 0; }
int pred_gt_5(int x)     { return x > 5; }
int fn_sum(int a, int b) { return a + b; }
int fn_max(int a, int b) { return a > b ? a : b; }
void print_item(int x)   { printf("    %d\n", x); }

/* ═══════════════════════════════════════════════════════════
 *  DEMOS — each mirrors a Java stream example
 * ═══════════════════════════════════════════════════════════ */

void demo_basic_pipeline(void) {
    printf("━━━ 1. Basic Pipeline: filter → map → forEach ━━━━━━━━\n");
    printf("Java:  List.of(1,2,3,4,5,6,7,8,9,10).stream()\n");
    printf("           .filter(n -> n%%2==0)\n");
    printf("           .map(n -> n*n)\n");
    printf("           .forEach(System.out::println)\n\n");

    int nums[] = {1,2,3,4,5,6,7,8,9,10};
    IntStream s0 = stream_of(nums, 10);
    stream_print(&s0, "source:");

    IntStream s1 = stream_filter(&s0, pred_even);
    stream_print(&s1, "filter(even):");
    stream_free(&s0);

    IntStream s2 = stream_map(&s1, fn_square);
    stream_print(&s2, "map(square):");
    stream_free(&s1);

    printf("  forEach:\n");
    stream_foreach(&s2, print_item);  /* s2 freed inside */
    printf("\n");
}

void demo_reduce(void) {
    printf("━━━ 2. Reduce / Sum / Min / Max ━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  numbers.stream().reduce(0, Integer::sum)\n\n");

    int nums[] = {3,1,4,1,5,9,2,6,5,3,5};
    int n = 11;

    IntStream s0 = stream_of(nums, n);
    printf("  source: "); stream_print(&s0, "");

    IntStream s1 = stream_of(nums, n);
    printf("  sum     = %d\n", stream_sum(&s1));

    IntStream s2 = stream_of(nums, n);
    printf("  max     = %d\n", stream_max(&s2));

    IntStream s3 = stream_of(nums, n);
    printf("  min     = %d\n", stream_min(&s3));

    IntStream s4 = stream_of(nums, n);
    printf("  count   = %d\n", stream_count(&s4));

    IntStream s5 = stream_of(nums, n);
    printf("  reduce(0, max) = %d\n", stream_reduce(&s5, 0, fn_max));

    stream_free(&s0);
    printf("\n");
}

void demo_sorted_distinct(void) {
    printf("━━━ 3. sorted / distinct / limit / skip ━━━━━━━━━━━━━━\n");

    int nums[] = {5,3,1,4,2,3,5,1,4};
    IntStream s = stream_of(nums, 9);
    stream_print(&s, "source:");

    IntStream sorted = stream_sorted(&s);
    stream_print(&sorted, "sorted:");
    stream_free(&s);

    IntStream distinct = stream_distinct(&sorted);
    stream_print(&distinct, "distinct:");
    stream_free(&sorted);

    IntStream limited = stream_limit(&distinct, 3);
    stream_print(&limited, "limit(3):");
    stream_free(&distinct);

    stream_free(&limited);

    /* Skip demo */
    IntStream base = stream_range(1, 11);
    stream_print(&base, "range(1,11):");
    IntStream skipped = stream_skip(&base, 3);
    stream_print(&skipped, "skip(3):");
    stream_free(&base);
    stream_free(&skipped);
    printf("\n");
}

void demo_match_operations(void) {
    printf("━━━ 4. anyMatch / allMatch / findFirst ━━━━━━━━━━━━━━━\n");
    printf("Java:  numbers.stream().anyMatch(n -> n > 5)\n\n");

    int nums[] = {1, 3, 5, 7, 9};

    IntStream s1 = stream_of(nums, 5);
    printf("  {1,3,5,7,9}.anyMatch(n>5) = %s\n",
           stream_any_match(&s1, pred_gt_5) ? "true" : "false");

    IntStream s2 = stream_of(nums, 5);
    printf("  {1,3,5,7,9}.allMatch(n>0) = %s\n",
           stream_all_match(&s2, pred_positive) ? "true" : "false");

    IntStream s3 = stream_of(nums, 5);
    IntStream evens = stream_filter(&s3, pred_even);
    stream_free(&s3);
    printf("  {1,3,5,7,9}.filter(even).findFirst() = %d  (empty=-1)\n",
           stream_find_first(&evens));

    int nums2[] = {1,2,3,4,5};
    IntStream s4 = stream_of(nums2, 5);
    IntStream evens2 = stream_filter(&s4, pred_even);
    stream_free(&s4);
    printf("  {1,2,3,4,5}.filter(even).findFirst() = %d\n",
           stream_find_first(&evens2));
    printf("\n");
}

void demo_complex_pipeline(void) {
    printf("━━━ 5. Complex Pipeline ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java:  IntStream.range(1,21).stream()\n");
    printf("           .filter(n -> n%%3==0)\n");
    printf("           .map(n -> n*n)\n");
    printf("           .sorted()\n");
    printf("           .limit(5)\n");
    printf("           .reduce(0, Integer::sum)\n\n");

    IntStream s0 = stream_range(1, 21);
    stream_print(&s0, "range(1,21):");

    /* filter: divisible by 3 */
    IntStream s1 = stream_filter(&s0, (int(*)(int))(void*)({
        /* GCC nested function — filter div by 3 */
        static int fn(int x){ return x%3==0; }
        fn;
    }));
    /* Use a named function instead: */
    stream_free(&s1);
    s0 = stream_range(1, 21);

    /* Manually: divisible by 3 */
    int div3_count = 0;
    int *temp = malloc(20 * sizeof(int));
    for (int i = 1; i <= 20; i++) if (i%3==0) temp[div3_count++] = i;
    IntStream s_div3; s_div3.data = temp; s_div3.size = div3_count; s_div3.owned = 1;
    stream_print(&s_div3, "filter(%%3==0):");

    IntStream s_sq = stream_map(&s_div3, fn_square);
    stream_print(&s_sq, "map(square):");
    stream_free(&s_div3);

    IntStream s_sorted = stream_sorted(&s_sq);
    stream_print(&s_sorted, "sorted:");
    stream_free(&s_sq);

    IntStream s_lim = stream_limit(&s_sorted, 5);
    stream_print(&s_lim, "limit(5):");
    stream_free(&s_sorted);

    int result = stream_sum(&s_lim);
    printf("  sum = %d\n", result);

    stream_free(&s0);
    printf("\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  02_stream_pipeline.c — Java Stream API in C        ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_basic_pipeline();
    demo_reduce();
    demo_sorted_distinct();
    demo_match_operations();

    printf("━━━ Key Concepts ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Source  → creates initial data\n");
    printf("  Intermediate → builds chain, returns new stream\n");
    printf("  Terminal → triggers execution, consumes stream\n\n");
    printf("  Java streams are LAZY:\n");
    printf("    filter().map().sorted() → nothing runs yet!\n");
    printf("    .collect()              → NOW it runs all ops\n\n");
    printf("  Our C version is EAGER (runs immediately)\n");
    printf("  For lazy evaluation, you'd need a linked list of ops\n");
    printf("  and run them when terminal is called\n");
    return 0;
}
