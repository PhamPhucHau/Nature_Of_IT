/*
 * async_execution.c  —  Spring @Async + Request Thread Model in C
 *
 * Build:  gcc -Wall -Wextra -pthread -o async_execution async_execution.c
 * Run:    ./async_execution
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   1. Spring's @Async: submit a task to a dedicated thread pool
 *      and return immediately to the caller (fire-and-forget or future)
 *
 *   2. Spring's Tomcat request thread lifecycle:
 *      - Thread picked from pool
 *      - SecurityContext set in ThreadLocal
 *      - Transaction context set in ThreadLocal
 *      - Business logic executes
 *      - ThreadLocals cleared
 *      - Thread returned to pool
 *
 *   3. TransactionSynchronizationManager simulation:
 *      ThreadLocal<Map<DataSource, Connection>> binds one connection per thread
 *
 * KEY SPRING PATTERN:
 *   Request thread → @Service → @Repository → uses ThreadLocal connection
 *   @Async thread  → separate thread pool → DOES NOT share ThreadLocal!
 *   This is the most common @Async bug in Spring.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

/* ═══════════════════════════════════════════════════════════
 *  SPRING CONTEXT — simulates what Spring puts in ThreadLocal
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    int  user_id;
    char username[32];
    char role[16];
} SecurityContext;   /* Java: SecurityContextHolder.getContext() */

typedef struct {
    int  connection_id;
    char datasource_url[128];
    int  active_transaction;
    int  auto_commit;
} ConnectionContext;  /* Java: TransactionSynchronizationManager */

typedef struct {
    char             request_id[64];
    char             request_path[128];
    SecurityContext  security;
    ConnectionContext connection;
    int              has_transaction;
} RequestContext;

/* ThreadLocal keys */
static pthread_key_t g_request_context_key;

static void request_context_destructor(void *ctx) {
    if (ctx) {
        RequestContext *rc = (RequestContext*)ctx;
        printf("  [ThreadLocal] Cleaning up context for request '%s'\n",
               rc->request_id);
        free(ctx);
    }
}

/* Get current thread's RequestContext (never null after init) */
static RequestContext* get_context(void) {
    return (RequestContext*)pthread_getspecific(g_request_context_key);
}

/* Set context for this thread */
static void set_context(RequestContext *ctx) {
    pthread_setspecific(g_request_context_key, ctx);
}

/* Clear context (end of request) */
static void clear_context(void) {
    RequestContext *ctx = get_context();
    if (ctx) {
        free(ctx);
        pthread_setspecific(g_request_context_key, NULL);
    }
}

/* ═══════════════════════════════════════════════════════════
 *  SIMPLE THREAD POOL (from phase 7 — reused here)
 * ═══════════════════════════════════════════════════════════ */
#define ASYNC_POOL_SIZE 4
#define ASYNC_QUEUE_MAX 16

typedef void (*AsyncTaskFn)(void *arg);

typedef struct AsyncTask {
    AsyncTaskFn      fn;
    void            *arg;
    struct AsyncTask *next;
} AsyncTask;

typedef struct {
    AsyncTask      *head, *tail;
    int             size;
    pthread_mutex_t lock;
    pthread_cond_t  not_empty;
    int             closed;
    pthread_t       workers[ASYNC_POOL_SIZE];
    atomic_int      completed;
} AsyncPool;

static void async_pool_destroy(AsyncPool *p);

static void* async_worker(void *arg) {
    AsyncPool *pool = (AsyncPool*)arg;
    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (pool->size == 0 && !pool->closed) {
            pthread_cond_wait(&pool->not_empty, &pool->lock);
        }
        AsyncTask *task = NULL;
        if (pool->head) {
            task = pool->head;
            pool->head = task->next;
            if (!pool->head) pool->tail = NULL;
            pool->size--;
        }
        pthread_mutex_unlock(&pool->lock);
        if (!task) break;
        task->fn(task->arg);
        free(task);
        atomic_fetch_add(&pool->completed, 1);
    }
    return NULL;
}

static AsyncPool* async_pool_create(void) {
    AsyncPool *p = calloc(1, sizeof(AsyncPool));
    pthread_mutex_init(&p->lock, NULL);
    pthread_cond_init(&p->not_empty, NULL);
    for (int i = 0; i < ASYNC_POOL_SIZE; i++) {
        pthread_create(&p->workers[i], NULL, async_worker, p);
    }
    return p;
}

static void async_submit(AsyncPool *pool, AsyncTaskFn fn, void *arg) {
    AsyncTask *task = malloc(sizeof(AsyncTask));
    task->fn   = fn;
    task->arg  = arg;
    task->next = NULL;
    pthread_mutex_lock(&pool->lock);
    if (!pool->head) pool->head = task;
    else             pool->tail->next = task;
    pool->tail = task;
    pool->size++;
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
}

static void async_pool_shutdown(AsyncPool *pool) {
    pthread_mutex_lock(&pool->lock);
    pool->closed = 1;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->lock);
    for (int i = 0; i < ASYNC_POOL_SIZE; i++) pthread_join(pool->workers[i], NULL);
}

static void async_pool_destroy(AsyncPool *p) {
    pthread_mutex_destroy(&p->lock);
    pthread_cond_destroy(&p->not_empty);
    free(p);
}

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED LAYERS
 * ═══════════════════════════════════════════════════════════ */

/* @Repository — uses ThreadLocal connection */
static void user_repository_find(int user_id) {
    RequestContext *ctx = get_context();

    if (ctx && ctx->has_transaction) {
        printf("    [UserRepository] findById(%d) using connection #%d (TX active)\n",
               user_id, ctx->connection.connection_id);
    } else {
        printf("    [UserRepository] findById(%d) — NO TRANSACTION CONTEXT\n", user_id);
    }
    usleep(5000);  /* simulate DB query */
}

