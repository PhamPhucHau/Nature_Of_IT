/*
 * volatile_visibility.c  —  Java Memory Model: volatile & Memory Barriers in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o volatile_visibility volatile_visibility.c
 * Run:    ./volatile_visibility
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   The Java Memory Model (JMM): specifically how volatile and
 *   synchronized prevent memory visibility bugs across CPU cores.
 *
 *   KEY INSIGHT:
 *     Without volatile/synchronized, changes in one thread
 *     may not be visible to other threads.
 *     This is NOT a Java bug — it is CPU architecture behavior.
 *     The JVM translates volatile into CPU memory barrier instructions.
 *
 *   REAL JVM BEHAVIOR:
 *     volatile write → stores to L1 cache, then MFENCE instruction
 *                      (x86) or dmb (ARM) → all caches synchronized
 *     volatile read  → LFENCE + read from memory (not stale cache)
 *
 *   C simulation: we use _Atomic / __sync_synchronize() to model
 *   the same memory barrier effects.
 *
 *   THREAD SAFETY DEMONSTRATIONS:
 *     1. Race condition (no synchronization) → wrong count
 *     2. volatile flag (visibility only) → correct flag, wrong count
 *     3. atomic int (CAS operations) → correct count
 *     4. Happens-before chain simulation
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>   /* C11 atomics — models Java volatile/Atomic */

/* ═══════════════════════════════════════════════════════════
 *  MEMORY BARRIER MACROS
 *  Java volatile write/read map to these CPU instructions.
 *
 *  x86: MFENCE (full fence), SFENCE (store), LFENCE (load)
 *  ARM: dmb ish (full), dmb ishst (store), dmb ishld (load)
 *
 *  __sync_synchronize() is a GCC full memory barrier.
 * ═══════════════════════════════════════════════════════════ */

/* Java: volatile write → write + STORE FENCE */
#define VOLATILE_WRITE(var, val)  do { (var) = (val); __sync_synchronize(); } while(0)

/* Java: volatile read → LOAD FENCE + read */
#define VOLATILE_READ(var)        ({ __sync_synchronize(); (var); })

/* ═══════════════════════════════════════════════════════════
 *  DEMO 1: RACE CONDITION — no synchronization
 *
 *  Java equivalent:
 *    class Counter { int count = 0; }  // NOT atomic, NOT volatile
 *    counter.count++;                  // read-modify-write is NOT atomic
 *
 *  Problem: counter++ is 3 operations:
 *    1. LOAD count → register
 *    2. ADD register, 1
 *    3. STORE register → count
 *    Two threads can both LOAD the same value → one increment is lost
 * ═══════════════════════════════════════════════════════════ */

static volatile int race_counter = 0;  /* shared, non-atomic counter */

static void* race_thread(void *arg) {
    int increments = *(int*)arg;
    for (int i = 0; i < increments; i++) {
        /* BAD: this is NOT atomic! */
        /* load → increment → store: can be interrupted between any step */
        race_counter++;
    }
    return NULL;
}

