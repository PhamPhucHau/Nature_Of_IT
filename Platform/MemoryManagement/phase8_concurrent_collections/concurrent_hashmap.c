/*
 * concurrent_hashmap.c  —  Java ConcurrentHashMap in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o concurrent_hashmap concurrent_hashmap.c
 * Run:    ./concurrent_hashmap
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   java.util.concurrent.ConcurrentHashMap (Java 8+ implementation)
 *
 *   Key design choices:
 *     1. Lock-free READS: volatile Node pointers → no lock for gets
 *     2. Fine-grained WRITES: lock only the affected bucket
 *        synchronized(bucket_head) { insert/update }
 *     3. No global lock → many threads can write different keys simultaneously
 *
 *   Performance vs synchronized(HashMap):
 *     HashMap+synchronized: ONE writer at a time (global lock)
 *     ConcurrentHashMap:    N writers simultaneously (N = number of buckets)
 *
 *   Spring uses ConcurrentHashMap for:
 *     - Bean definitions registry (BeanDefinitionMap)
 *     - Bean singleton cache (singletonObjects)
 *     - @Cacheable default implementation
 *     - Request mapping table (HandlerMapping)
 *
 *   C SIMULATION:
 *     Each bucket has its own mutex.
 *     Reads use volatile pointers (no lock).
 *     Writes lock only the single affected bucket mutex.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>

/* ═══════════════════════════════════════════════════════════
 *  CONFIGURATION
 * ═══════════════════════════════════════════════════════════ */
#define BUCKET_COUNT  16          /* 16 buckets = 16 concurrent writers */
#define KEY_LEN       64
#define VAL_LEN       256

/* ═══════════════════════════════════════════════════════════
 *  NODE — one key-value pair in the hash chain
 *  Java: ConcurrentHashMap.Node<K,V>
 *    final int hash;
 *    final K key;
 *    volatile V val;      ← volatile for lock-free reads
 *    volatile Node next;  ← volatile for lock-free chain walk
 * ═══════════════════════════════════════════════════════════ */
typedef struct Node {
    char         key[KEY_LEN];
    char         val[VAL_LEN];
    unsigned int hash;
    struct Node *volatile next;  /* volatile = Java volatile Node next */
} Node;

/* ═══════════════════════════════════════════════════════════
 *  BUCKET — one slot in the hash table array
 *  Java: volatile Node[] table;
 *    table[i] is the head of the chain for bucket i
 *    volatile = any write to head is immediately visible to all threads
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Node *volatile   head;         /* volatile → lock-free reads */
    pthread_mutex_t  lock;         /* write lock for this bucket */
    atomic_int       chain_length; /* for load factor monitoring  */
} Bucket;

/* ═══════════════════════════════════════════════════════════
 *  CONCURRENT HASHMAP
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Bucket      buckets[BUCKET_COUNT];
    atomic_int  size;               /* total number of entries     */
    atomic_long get_ops;            /* read operations (no lock)   */
    atomic_long put_ops;            /* write operations (lock)     */
    atomic_long cas_retries;        /* CAS retry count             */
} ConcurrentHashMap;

/* ─────────────────────────────────────────────────────────────
 *  Hash function
 * ──────────────────────────────────────────────────────────── */
static unsigned int chm_hash(const char *key) {
    /* Java's String.hashCode() equivalent */
    unsigned int h = 0;
    while (*key) {
        h = h * 31 + (unsigned char)*key++;
    }
    /* Java's ConcurrentHashMap spreads hash bits to reduce bucket collisions */
    h ^= (h >> 16);
    return h;
}

static int chm_bucket_index(unsigned int hash) {
    return (int)(hash & (BUCKET_COUNT - 1));  /* fast modulo for power-of-2 */
}

/* ─────────────────────────────────────────────────────────────
 *  Initialize
 * ──────────────────────────────────────────────────────────── */
static void chm_init(ConcurrentHashMap *m) {
    memset(m, 0, sizeof(ConcurrentHashMap));
    for (int i = 0; i < BUCKET_COUNT; i++) {
        m->buckets[i].head = NULL;
        pthread_mutex_init(&m->buckets[i].lock, NULL);
    }
}

/* ─────────────────────────────────────────────────────────────
 *  GET — lock-free read
 *
 *  Java: ConcurrentHashMap.get(key)
 *  Implementation:
 *    Node e = table[hash(key)];         ← volatile read of bucket head
 *    while (e != null) {
 *        if (e.key.equals(key)) return e.val;  ← volatile read of value
 *        e = e.next;                    ← volatile read of next
 *    }
 *    return null;
 *
 *  NO LOCK — safe because:
 *    - Bucket head pointer is volatile
 *    - Node.val is volatile
 *    - Node.next is volatile
 *    - ConcurrentHashMap only adds nodes at HEAD (prepend) → readers
 *      see either old chain or new chain, never a broken one
 * ──────────────────────────────────────────────────────────── */
