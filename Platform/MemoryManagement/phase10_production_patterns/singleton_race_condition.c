/*
 * singleton_race_condition.c  —  Spring Singleton Race Conditions in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o singleton_race singleton_race_condition.c
 * Run:    ./singleton_race
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   The most common Spring Boot concurrency bug:
 *   A @Service singleton bean with MUTABLE INSTANCE FIELDS
 *   being accessed by multiple request threads simultaneously.
 *
 *   Spring @Service is singleton scope by default.
 *   ONE instance shared across ALL Tomcat request threads.
 *   Mutable fields = race conditions unless properly synchronized.
 *
 *   This file demonstrates:
 *     1. BUGGY: ArrayList as instance field → corruption
 *     2. FIXED: CopyOnWriteArrayList / ConcurrentHashMap / AtomicInteger
 *     3. BEST:  Stateless design (no mutable instance fields)
 *
 *   AND: Connection pool starvation simulation
 *        (Tomcat threads >> HikariCP connections)
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

/* ═══════════════════════════════════════════════════════════
 *  DEMO 1: Buggy Singleton — mutable ArrayList without sync
 *
 *  Java equivalent (BUGGY):
 *    @Service
 *    class UserService {
 *        List<String> log = new ArrayList<>();  // ← DANGER!
 *
 *        public void process(String user) {
 *            log.add(user);  // concurrent add → corruption
 *        }
 *    }
 * ═══════════════════════════════════════════════════════════ */

/* Simulates a non-thread-safe list (like Java ArrayList) */
#define LIST_CAPACITY 256

typedef struct {
    char  items[LIST_CAPACITY][32];
    int   size;
    /* NO lock — this is the bug */
} UnsafeList;

static UnsafeList g_buggy_log = { .size = 0 };

static void unsafe_list_add(UnsafeList *list, const char *item) {
    /* BUG: this is a race condition:
     * Thread A reads size=5, Thread B reads size=5
     * Both write to items[5], both increment size
     * size becomes 6 but one entry is overwritten!
     * Or size increment is non-atomic → lost update */
    if (list->size < LIST_CAPACITY) {
        strncpy(list->items[list->size], item, 31); /* step 1: write item */
        /* RACE: between step 1 and step 2, another thread may also write items[size] */
        list->size++;                                /* step 2: increment */
    }
}

static void* buggy_service_thread(void *arg) {
    int tid = *(int*)arg;
    for (int i = 0; i < 50; i++) {
        char entry[32];
        snprintf(entry, sizeof(entry), "T%d-entry%d", tid, i);
        unsafe_list_add(&g_buggy_log, entry);
    }
    return NULL;
}

