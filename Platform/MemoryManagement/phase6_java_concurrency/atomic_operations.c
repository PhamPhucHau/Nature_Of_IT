/*
 * atomic_operations.c  —  Java Atomic Classes in C (CAS, AtomicInteger)
 *
 * Build:  gcc -Wall -Wextra -pthread -o atomic_operations atomic_operations.c
 * Run:    ./atomic_operations
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   java.util.concurrent.atomic.*
 *     AtomicInteger, AtomicLong, AtomicReference, LongAdder
 *
 *   All atomic classes use CAS (Compare-And-Swap) internally.
 *   CAS is a hardware instruction (x86: LOCK CMPXCHG) that:
 *     1. Reads current value atomically
 *     2. Compares to expected
 *     3. Only writes new value if current == expected
 *     4. Returns success/failure
 *     ALL in a single uninterruptible CPU instruction.
 *
 *   CAS is the foundation of:
 *     - lock-free data structures
 *     - lock-free counters
 *     - non-blocking algorithms
 *
 *   KEY INSIGHT:
 *     synchronized blocks ALL other threads.
 *     CAS (atomic) only RETRIES when specifically THIS value changed.
 *     Under low contention: atomic >> synchronized (no OS involvement).
 *     Under HIGH contention: atomic spins → LongAdder spreads load.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

/* ═══════════════════════════════════════════════════════════
 *  DEMO 1: CAS Loop — the heart of AtomicInteger
 *
 *  Java AtomicInteger.incrementAndGet():
 *    do {
 *        current = get();
 *        next    = current + 1;
 *    } while (!compareAndSet(current, next));
 *    return next;
 *
 *  C equivalent using C11 atomics:
 *    atomic_compare_exchange_weak()
 *
 *  x86 assembly: LOCK CMPXCHG  ← single bus-locked instruction
 *  ARM assembly: LDXR/STXR pair with exclusion monitor
 * ═══════════════════════════════════════════════════════════ */

/* Simulate AtomicInteger.incrementAndGet() */
static int atomic_increment_get(atomic_int *counter) {
    int expected, desired;
    int cas_retries = 0;

    do {
        expected = atomic_load_explicit(counter, memory_order_relaxed);
        desired  = expected + 1;
        cas_retries++;
        /* atomic_compare_exchange_weak:
         *   if(*counter == expected) { *counter = desired; return true; }
         *   else { expected = *counter; return false; }
         * Java: compareAndSet(expected, desired)
         * x86:  LOCK CMPXCHG
         */
    } while (!atomic_compare_exchange_weak_explicit(
                 counter,
                 &expected,
                 desired,
                 memory_order_acq_rel,    /* success: full barrier  */
                 memory_order_relaxed));  /* failure: just reload   */

    if (cas_retries > 1) {
        /* This means another thread modified the counter between our load and CAS */
        /* We had to retry — this is the "spinning" cost of CAS under contention  */
    }
    return desired;
}

/* Thread function: each thread increments the counter N times */
static atomic_int  g_atomic_counter = 0;
static int g_increment_count;

static void* cas_worker(void *arg) {
    int tid = *(int*)arg;
    (void)tid;

    for (int i = 0; i < g_increment_count; i++) {
        atomic_increment_get(&g_atomic_counter);
    }
    return NULL;
}