static const char* chm_get(ConcurrentHashMap *m, const char *key) {
    unsigned int hash  = chm_hash(key);
    int          idx   = chm_bucket_index(hash);
    Bucket      *bucket = &m->buckets[idx];

    atomic_fetch_add(&m->get_ops, 1);

    /* Walk the chain — NO LOCK (volatile pointer chain traversal) */
    Node *node = bucket->head;  /* volatile read */
    while (node) {
        if (node->hash == hash && strcmp(node->key, key) == 0) {
            return node->val;   /* volatile read of value */
        }
        node = node->next;      /* volatile read of next */
    }
    return NULL;  /* key not found */
}

/* ─────────────────────────────────────────────────────────────
 *  PUT — fine-grained locked write
 *
 *  Java: ConcurrentHashMap.put(key, value)
 *  Implementation:
 *    int idx = spread(hash(key)) & (n-1);
 *    synchronized(table[idx]) {           ← lock ONLY this bucket
 *        Node head = table[idx];
 *        // walk chain: update if key exists, prepend if new
 *    }
 *
 *  Only the affected bucket is locked → N other buckets remain accessible.
 * ──────────────────────────────────────────────────────────── */
static void chm_put(ConcurrentHashMap *m, const char *key, const char *val) {
    unsigned int hash   = chm_hash(key);
    int          idx    = chm_bucket_index(hash);
    Bucket      *bucket = &m->buckets[idx];

    atomic_fetch_add(&m->put_ops, 1);

    pthread_mutex_lock(&bucket->lock);     /* Java: synchronized(bucket_head) */

    /* Search chain for existing key */
    Node *node = bucket->head;
    while (node) {
        if (node->hash == hash && strcmp(node->key, key) == 0) {
            /* Key exists — update value */
            strncpy(node->val, val, VAL_LEN - 1);
            pthread_mutex_unlock(&bucket->lock);
            return;
        }
        node = node->next;
    }

    /* Key not found — prepend new node at head */
    Node *new_node = calloc(1, sizeof(Node));
    strncpy(new_node->key, key, KEY_LEN - 1);
    strncpy(new_node->val, val, VAL_LEN - 1);
    new_node->hash = hash;
    new_node->next = bucket->head;  /* volatile write: insert at head */
    bucket->head   = new_node;      /* volatile write: update head */

    atomic_fetch_add(&bucket->chain_length, 1);
    atomic_fetch_add(&m->size, 1);

    pthread_mutex_unlock(&bucket->lock);   /* Java: end synchronized block */
}

/* ─────────────────────────────────────────────────────────────
 *  PUT IF ABSENT — atomic check-then-act
 *  Java: map.putIfAbsent(key, value)
 *        Returns: existing value if key present, null if inserted
 * ──────────────────────────────────────────────────────────── */
static const char* chm_put_if_absent(ConcurrentHashMap *m,
                                      const char *key, const char *val) {
    unsigned int hash   = chm_hash(key);
    int          idx    = chm_bucket_index(hash);
    Bucket      *bucket = &m->buckets[idx];

    pthread_mutex_lock(&bucket->lock);

    /* Check if key exists */
    Node *node = bucket->head;
    while (node) {
        if (node->hash == hash && strcmp(node->key, key) == 0) {
            const char *existing = node->val;
            pthread_mutex_unlock(&bucket->lock);
            return existing;   /* already present — return existing value */
        }
        node = node->next;
    }

    /* Insert new */
    Node *new_node = calloc(1, sizeof(Node));
    strncpy(new_node->key, key, KEY_LEN - 1);
    strncpy(new_node->val, val, VAL_LEN - 1);
    new_node->hash = hash;
    new_node->next = bucket->head;
    bucket->head   = new_node;
    atomic_fetch_add(&m->size, 1);

    pthread_mutex_unlock(&bucket->lock);
    return NULL;  /* inserted successfully */
}

/* Print map stats */
static void chm_stats(const ConcurrentHashMap *m) {
    printf("  ConcurrentHashMap stats:\n");
    printf("    Size:        %d entries\n", atomic_load(&m->size));
    printf("    Get ops:     %ld (lock-free reads)\n", atomic_load(&m->get_ops));
    printf("    Put ops:     %ld (bucket-locked writes)\n", atomic_load(&m->put_ops));
    printf("    Bucket fill: ");
    for (int i = 0; i < BUCKET_COUNT; i++) {
        int len = atomic_load(&m->buckets[i].chain_length);
        if (len > 0) printf("[%d]=%d ", i, len);
    }
    printf("\n\n");
}

