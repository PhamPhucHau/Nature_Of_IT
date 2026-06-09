/*
 * mutex_monitor.c  —  Java Monitor (synchronized + wait/notify) in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o mutex_monitor mutex_monitor.c
 * Run:    ./mutex_monitor
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   Java's built-in monitor — the foundation of synchronized blocks.
 *   Every Java object has a monitor: mutex + condition variable.
 *
 *   Java → C mapping:
 *     synchronized(obj)    → pthread_mutex_lock(&obj.mutex)
 *     obj.wait()           → pthread_cond_wait(&obj.cond, &obj.mutex)
 *     obj.notify()         → pthread_cond_signal(&obj.cond)
 *     obj.notifyAll()      → pthread_cond_broadcast(&obj.cond)
 *
 *   PRODUCER-CONSUMER pattern — the canonical use of wait/notify:
 *
 *   Java:
 *     synchronized(queue) {
 *         while (queue.isEmpty()) queue.wait();  // wait until item available
 *         return queue.poll();
 *     }
 *
 *     synchronized(queue) {
 *         queue.add(item);
 *         queue.notifyAll();  // wake waiting consumers
 *     }
 *
 * KEY LESSON:
 *   wait() ATOMICALLY releases the lock AND suspends the thread.
 *   Without this atomic release+wait, the producer could notify()
 *   BEFORE the consumer calls wait() → consumer waits forever (missed signal).
 *
 *   ALWAYS use wait() in a LOOP:
 *     while (condition is false) { wait(); }
 *     NOT: if (condition is false) { wait(); }
 *   Reason: spurious wakeups (JVM/OS can wake threads without notify())
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/* ═══════════════════════════════════════════════════════════
 *  JAVA MONITOR STRUCT
 *  Combines mutex + condition variable — like Java's built-in monitor
 *  Java: every Object has one of these implicitly in its header
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    pthread_mutex_t mutex;    /* Java: implicit lock on object   */
    pthread_cond_t  cond;     /* Java: implicit condition (wait/notify) */
    int             owner_tid;/* thread currently holding the lock      */
    int             wait_count;/* number of threads in wait()           */
} JavaMonitor;

static void monitor_init(JavaMonitor *m) {
    pthread_mutex_init(&m->mutex, NULL);
    pthread_cond_init(&m->cond, NULL);
    m->owner_tid  = -1;
    m->wait_count = 0;
}

/* synchronized(m): enter monitor */
static void monitor_enter(JavaMonitor *m) {
    pthread_mutex_lock(&m->mutex);
    m->owner_tid = (int)(unsigned long)pthread_self();
}

/* end of synchronized block: exit monitor */
static void monitor_exit(JavaMonitor *m) {
    m->owner_tid = -1;
    pthread_mutex_unlock(&m->mutex);
}

/* obj.wait(): release lock + wait for notify/notifyAll */
/* Java: must be called while holding the monitor */
static void monitor_wait(JavaMonitor *m) {
    m->wait_count++;
    m->owner_tid = -1;
    /* ATOMIC: releases mutex AND blocks — this is the critical guarantee */
    pthread_cond_wait(&m->cond, &m->mutex);
    m->owner_tid = (int)(unsigned long)pthread_self();
    m->wait_count--;
}

/* obj.notify(): wake ONE waiting thread */
static void monitor_notify(JavaMonitor *m) {
    pthread_cond_signal(&m->cond);
}

/* obj.notifyAll(): wake ALL waiting threads */
static void monitor_notify_all(JavaMonitor *m) {
    pthread_cond_broadcast(&m->cond);
}

static void monitor_destroy(JavaMonitor *m) {
    pthread_mutex_destroy(&m->mutex);
    pthread_cond_destroy(&m->cond);
}

/* ═══════════════════════════════════════════════════════════
 *  BLOCKING QUEUE SIMULATION
 *  Java: LinkedBlockingQueue / ArrayBlockingQueue
 *  This is the most common use of wait/notify in Java.
 * ═══════════════════════════════════════════════════════════ */
#define QUEUE_CAPACITY 5