/* @Service */
static void user_service_process(int user_id) {
    RequestContext *ctx = get_context();

    printf("  [@Service] Processing user %d (security: %s)\n",
           user_id, ctx ? ctx->security.username : "(none)");
    user_repository_find(user_id);
}

/* @Async task — runs on different thread, NO request ThreadLocal */
typedef struct { int user_id; char email[64]; } EmailArgs;

static void async_send_email(void *arg) {
    EmailArgs *ea = (EmailArgs*)arg;
    RequestContext *ctx = get_context();  /* ThreadLocal on ASYNC thread = NULL */

    printf("  [@Async emailExecutor] send_email(user=%d, email=%s)\n",
           ea->user_id, ea->email);

    if (!ctx) {
        printf("    [WARNING] No SecurityContext on async thread!\n");
        printf("    [WARNING] If this needs auth → must propagate context manually\n");
    }

    usleep(30000);  /* simulate email sending */
    printf("  [@Async] Email sent to %s\n", ea->email);
    free(ea);
}

/* ─────────────────────────────────────────────────────────────
 *  Request processing simulation (one Tomcat thread)
 * ──────────────────────────────────────────────────────────── */
typedef struct {
    AsyncPool *email_pool;
    int        user_id;
    char       username[32];
    char       request_id[32];
} RequestArgs;

static void* handle_http_request(void *arg) {
    RequestArgs *ra = (RequestArgs*)arg;

    /* ── STEP 1: Spring sets up ThreadLocal context ────────────── */
    RequestContext *ctx = calloc(1, sizeof(RequestContext));
    strncpy(ctx->request_id,    ra->request_id, 63);
    strncpy(ctx->request_path,  "/api/users/process", 127);

    /* SecurityContextHolder.setAuthentication(auth) */
    ctx->security.user_id = ra->user_id;
    strncpy(ctx->security.username, ra->username, 31);
    strncpy(ctx->security.role, "USER", 15);

    /* TransactionSynchronizationManager.bindResource(dataSource, conn) */
    ctx->connection.connection_id  = ra->user_id * 100;
    ctx->connection.active_transaction = 1;
    ctx->connection.auto_commit    = 0;
    strncpy(ctx->connection.datasource_url, "jdbc:mysql://localhost/app", 127);
    ctx->has_transaction = 1;

    set_context(ctx);  /* put in ThreadLocal */

    printf("[Request %s] Handling on thread %lu (user=%s)\n",
           ra->request_id,
           (unsigned long)pthread_self(),
           ctx->security.username);

    /* ── STEP 2: Business logic executes (same thread throughout) ── */
    user_service_process(ra->user_id);

    /* ── STEP 3: @Async — fire-and-forget on separate pool ──────── */
    printf("  [Controller] @Async: submitting email to emailExecutor pool\n");
    EmailArgs *ea = malloc(sizeof(EmailArgs));
    ea->user_id = ra->user_id;
    snprintf(ea->email, sizeof(ea->email), "user%d@example.com", ra->user_id);
    async_submit(ra->email_pool, async_send_email, ea);

    /* ── STEP 4: Response sent ──────────────────────────────────── */
    printf("  [Controller] Response: 200 OK (returned immediately, email async)\n");

    /* ── STEP 5: Spring cleanup — clear all ThreadLocals ─────────── */
    printf("  [Spring] Clearing ThreadLocals (SecurityContext, Transaction, Request)\n");
    clear_context();  /* SecurityContextHolder.clearContext() */

    printf("[Request %s] COMPLETE — thread returning to Tomcat pool\n\n",
           ra->request_id);

    free(ra);
    return NULL;
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_spring_request_thread_model(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO: Spring Request Thread Model + @Async\n");
    printf("Shows: ThreadLocal context lifecycle per request\n");
    printf("       @Async runs on separate pool, NO shared ThreadLocal\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    /* The @Async email thread pool */
    AsyncPool *email_pool = async_pool_create();

    /* 3 concurrent HTTP requests on different Tomcat threads */
    pthread_t request_threads[3];
    RequestArgs *args[3];

    char *users[]   = { "alice", "bob", "charlie" };
    char *req_ids[] = { "REQ-001", "REQ-002", "REQ-003" };

    for (int i = 0; i < 3; i++) {
        args[i] = malloc(sizeof(RequestArgs));
        args[i]->email_pool = email_pool;
        args[i]->user_id    = i + 1;
        strncpy(args[i]->username,   users[i],   31);
        strncpy(args[i]->request_id, req_ids[i], 31);
        pthread_create(&request_threads[i], NULL, handle_http_request, args[i]);
    }

    for (int i = 0; i < 3; i++) pthread_join(request_threads[i], NULL);

    printf("Waiting for async emails to complete...\n\n");
    usleep(100000);  /* allow async tasks to finish */

    async_pool_shutdown(email_pool);
    printf("Async tasks completed: %d\n\n", atomic_load(&email_pool->completed));
    async_pool_destroy(email_pool);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  async_execution.c — Spring @Async + Request Model  ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    pthread_key_create(&g_request_context_key, request_context_destructor);

    demo_spring_request_thread_model();

    pthread_key_delete(g_request_context_key);

    printf("━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Request thread carries ALL context in ThreadLocal\n");
    printf("  @Async runs on different thread → ThreadLocal is EMPTY\n");
    printf("  Spring clears ThreadLocals at end of each request\n");
    printf("  @Transactional binds connection to current thread (ThreadLocal)\n");
    printf("  @Async bug: calling @Transactional code from @Async thread\n");
    printf("    without propagating context → no transaction → data corruption\n");
    return 0;
}