static void chm_destroy(ConcurrentHashMap *m) {
    for (int i = 0; i < BUCKET_COUNT; i++) {
        Node *n = m->buckets[i].head;
        while (n) { Node *tmp = n->next; free(n); n = tmp; }
        pthread_mutex_destroy(&m->buckets[i].lock);
    }
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    ConcurrentHashMap *map;
    int                thread_id;
    int                ops_count;
} MapWorker;

static void* concurrent_writer(void *arg) {
    MapWorker *w = (MapWorker*)arg;

    for (int i = 0; i < w->ops_count; i++) {
        char key[KEY_LEN], val[VAL_LEN];
        snprintf(key, sizeof(key), "thread%d-key%d", w->thread_id, i);
        snprintf(val, sizeof(val), "value-from-thread%d-iteration%d", w->thread_id, i);
        chm_put(w->map, key, val);
    }
    return NULL;
}

static void* concurrent_reader(void *arg) {
    MapWorker *w = (MapWorker*)arg;

    for (int i = 0; i < w->ops_count; i++) {
        char key[KEY_LEN];
        snprintf(key, sizeof(key), "thread%d-key%d", w->thread_id % 4, i);
        chm_get(w->map, key);  /* may be null — that's ok */
    }
    return NULL;
}

void demo_concurrent_access(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Concurrent Reads and Writes\n");
    printf("4 writer threads + 4 reader threads simultaneously\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ConcurrentHashMap map;
    chm_init(&map);

    pthread_t writers[4], readers[4];
    MapWorker w_args[4], r_args[4];

    printf("  Starting 4 writer threads and 4 reader threads concurrently...\n\n");

    for (int i = 0; i < 4; i++) {
        w_args[i] = (MapWorker){ &map, i, 500 };
        r_args[i] = (MapWorker){ &map, i, 500 };
        pthread_create(&writers[i], NULL, concurrent_writer, &w_args[i]);
        pthread_create(&readers[i], NULL, concurrent_reader, &r_args[i]);
    }

    for (int i = 0; i < 4; i++) { pthread_join(writers[i], NULL); }
    for (int i = 0; i < 4; i++) { pthread_join(readers[i], NULL); }

    chm_stats(&map);

    /* Verify: read back some known entries */
    printf("  Spot-checking entries:\n");
    for (int t = 0; t < 2; t++) {
        for (int i = 0; i < 3; i++) {
            char key[KEY_LEN];
            snprintf(key, sizeof(key), "thread%d-key%d", t, i);
            const char *val = chm_get(&map, key);
            printf("    get('%s') = '%s'\n", key, val ? val : "NULL");
        }
    }
    printf("\n");

    chm_destroy(&map);
}

void demo_put_if_absent(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: putIfAbsent — Thread-Safe Lazy Initialization\n");
    printf("Java: map.computeIfAbsent(key, k -> expensiveCompute(k))\n");
    printf("Spring: singleton bean registry uses this pattern\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    ConcurrentHashMap map;
    chm_init(&map);

    /* Simulate Spring's singleton cache:
     * Multiple threads may request the same bean simultaneously.
     * Only ONE should create it; others should get the cached version. */

    const char *result1 = chm_put_if_absent(&map, "userService", "UserServiceImpl@0x1234");
    const char *result2 = chm_put_if_absent(&map, "userService", "UserServiceImpl@0x5678");
    const char *result3 = chm_put_if_absent(&map, "orderService", "OrderServiceImpl@0xabcd");

    printf("  First put userService:  %s\n", result1 ? "EXISTING" : "INSERTED (new)");
    printf("  Second put userService: %s\n", result2 ? result2 : "(inserted again — impossible)");
    printf("  Put orderService:       %s\n", result3 ? "EXISTING" : "INSERTED (new)");
    printf("  Current size: %d beans\n\n", atomic_load(&map.size));

    printf("  This is how Spring's singletonObjects cache works:\n");
    printf("  Multiple threads requesting the same bean → only one creation\n\n");

    chm_destroy(&map);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  concurrent_hashmap.c — ConcurrentHashMap in C      ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_concurrent_access();
    demo_put_if_absent();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Lock-free reads: volatile Node pointers → no lock needed for get()\n");
    printf("  Fine-grained writes: lock ONLY the affected bucket\n");
    printf("  BUCKET_COUNT concurrent writers can operate simultaneously\n");
    printf("  putIfAbsent: atomic check-and-insert (no TOCTOU race)\n");
    printf("  Spring uses ConcurrentHashMap for bean cache + mapping tables\n");
    return 0;
}