typedef struct {
    int          items[QUEUE_CAPACITY];
    int          head, tail, size;
    JavaMonitor  monitor;        /* Java: synchronized(this) */
    int          closed;         /* no more items will be added */
} BlockingQueue;

static void bq_init(BlockingQueue *q) {
    memset(q, 0, sizeof(BlockingQueue));
    monitor_init(&q->monitor);
}

/* BlockingQueue.put(item) — blocks if full */
static void bq_put(BlockingQueue *q, int item, int producer_id) {
    monitor_enter(&q->monitor);                 /* synchronized(q) { */

    /* Java: while (size >= capacity) wait(); */
    while (q->size >= QUEUE_CAPACITY && !q->closed) {
        printf("  [Producer %d] Queue FULL (%d/%d) — waiting...\n",
               producer_id, q->size, QUEUE_CAPACITY);
        monitor_wait(&q->monitor);              /* wait() — queue not full */
    }

    if (!q->closed) {
        q->items[q->tail] = item;
        q->tail = (q->tail + 1) % QUEUE_CAPACITY;
        q->size++;
        printf("  [Producer %d] PUT %3d → queue size=%d/%d\n",
               producer_id, item, q->size, QUEUE_CAPACITY);
        monitor_notify_all(&q->monitor);        /* notifyAll() — item available */
    }

    monitor_exit(&q->monitor);                 /* } end synchronized */
}

/* BlockingQueue.take() — blocks if empty, returns -1 if closed+empty */
static int bq_take(BlockingQueue *q, int consumer_id) {
    monitor_enter(&q->monitor);

    /* Java: while (size == 0 && !closed) wait(); */
    while (q->size == 0 && !q->closed) {
        printf("  [Consumer %d] Queue EMPTY — waiting...\n", consumer_id);
        monitor_wait(&q->monitor);             /* wait() — queue not empty */
    }

    int item = -1;
    if (q->size > 0) {
        item = q->items[q->head];
        q->head = (q->head + 1) % QUEUE_CAPACITY;
        q->size--;
        printf("  [Consumer %d] TAKE %3d ← queue size=%d/%d\n",
               consumer_id, item, q->size, QUEUE_CAPACITY);
        monitor_notify_all(&q->monitor);       /* notifyAll() — space available */
    }

    monitor_exit(&q->monitor);
    return item;
}

/* Close the queue — wake all waiting threads */
static void bq_close(BlockingQueue *q) {
    monitor_enter(&q->monitor);
    q->closed = 1;
    monitor_notify_all(&q->monitor);   /* wake all waiting consumers */
    monitor_exit(&q->monitor);
    printf("  [Queue] Closed — no more items\n");
}

static void bq_destroy(BlockingQueue *q) {
    monitor_destroy(&q->monitor);
}

/* ─────────────────────────────────────────────────────────────
 *  Thread functions for producer-consumer demo
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    BlockingQueue *q;
    int            id;
    int            count;
} PCArgs;

static void* producer_fn(void *arg) {
    PCArgs *pa = (PCArgs*)arg;
    for (int i = 1; i <= pa->count; i++) {
        int item = pa->id * 100 + i;   /* unique item per producer */
        bq_put(pa->q, item, pa->id);
        usleep(20000);  /* 20ms between items */
    }
    printf("  [Producer %d] Done producing %d items\n", pa->id, pa->count);
    return NULL;
}