void demo_buggy_singleton(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Buggy Singleton — mutable ArrayList race condition\n");
    printf("Java: @Service class has 'List<String> log = new ArrayList<>()'\n");
    printf("      Multiple request threads add to log simultaneously\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int num_threads = 4;
    int expected = 50 * num_threads;
    pthread_t threads[4];
    int ids[] = {0,1,2,3};

    g_buggy_log.size = 0;

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, buggy_service_thread, &ids[i]);
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    printf("  Expected entries: %d\n", expected);
    printf("  Actual entries:   %d\n", g_buggy_log.size);
    if (g_buggy_log.size != expected) {
        printf("  ✗ LOST %d entries due to race condition!\n\n", expected - g_buggy_log.size);
    } else {
        printf("  ~ Happened to get correct count (race is non-deterministic)\n\n");
    }

    /* Check for duplicate entries (another symptom of the race) */
    int empty_slots = 0;
    for (int i = 0; i < g_buggy_log.size; i++) {
        if (g_buggy_log.items[i][0] == '\0') empty_slots++;
    }
    printf("  Empty/corrupted slots: %d\n\n", empty_slots);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 2: Fixed Singleton — atomic counter + concurrent list
 *
 *  Java fix:
 *    AtomicInteger requestCount = new AtomicInteger();
 *    CopyOnWriteArrayList<String> log = new CopyOnWriteArrayList<>();
 * ═══════════════════════════════════════════════════════════ */

/* Thread-safe list: lock on every write */
typedef struct {
    char            items[LIST_CAPACITY][32];
    int             size;
    pthread_mutex_t lock;  /* guards: size + items */
} SafeList;

static SafeList    g_safe_log     = { .size = 0 };
static atomic_int  g_request_count = 0;   /* Java: AtomicInteger */

static void safe_list_init(SafeList *list) {
    list->size = 0;
    pthread_mutex_init(&list->lock, NULL);
}

static void safe_list_add(SafeList *list, const char *item) {
    pthread_mutex_lock(&list->lock);
    if (list->size < LIST_CAPACITY) {
        strncpy(list->items[list->size], item, 31);
        list->size++;
    }
    pthread_mutex_unlock(&list->lock);
}

static void* fixed_service_thread(void *arg) {
    int tid = *(int*)arg;
    for (int i = 0; i < 50; i++) {
        /* Atomic request count — thread safe */
        int req_num = atomic_fetch_add(&g_request_count, 1);
        (void)req_num;

        char entry[32];
        snprintf(entry, sizeof(entry), "T%d-entry%d", tid, i);
        safe_list_add(&g_safe_log, entry);  /* thread-safe add */
    }
    return NULL;
}

void demo_fixed_singleton(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Fixed Singleton — AtomicInteger + thread-safe list\n");
    printf("Java: AtomicInteger count; CopyOnWriteArrayList<> log;\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int num_threads = 4;
    int expected = 50 * num_threads;
    pthread_t threads[4];
    int ids[] = {0,1,2,3};

    safe_list_init(&g_safe_log);
    atomic_store(&g_request_count, 0);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, fixed_service_thread, &ids[i]);
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    printf("  Expected entries:   %d\n", expected);
    printf("  Actual entries:     %d\n", g_safe_log.size);
    printf("  Total requests:     %d\n", atomic_load(&g_request_count));
    printf("  %s\n\n",
           g_safe_log.size == expected ? "✓ CORRECT — no lost entries" : "✗ WRONG");

    pthread_mutex_destroy(&g_safe_log.lock);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 3: BEST — Stateless Bean (no mutable instance state)
 *
 *  Java best practice:
 *    @Service class UserService {
 *        // NO instance fields that change after construction
 *        // All logic uses parameters and method locals
 *        public String process(String input) {
 *            String result = transform(input);  // local only
 *            return result;
 *        }
 *    }
 *
 *  Stateless = ZERO synchronization needed = maximum throughput
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int  result;
    char message[64];
} ProcessResult;

/* Stateless function — no shared state touched */
static ProcessResult stateless_process(const char *username, int user_id) {
    ProcessResult r;
    r.result = user_id * 42;   /* computation using only local data + params */
    snprintf(r.message, sizeof(r.message),
             "Processed %s → result=%d", username, r.result);
    return r;  /* returns value (on stack) — no heap allocation needed */
}

static void* stateless_thread(void *arg) {
    int tid = *(int*)arg;
    for (int i = 0; i < 100; i++) {
        char username[32];
        snprintf(username, sizeof(username), "user_%d_%d", tid, i);
        ProcessResult r = stateless_process(username, tid * 100 + i);
        (void)r;  /* use the result */
    }
    return NULL;
}

void demo_stateless_best(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: BEST PRACTICE — Stateless Spring Service\n");
    printf("No mutable instance fields → zero synchronization → max throughput\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    int num_threads = 8;
    pthread_t threads[8];
    int ids[] = {0,1,2,3,4,5,6,7};

    printf("  %d threads each calling stateless_process() 100 times...\n\n",
           num_threads);

    for (int i = 0; i < num_threads; i++)
        pthread_create(&threads[i], NULL, stateless_thread, &ids[i]);
    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    printf("  ✓ %d calls completed — NO synchronization needed\n", 100 * num_threads);
    printf("  ✓ No race conditions possible (no shared mutable state)\n");
    printf("  ✓ Perfect horizontal scaling\n\n");

    printf("  Stateless means:\n");
    printf("    1. No List/Map instance fields that change\n");
    printf("    2. All state passed as parameters\n");
    printf("    3. Dependencies are read-only (injected once at construction)\n");
    printf("    4. Autowired repos/services are themselves stateless\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 4: Connection Pool Starvation
 *  Simulates: many Tomcat threads, few DB connections
 * ═══════════════════════════════════════════════════════════ */

#define POOL_CONNECTIONS  3   /* HikariCP pool size (too small!) */
#define REQUEST_THREADS   8   /* Tomcat threads (too many for pool) */

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  available;
    int             connections_free;
    int             total_waited;
    int             timeouts;
} ConnectionPool;

static ConnectionPool g_conn_pool;

static void pool_init_small(void) {
    pthread_mutex_init(&g_conn_pool.lock, NULL);
    pthread_cond_init(&g_conn_pool.available, NULL);
    g_conn_pool.connections_free = POOL_CONNECTIONS;
    g_conn_pool.total_waited     = 0;
    g_conn_pool.timeouts         = 0;
}

/* Acquire a connection — simulates HikariCP getConnection() */
static int pool_acquire(int tid, int timeout_ms) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += timeout_ms * 1000000L;
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec  += ts.tv_nsec / 1000000000L;
        ts.tv_nsec %= 1000000000L;
    }

    pthread_mutex_lock(&g_conn_pool.lock);

    int waited = 0;
    while (g_conn_pool.connections_free == 0) {
        waited++;
        int rc = pthread_cond_timedwait(&g_conn_pool.available,
                                         &g_conn_pool.lock, &ts);
        if (rc != 0) {
            /* Timeout — HikariCP throws SQLTimeoutException */
            printf("  [Thread %d] TIMEOUT waiting for connection (pool=%d/%d)\n",
                   tid, g_conn_pool.connections_free, POOL_CONNECTIONS);
            g_conn_pool.timeouts++;
            pthread_mutex_unlock(&g_conn_pool.lock);
            return 0;  /* failed to acquire */
        }
    }

    g_conn_pool.connections_free--;
    if (waited > 0) g_conn_pool.total_waited++;
    pthread_mutex_unlock(&g_conn_pool.lock);

    printf("  [Thread %d] Got connection (pool remaining: %d/%d)\n",
           tid, g_conn_pool.connections_free, POOL_CONNECTIONS);
    return 1;
}

/* Release a connection back to pool */
static void pool_release(int tid) {
    pthread_mutex_lock(&g_conn_pool.lock);
    g_conn_pool.connections_free++;
    pthread_cond_signal(&g_conn_pool.available);
    pthread_mutex_unlock(&g_conn_pool.lock);
    printf("  [Thread %d] Released connection (pool: %d/%d)\n",
           tid, g_conn_pool.connections_free, POOL_CONNECTIONS);
}

typedef struct { int tid; } ConnArgs;

static void* db_worker(void *arg) {
    ConnArgs *ca = (ConnArgs*)arg;
    int ok = pool_acquire(ca->tid, 200);  /* 200ms timeout */
    if (ok) {
        usleep(100000);  /* hold connection for 100ms (simulate query) */
        pool_release(ca->tid);
    }
    free(ca);
    return NULL;
}

void demo_connection_starvation(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 4: Connection Pool Starvation\n");
    printf("Tomcat: %d threads, HikariCP: %d connections\n",
           REQUEST_THREADS, POOL_CONNECTIONS);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    pool_init_small();

    pthread_t threads[REQUEST_THREADS];

    printf("  %d threads all try to get DB connection simultaneously:\n\n",
           REQUEST_THREADS);

    for (int i = 0; i < REQUEST_THREADS; i++) {
        ConnArgs *ca = malloc(sizeof(ConnArgs));
        ca->tid = i + 1;
        pthread_create(&threads[i], NULL, db_worker, ca);
    }

    for (int i = 0; i < REQUEST_THREADS; i++) pthread_join(threads[i], NULL);

    printf("\n  Results:\n");
    printf("    Pool size:       %d connections\n", POOL_CONNECTIONS);
    printf("    Threads:         %d\n", REQUEST_THREADS);
    printf("    Threads that waited: %d\n", g_conn_pool.total_waited);
    printf("    Timeouts (503):  %d\n\n", g_conn_pool.timeouts);

    printf("  FIX: Increase HikariCP maximumPoolSize OR\n");
    printf("       Reduce Tomcat threads OR\n");
    printf("       Use virtual threads (park instead of blocking OS thread)\n\n");

    pthread_mutex_destroy(&g_conn_pool.lock);
    pthread_cond_destroy(&g_conn_pool.available);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  singleton_race_condition.c — Spring Prod Patterns  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_buggy_singleton();
    demo_fixed_singleton();
    demo_stateless_best();
    demo_connection_starvation();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Spring @Service is SINGLETON — shared across ALL threads\n");
    printf("  Mutable instance fields = race conditions\n");
    printf("  FIX 1: AtomicInteger/ConcurrentHashMap for counters/caches\n");
    printf("  FIX 2: synchronized on critical sections\n");
    printf("  BEST:  Stateless beans (no mutable state at all)\n");
    printf("  Pool starvation: HikariCP connections << Tomcat threads → timeouts\n");
    printf("  Monitor: hikaricp.connections.pending via /actuator/metrics\n");
    return 0;
}