void demo_cas_counter(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: CAS-based Atomic Counter (AtomicInteger)\n");
    printf("Java: AtomicInteger count = new AtomicInteger(0);\n");
    printf("      count.incrementAndGet();  // LOCK CMPXCHG on x86\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int num_threads = 4;
    g_increment_count = 250000;
    int expected = g_increment_count * num_threads;

    pthread_t threads[4];
    int ids[] = {0,1,2,3};

    atomic_store(&g_atomic_counter, 0);

    printf("  %d threads × %d increments = expected %d\n\n",
           num_threads, g_increment_count, expected);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, cas_worker, &ids[i]);
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    int actual = atomic_load(&g_atomic_counter);
    printf("  Result: %d  (expected %d)  %s\n\n", actual, expected,
           actual == expected ? "✓ CORRECT" : "✗ WRONG");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 2: AtomicReference CAS — thread-safe pointer update
 *
 *  Java: AtomicReference<Node> head = new AtomicReference<>(null);
 *        head.compareAndSet(oldHead, newHead);
 *
 *  Use case: lock-free stack / linked list
 * ═══════════════════════════════════════════════════════════ */

typedef struct Node {
    int         value;
    struct Node *next;
} Node;

/* Lock-free stack using AtomicReference for head pointer */
typedef struct {
    atomic_uintptr_t head;  /* AtomicReference<Node> head */
    atomic_int       size;
} LockFreeStack;

static void stack_init(LockFreeStack *s) {
    atomic_store(&s->head, (uintptr_t)NULL);
    atomic_store(&s->size, 0);
}

/* Lock-free push: prepend to linked list using CAS */
static void stack_push(LockFreeStack *s, int value) {
    Node *new_node = malloc(sizeof(Node));
    new_node->value = value;

    uintptr_t old_head;
    do {
        /* Read current head */
        old_head = atomic_load(&s->head);
        new_node->next = (Node*)old_head;

        /* Try to make new_node the head:
         * CAS: if(head == old_head) { head = new_node; return true; }
         * If another thread pushed between our read and CAS → retry */
    } while (!atomic_compare_exchange_weak(&s->head,
                                            &old_head,
                                            (uintptr_t)new_node));

    atomic_fetch_add(&s->size, 1);
}

/* Lock-free pop */
static int stack_pop(LockFreeStack *s, int *value) {
    uintptr_t old_head;
    Node *head_node;

    do {
        old_head  = atomic_load(&s->head);
        head_node = (Node*)old_head;
        if (!head_node) return 0;  /* empty */
        /* Try to advance head to next */
    } while (!atomic_compare_exchange_weak(&s->head,
                                            &old_head,
                                            (uintptr_t)head_node->next));

    *value = head_node->value;
    free(head_node);
    atomic_fetch_sub(&s->size, 1);
    return 1;
}

void demo_lock_free_stack(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Lock-Free Stack (AtomicReference CAS)\n");
    printf("Java: AtomicReference<Node> head = new AtomicReference<>();\n");
    printf("      head.compareAndSet(expected, newHead);\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    LockFreeStack stack;
    stack_init(&stack);

    printf("  Pushing 1, 2, 3, 4, 5...\n");
    for (int i = 1; i <= 5; i++) stack_push(&stack, i);
    printf("  Stack size: %d\n\n", atomic_load(&stack.size));

    printf("  Popping:\n");
    int val;
    while (stack_pop(&stack, &val)) {
        printf("    popped: %d\n", val);
    }
    printf("\n  Lock-free: no mutex needed! Pure CAS operations.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 3: CountDownLatch simulation
 *  Java: CountDownLatch latch = new CountDownLatch(N);
 *        latch.await();    // blocks until count reaches 0
 *        latch.countDown(); // decrement count
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    atomic_int       count;      /* starts at N, decrements to 0 */
    pthread_mutex_t  mutex;
    pthread_cond_t   cond;       /* signaled when count reaches 0 */
} CountDownLatch;

static void latch_init(CountDownLatch *l, int count) {
    atomic_store(&l->count, count);
    pthread_mutex_init(&l->mutex, NULL);
    pthread_cond_init(&l->cond, NULL);
}

static void latch_count_down(CountDownLatch *l) {
    int remaining = atomic_fetch_sub(&l->count, 1) - 1;
    if (remaining == 0) {
        /* Last countdown — wake all awaiting threads */
        pthread_mutex_lock(&l->mutex);
        pthread_cond_broadcast(&l->cond);
        pthread_mutex_unlock(&l->mutex);
    }
}

static void latch_await(CountDownLatch *l) {
    pthread_mutex_lock(&l->mutex);
    while (atomic_load(&l->count) > 0) {
        pthread_cond_wait(&l->cond, &l->mutex);
    }
    pthread_mutex_unlock(&l->mutex);
}

typedef struct {
    CountDownLatch *latch;
    int             service_id;
    int             startup_ms;
} ServiceArgs;

static void* start_service(void *arg) {
    ServiceArgs *sa = (ServiceArgs*)arg;
    usleep(sa->startup_ms * 1000);
    printf("  [Service %d] Started (took %d ms)\n",
           sa->service_id, sa->startup_ms);
    latch_count_down(sa->latch);  /* latch.countDown() */
    return NULL;
}

void demo_countdown_latch(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: CountDownLatch — Wait for N Services to Start\n");
    printf("Java: latch.await(); // block until all services ready\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int num_services = 3;
    CountDownLatch latch;
    latch_init(&latch, num_services);

    pthread_t threads[3];
    ServiceArgs args[] = {
        {&latch, 1, 80},    /* service 1 takes 80ms */
        {&latch, 2, 150},   /* service 2 takes 150ms */
        {&latch, 3, 50},    /* service 3 takes 50ms */
    };

    printf("  Starting %d services in parallel...\n", num_services);
    for (int i = 0; i < num_services; i++)
        pthread_create(&threads[i], NULL, start_service, &args[i]);

    printf("  [Main] Waiting for all services (latch.await())...\n");
    latch_await(&latch);   /* blocks until count reaches 0 */
    printf("  [Main] All %d services ready! Sending traffic.\n\n", num_services);

    for (int i = 0; i < num_services; i++) pthread_join(threads[i], NULL);
    pthread_mutex_destroy(&latch.mutex);
    pthread_cond_destroy(&latch.cond);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  atomic_operations.c — Java Atomics in C            ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_cas_counter();
    demo_lock_free_stack();
    demo_countdown_latch();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  CAS = Compare-And-Swap: single hardware instruction, no lock\n");
    printf("  AtomicInteger = CAS loop until increment succeeds\n");
    printf("  Lock-free = no mutex → no blocking, no context switching\n");
    printf("  CountDownLatch = barrier: wait until N tasks complete\n");
    printf("  High contention CAS → spinning → use LongAdder or synchronized\n");
    return 0;
}