static void* consumer_fn(void *arg) {
    PCArgs *ca = (PCArgs*)arg;
    int items_consumed = 0;

    while (1) {
        int item = bq_take(ca->q, ca->id);
        if (item == -1) {
            printf("  [Consumer %d] Queue closed, consumed %d items total\n",
                   ca->id, items_consumed);
            break;
        }
        items_consumed++;
        usleep(30000);  /* 30ms to "process" each item */
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 1: Producer-Consumer with wait/notify
 * ═══════════════════════════════════════════════════════════ */
void demo_producer_consumer(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Producer-Consumer (BlockingQueue with wait/notify)\n");
    printf("Java: LinkedBlockingQueue — backed by monitor wait/notify\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    BlockingQueue q;
    bq_init(&q);

    /* 2 producers, 3 consumers */
    pthread_t producers[2], consumers[3];
    PCArgs p_args[2] = { {&q, 1, 5}, {&q, 2, 5} };   /* 5 items each */
    PCArgs c_args[3] = { {&q, 1, 0}, {&q, 2, 0}, {&q, 3, 0} };

    printf("  Starting 2 producers (5 items each = 10 total)\n");
    printf("  Starting 3 consumers\n");
    printf("  Queue capacity: %d\n\n", QUEUE_CAPACITY);

    for (int i = 0; i < 3; i++) pthread_create(&consumers[i], NULL, consumer_fn, &c_args[i]);
    for (int i = 0; i < 2; i++) pthread_create(&producers[i], NULL, producer_fn, &p_args[i]);

    /* Wait for producers to finish */
    for (int i = 0; i < 2; i++) pthread_join(producers[i], NULL);

    /* Signal consumers no more items */
    bq_close(&q);

    for (int i = 0; i < 3; i++) pthread_join(consumers[i], NULL);
    bq_destroy(&q);

    printf("\n  LESSON: wait() atomically releases lock + suspends.\n");
    printf("          Always use while() not if() — spurious wakeups exist.\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO 2: DEADLOCK — two threads each hold one lock, wait for the other
 * ═══════════════════════════════════════════════════════════ */
void demo_deadlock_simulation(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Deadlock — Why and How to Avoid It\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    printf("  DEADLOCK SCENARIO (NOT executed — would hang):\n\n");
    printf("  Thread A:                    Thread B:\n");
    printf("    synchronized(lockX) {        synchronized(lockY) {\n");
    printf("      // holds X                   // holds Y\n");
    printf("      synchronized(lockY) {        synchronized(lockX) {\n");
    printf("        // wants Y                   // wants X\n");
    printf("        // BLOCKED ←────────────────── BLOCKED\n");
    printf("      }                            }\n");
    printf("    }                            }\n\n");

    printf("  PREVENTION STRATEGIES:\n\n");
    printf("  1. LOCK ORDERING: Always acquire locks in the same order.\n");
    printf("     Thread A:  lock(Math.min(a,b)); lock(Math.max(a,b));\n");
    printf("     Thread B:  lock(Math.min(a,b)); lock(Math.max(a,b));\n");
    printf("     → Both acquire in same order → no deadlock possible\n\n");

    printf("  2. TRYLOCK WITH TIMEOUT:\n");
    printf("     Java: lock.tryLock(500, TimeUnit.MILLISECONDS)\n");
    printf("     C:    pthread_mutex_timedlock(&mutex, &timeout)\n");
    printf("     If lock not acquired in time → release held locks + retry\n\n");

    printf("  3. LOCK HIERARCHY:\n");
    printf("     Assign a level to each lock (e.g., by address or ID).\n");
    printf("     Threads may only acquire locks in ascending level order.\n\n");

    printf("  4. AVOID NESTED LOCKS:\n");
    printf("     Design code to never hold more than one lock at a time.\n\n");

    printf("  Spring @Transactional DEADLOCK common scenario:\n");
    printf("    Method A (TX1): updates table_users, then calls Method B\n");
    printf("    Method B (TX2): updates table_orders, then updates table_users\n");
    printf("    → DB row locks create the same deadlock at the SQL level\n");
    printf("    Fix: consistent table update order, shorter transactions\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  mutex_monitor.c — Java Monitor in C                ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_producer_consumer();
    demo_deadlock_simulation();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Java monitor = mutex + condition variable combined\n");
    printf("  synchronized(obj) = pthread_mutex_lock(&obj.mutex)\n");
    printf("  wait() atomically releases lock + suspends thread\n");
    printf("  ALWAYS use while() loop around wait() (spurious wakeups)\n");
    printf("  notifyAll() safer than notify() (all waiters re-check condition)\n");
    printf("  Deadlock prevention: consistent lock ordering or tryLock timeout\n");
    return 0;
}