void demo_race_condition(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Race Condition — counter++ without synchronization\n");
    printf("Java: int count = 0; count++;  // NOT thread-safe!\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int increments_per_thread = 100000;
    int num_threads = 4;
    int expected = increments_per_thread * num_threads;

    race_counter = 0;
    pthread_t threads[4];

    printf("  %d threads each incrementing %d times\n",
           num_threads, increments_per_thread);
    printf("  Expected: %d\n", expected);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, race_thread, &increments_per_thread);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("  Actual:   %d\n", race_counter);
    printf("  Lost increments: %d\n\n", expected - race_counter);

    printf("  WHY: counter++ is READ → MODIFY → WRITE\n");
    printf("       Two threads can READ the same value simultaneously\n");
    printf("       Both write back the same incremented value\n");
    printf("       One increment is LOST\n");
    printf("  FIX: synchronized(lock){ count++; }  or  AtomicInteger.incrementAndGet()\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 2: ATOMIC COUNTER — thread-safe using CAS
 *
 *  Java equivalent: AtomicInteger count = new AtomicInteger(0);
 *                   count.incrementAndGet();
 *
 *  CAS (Compare-And-Swap):
 *    atomic operation: if(current == expected) { current = new; return true; }
 *    On x86: LOCK CMPXCHG instruction (single bus-locked operation)
 *    On ARM: LDREX/STREX load-exclusive/store-exclusive pair
 *
 *  Loop-based CAS (same as Java's AtomicInteger):
 *    int old, new;
 *    do {
 *        old = count;          // read current
 *        new = old + 1;        // compute desired
 *    } while (!CAS(&count, old, new));  // retry if changed
 * ═══════════════════════════════════════════════════════════ */

static atomic_int atomic_counter = 0;  /* Java: AtomicInteger */

static void* atomic_thread(void *arg) {
    int increments = *(int*)arg;
    for (int i = 0; i < increments; i++) {
        /* atomic_fetch_add = Java AtomicInteger.getAndIncrement()
         * Implemented as LOCK XADD on x86 — single bus-locked instruction */
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}

void demo_atomic_counter(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Atomic Counter — thread-safe using CAS\n");
    printf("Java: AtomicInteger count = new AtomicInteger(0);\n");
    printf("      count.incrementAndGet();\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int increments_per_thread = 100000;
    int num_threads = 4;
    int expected = increments_per_thread * num_threads;

    atomic_counter = 0;
    pthread_t threads[4];

    printf("  %d threads each doing atomic_fetch_add %d times\n",
           num_threads, increments_per_thread);
    printf("  Expected: %d\n", expected);

    for (int i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, atomic_thread, &increments_per_thread);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("  Actual:   %d\n", atomic_counter);
    printf("  Lost increments: %d\n\n", expected - atomic_counter);

    printf("  WHY: atomic_fetch_add = LOCK XADD (x86) = single bus-locked op\n");
    printf("       No other thread can interrupt between read and write\n");
    printf("       Same as Java: AtomicInteger.getAndIncrement() → always correct\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 3: VOLATILE FLAG — producer/consumer visibility
 *
 *  Java equivalent:
 *    volatile boolean ready = false;
 *    volatile Object data = null;
 *
 *  The critical happens-before guarantee:
 *    1. producer: data = computeData();   // write data
 *    2. producer: ready = true;           // volatile write (barrier)
 *    3. consumer: while(!ready){}        // volatile read (barrier)
 *    4. consumer: use(data);              // read data
 *
 *  Happens-before chain: 1 HB 2 HB 3 HB 4
 *  → data write is visible when consumer reads it
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int          data_value;    /* the "data" being produced   */
    atomic_int   ready;         /* "volatile boolean ready"    */
    atomic_int   consumed;      /* confirmation consumer got it*/
} SharedData;

static SharedData g_shared = {0, 0, 0};

static void* producer_thread(void *arg) {
    (void)arg;
    printf("  [Producer] Computing data...\n");

    /* Simulate work */
    int result = 42 * 1000;   /* some computation */

    /* Step 1: write data BEFORE setting ready */
    g_shared.data_value = result;     /* write data first */

    printf("  [Producer] Data ready: %d\n", result);
    printf("  [Producer] Issuing memory barrier (volatile write)...\n");

    /* Step 2: volatile write to ready — acts as memory barrier */
    /* Java: ready = true; (volatile)
     * CPU: flush write buffer → ensure data_value write is visible
     *      before ready = true write completes */
    atomic_store_explicit(&g_shared.ready, 1, memory_order_release);
    /* memory_order_release = Java volatile write (store fence) */

    printf("  [Producer] Flag set, data is visible to consumer\n");
    return NULL;
}

static void* consumer_thread(void *arg) {
    (void)arg;
    printf("  [Consumer] Waiting for data...\n");

    /* Spin until ready flag is set */
    /* Java: while (!ready) {}  (volatile read)
     * CPU: each read re-fetches from memory (not stale cache) */
    while (atomic_load_explicit(&g_shared.ready, memory_order_acquire) == 0) {
        /* memory_order_acquire = Java volatile read (load fence) */
        /* busy wait */
    }

    /* After volatile read, data_value is guaranteed visible */
    printf("  [Consumer] Flag seen! Reading data: %d\n", g_shared.data_value);

    int expected = 42 * 1000;
    if (g_shared.data_value == expected) {
        printf("  [Consumer] ✓ Data is correct and complete!\n");
    } else {
        printf("  [Consumer] ✗ Data is WRONG (visibility failure!)\n");
    }
    atomic_store(&g_shared.consumed, 1);
    return NULL;
}

void demo_volatile_flag(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: volatile Flag — Producer/Consumer Visibility\n");
    printf("Java: volatile boolean ready; volatile Object data;\n");
    printf("      Producer writes data, sets ready. Consumer spins on ready.\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    g_shared.data_value = 0;
    g_shared.ready      = 0;
    g_shared.consumed   = 0;

    pthread_t prod_tid, cons_tid;

    /* Start consumer first — it will wait */
    pthread_create(&cons_tid, NULL, consumer_thread, NULL);
    pthread_create(&prod_tid, NULL, producer_thread, NULL);

    pthread_join(prod_tid, NULL);
    pthread_join(cons_tid, NULL);

    printf("\n  KEY: volatile write (release) HB volatile read (acquire)\n");
    printf("       → Everything written BEFORE the release is visible\n");
    printf("         AFTER the acquire. This is the JMM happens-before rule.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 4: DOUBLE-CHECKED LOCKING — classic volatile mistake
 *
 *  Java: the infamous singleton pattern
 *    Without volatile: partially constructed object may be visible
 *    With volatile:    correct because of happens-before
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int  initialized;
    int  value;
    char name[32];
} Singleton;

static atomic_uintptr_t singleton_instance = 0;  /* volatile pointer */
static pthread_mutex_t  singleton_mutex = PTHREAD_MUTEX_INITIALIZER;

static Singleton* get_singleton(void) {
    /* First check: volatile read — fast path (no lock) */
    Singleton *inst = (Singleton*)atomic_load_explicit(
        &singleton_instance, memory_order_acquire);

    if (inst == NULL) {
        /* Slow path: take the lock */
        pthread_mutex_lock(&singleton_mutex);

        /* Second check: another thread may have created it while we waited */
        inst = (Singleton*)atomic_load(&singleton_instance);

        if (inst == NULL) {
            /* Create the singleton */
            inst = malloc(sizeof(Singleton));
            inst->initialized = 1;
            inst->value = 999;
            strncpy(inst->name, "ConfigSingleton", 31);

            /* Java: instance = new Singleton();  (volatile write)
             * memory_order_release ensures all fields written above
             * are visible to any thread that reads the pointer */
            atomic_store_explicit(
                &singleton_instance, (uintptr_t)inst, memory_order_release);

            printf("  [Singleton] Created: value=%d, name=%s\n",
                   inst->value, inst->name);
        } else {
            printf("  [Singleton] Another thread already created it\n");
        }

        pthread_mutex_unlock(&singleton_mutex);
    }

    return inst;
}

static void* singleton_thread(void *arg) {
    int tid = *(int*)arg;
    Singleton *s = get_singleton();
    printf("  [Thread %d] Got singleton: value=%d initialized=%d\n",
           tid, s->value, s->initialized);
    return NULL;
}

void demo_double_checked_locking(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Double-Checked Locking (DCL) — Thread-safe Singleton\n");
    printf("Java: private static volatile Singleton instance;\n");
    printf("      Must be volatile or partially constructed object visible!\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    singleton_instance = 0;

    pthread_t threads[4];
    int ids[] = {0, 1, 2, 3};

    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, singleton_thread, &ids[i]);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    Singleton *s = (Singleton*)atomic_load(&singleton_instance);
    if (s) {
        free(s);
        atomic_store(&singleton_instance, 0);
    }

    printf("\n  KEY: Without volatile, 'instance' assignment (pointer write)\n");
    printf("       may be visible to other threads BEFORE all fields are initialized.\n");
    printf("       volatile write = memory barrier = all prior writes are visible first.\n");
    printf("\n  Spring Bean: @Scope('singleton') beans are created once and cached.\n");
    printf("               The ApplicationContext ensures publication safety.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  volatile_visibility.c — Java Memory Model in C     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_race_condition();
    demo_atomic_counter();
    demo_volatile_flag();
    demo_double_checked_locking();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Race condition: read-modify-write is NOT atomic without synchronization\n");
    printf("  volatile: visibility + ordering guarantee (NOT mutual exclusion)\n");
    printf("  Atomic (CAS): lock-free counter, compare-and-swap\n");
    printf("  volatile write = STORE FENCE = flush to RAM before proceeding\n");
    printf("  volatile read  = LOAD FENCE  = re-fetch from RAM (skip cache)\n");
    printf("  happens-before: volatile write HB volatile read = safe communication\n");
    return 0;
}
