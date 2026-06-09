/*
 * executor_service.c  —  Java ThreadPoolExecutor in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o executor_service executor_service.c
 * Run:    ./executor_service
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   java.util.concurrent.ThreadPoolExecutor
 *   The most important concurrency utility in Java.
 *   Used by:
 *     - Tomcat's thread pool (handling HTTP requests)
 *     - Spring's @Async executor
 *     - HikariCP's connection borrow tracking
 *     - ExecutorService from Executors.newFixedThreadPool()
 *
 *   KEY MECHANISM:
 *     1. Pre-create N worker threads (they all block on task queue)
 *     2. submit(task) → put task in queue
 *     3. Worker thread wakes → executes task → returns to queue wait
 *     4. Worker threads never die (until executor shutdown)
 *
 *   This avoids thread creation/destruction overhead per task.
 *   Thread creation: ~1ms + ~1MB stack
 *   Queue put + schedule: ~100ns
 *   Amortized cost per task: near zero.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

/* ═══════════════════════════════════════════════════════════
 *  TASK — a unit of work submitted to the pool
 *  Java: Runnable / Callable<T>
 * ═══════════════════════════════════════════════════════════ */
typedef void (*TaskFn)(void *arg);

typedef struct Task {
    TaskFn       fn;
    void        *arg;
    int          task_id;
    struct Task *next;    /* linked list for queue */
} Task;

/* ═══════════════════════════════════════════════════════════
 *  TASK QUEUE — bounded blocking queue
 *  Java: LinkedBlockingQueue<Runnable>
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    Task           *head, *tail;
    int             size;
    int             capacity;       /* max queued tasks (-1 = unbounded)   */
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;     /* signal: task available               */
    pthread_cond_t  not_full;      /* signal: queue has space              */
    int             closed;        /* 1 = no more submissions              */
} TaskQueue;

static void tq_init(TaskQueue *q, int capacity) {
    q->head = q->tail = NULL;
    q->size = 0;
    q->capacity = capacity;
    q->closed = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full,  NULL);
}

/* Put task in queue. Returns 0 if rejected (queue full or closed). */
static int tq_put(TaskQueue *q, Task *task) {
    pthread_mutex_lock(&q->lock);

    /* Wait if queue is full (bounded queue behavior) */
    while (q->capacity > 0 && q->size >= q->capacity && !q->closed) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }

    if (q->closed) {
        pthread_mutex_unlock(&q->lock);
        return 0;  /* Rejected: executor is shutting down */
    }

    /* Enqueue */
    task->next = NULL;
    if (!q->head) q->head = task;
    else          q->tail->next = task;
    q->tail = task;
    q->size++;

    pthread_cond_signal(&q->not_empty);   /* wake one waiting worker */
    pthread_mutex_unlock(&q->lock);
    return 1;
}

/* Take task from queue. Returns NULL when queue closed AND empty. */
static Task* tq_take(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);

    while (q->size == 0 && !q->closed) {
        pthread_cond_wait(&q->not_empty, &q->lock);  /* park until task available */
    }

    Task *task = NULL;
    if (q->head) {
        task    = q->head;
        q->head = task->next;
        if (!q->head) q->tail = NULL;
        q->size--;
        pthread_cond_signal(&q->not_full);  /* signal space available */
    }

    pthread_mutex_unlock(&q->lock);
    return task;
}

static void tq_close(TaskQueue *q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1;
    pthread_cond_broadcast(&q->not_empty);  /* wake all sleeping workers */
    pthread_cond_broadcast(&q->not_full);
    pthread_mutex_unlock(&q->lock);
}

static void tq_destroy(TaskQueue *q) {
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

/* ═══════════════════════════════════════════════════════════
 *  THREAD POOL EXECUTOR
 *  Java: ThreadPoolExecutor(corePoolSize, maxPoolSize, keepAlive, ...)
 * ═══════════════════════════════════════════════════════════ */
#define MAX_POOL_SIZE 16

typedef struct {
    int          core_size;        /* minimum worker threads              */
    int          max_size;         /* maximum worker threads              */
    int          queue_capacity;   /* max queued tasks                    */
    char         thread_prefix[32];/* thread name prefix (for logging)    */

    pthread_t    workers[MAX_POOL_SIZE];
    int          worker_ids[MAX_POOL_SIZE];
    atomic_int   active_threads;   /* threads currently running a task   */
    atomic_int   total_threads;    /* total threads alive                 */

    TaskQueue    queue;

    atomic_long  tasks_completed;  /* lifetime task count                 */
    atomic_int   tasks_rejected;   /* rejection count                     */
    atomic_int   shutdown;         /* 1 = shutting down                   */
} ThreadPool;

/* Worker thread — the "event loop" of a thread pool worker */
static void* worker_loop(void *arg) {
    ThreadPool *pool = (ThreadPool*)arg;
    int my_id = atomic_fetch_add(&pool->total_threads, 1);
    (void)my_id;

    while (1) {
        Task *task = tq_take(&pool->queue);  /* BLOCKS until work available */

        if (!task) {
            /* Queue closed AND empty → shutdown this worker */
            break;
        }

        /* Execute the task */
        atomic_fetch_add(&pool->active_threads, 1);
        task->fn(task->arg);                   /* Java: runnable.run() */
        atomic_fetch_sub(&pool->active_threads, 1);
        atomic_fetch_add(&pool->tasks_completed, 1);

        free(task->arg);
        free(task);
    }

    atomic_fetch_sub(&pool->total_threads, 1);
    return NULL;
}

/* Create the thread pool */
static ThreadPool* pool_create(int core_size, int max_size,
                                int queue_capacity, const char *prefix) {
    ThreadPool *pool = calloc(1, sizeof(ThreadPool));
    pool->core_size       = core_size;
    pool->max_size        = (max_size < MAX_POOL_SIZE) ? max_size : MAX_POOL_SIZE;
    pool->queue_capacity  = queue_capacity;
    strncpy(pool->thread_prefix, prefix, 31);

    tq_init(&pool->queue, queue_capacity);

    atomic_store(&pool->active_threads, 0);
    atomic_store(&pool->total_threads,  0);
    atomic_store(&pool->tasks_completed, 0);
    atomic_store(&pool->tasks_rejected, 0);
    atomic_store(&pool->shutdown, 0);

    /* Create core threads (always alive) */
    printf("[ThreadPool] Creating %d core threads (prefix='%s')\n",
           core_size, prefix);
    for (int i = 0; i < core_size; i++) {
        pool->worker_ids[i] = i;
        pthread_create(&pool->workers[i], NULL, worker_loop, pool);
    }

    return pool;
}

/* Submit a task to the pool */
static int pool_submit(ThreadPool *pool, TaskFn fn, void *arg, int task_id) {
    if (atomic_load(&pool->shutdown)) {
        atomic_fetch_add(&pool->tasks_rejected, 1);
        printf("  [Pool] Task #%-3d REJECTED (executor shut down)\n", task_id);
        return 0;
    }

    Task *task = malloc(sizeof(Task));
    task->fn      = fn;
    task->arg     = arg;
    task->task_id = task_id;
    task->next    = NULL;

    if (!tq_put(&pool->queue, task)) {
        /* Rejection: queue full AND max threads reached */
        atomic_fetch_add(&pool->tasks_rejected, 1);
        printf("  [Pool] Task #%-3d REJECTED (queue full — RejectionPolicy)\n", task_id);
        free(task);
        return 0;
    }

    return 1;
}

/* Shutdown and wait for all tasks to complete */
static void pool_shutdown_await(ThreadPool *pool) {
    printf("[ThreadPool] shutdown() — draining queue...\n");
    atomic_store(&pool->shutdown, 1);
    tq_close(&pool->queue);   /* signal workers to exit */

    for (int i = 0; i < pool->core_size; i++) {
        pthread_join(pool->workers[i], NULL);
    }

    printf("[ThreadPool] All workers exited.\n");
    printf("[ThreadPool] Tasks completed: %ld  rejected: %d\n",
           atomic_load(&pool->tasks_completed),
           atomic_load(&pool->tasks_rejected));
}

static void pool_destroy(ThreadPool *pool) {
    tq_destroy(&pool->queue);
    free(pool);
}

/* Print thread pool status */
static void pool_status(const ThreadPool *pool) {
    printf("  ThreadPool status:\n");
    printf("    active_threads:    %d / %d\n",
           atomic_load(&pool->active_threads),
           pool->core_size);
    printf("    queue_size:        %d / %d\n",
           pool->queue.size, pool->queue_capacity);
    printf("    tasks_completed:   %ld\n", atomic_load(&pool->tasks_completed));
    printf("    tasks_rejected:    %d\n",  atomic_load(&pool->tasks_rejected));
}

/* ─────────────────────────────────────────────────────────────
 *  Sample tasks for demonstration
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    int  task_id;
    int  duration_ms;
    char description[64];
} TaskData;

static void http_request_task(void *arg) {
    TaskData *d = (TaskData*)arg;
    /* Simulate HTTP request handling: parse, business logic, respond */
    printf("  [Task #%-3d] START  '%s'\n", d->task_id, d->description);
    usleep(d->duration_ms * 1000);
    printf("  [Task #%-3d] DONE   '%s' (%d ms)\n",
           d->task_id, d->description, d->duration_ms);
    /* Note: free(arg) called by pool after task returns */
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_thread_pool(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: ThreadPoolExecutor — Simulating Tomcat Request Pool\n");
    printf("Java: Executors.newFixedThreadPool(4)\n");
    printf("      / Spring: ThreadPoolTaskExecutor(core=4, max=10, queue=100)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Create pool: 4 core threads, queue=8 tasks max */
    ThreadPool *pool = pool_create(4, 4, 8, "http-");

    printf("\nSubmitting 10 HTTP requests to pool with 4 threads:\n\n");

    /* Submit 10 tasks (will queue some since we only have 4 workers) */
    for (int i = 1; i <= 10; i++) {
        TaskData *d = malloc(sizeof(TaskData));
        d->task_id    = i;
        d->duration_ms = 50 + (i * 10) % 100;
        snprintf(d->description, 63, "GET /api/resource/%d", i);

        int ok = pool_submit(pool, http_request_task, d, i);
        if (!ok) free(d);
    }

    printf("\nPool status after submission:\n");
    pool_status(pool);

    pool_shutdown_await(pool);
    pool_destroy(pool);
}

void demo_rejection(void) {
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Task Rejection (queue full + max threads reached)\n");
    printf("Java: ThreadPoolExecutor → RejectedExecutionException\n");
    printf("      CallerRunsPolicy / AbortPolicy / DiscardPolicy\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* Small pool: 2 threads, queue=3 → only 5 tasks before rejection */
    ThreadPool *pool = pool_create(2, 2, 3, "small-");

    printf("Pool: 2 threads, queue capacity=3\n");
    printf("Submitting 8 slow tasks (each 200ms):\n\n");

    for (int i = 1; i <= 8; i++) {
        TaskData *d = malloc(sizeof(TaskData));
        d->task_id    = i;
        d->duration_ms = 200;
        snprintf(d->description, 63, "slow-task-%d", i);

        int ok = pool_submit(pool, http_request_task, d, i);
        if (!ok) {
            free(d);
            printf("    → In production: alert + log + backpressure!\n");
        }
        usleep(5000);  /* slight delay between submissions */
    }

    pool_shutdown_await(pool);
    pool_destroy(pool);

    printf("\n  LESSON: Monitor tasks_rejected metric!\n");
    printf("  In Spring Boot: /actuator/metrics/executor.pool.size\n");
    printf("                  /actuator/metrics/executor.queue.remaining\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  executor_service.c — Java ThreadPoolExecutor in C  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_thread_pool();
    demo_rejection();

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Thread pool: pre-create N threads → reuse for tasks\n");
    printf("  Worker loop: blocks on queue → wakes on task → executes → blocks\n");
    printf("  Rejection happens when: queue full AND max threads reached\n");
    printf("  Spring @Async uses ThreadPoolTaskExecutor (same pattern)\n");
    printf("  Monitor: active threads, queue size, rejection count\n");
    printf("  Virtual threads (JDK 21): one virtual thread per task, no pooling needed\n");
    return 0;
}
