/*
 * shutdown_hook.c  —  Spring Boot Graceful Shutdown in C
 *
 * Build:  gcc -Wall -Wextra -o shutdown_hook shutdown_hook.c
 * Run:    ./shutdown_hook
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.web.server.GracefulShutdown
 *   org.springframework.context.support.AbstractApplicationContext.close()
 *   org.springframework.context.SmartLifecycle
 *
 *   Spring Boot graceful shutdown flow:
 *     SIGTERM → JVM shutdown hook → SpringApplication.shutdownHook()
 *       → GracefulShutdown.shutDownGracefully()
 *         → connector.pause()          (stop new connections)
 *         → waitForActiveRequests()    (drain in-flight)
 *       → ApplicationContext.close()
 *         → SmartLifecycle.stop() in phase order (highest phase first)
 *         → @PreDestroy in reverse bean creation order
 *         → Database connections closed
 *
 * KEY INSIGHT:
 *   The ordered shutdown is the mirror of ordered startup.
 *   Things created first are destroyed last.
 *   This preserves dependency integrity during teardown:
 *     UserService depends on DataSource →
 *     UserService destroyed FIRST, DataSource destroyed LAST.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  IN-FLIGHT REQUEST TRACKER
 *  Spring Boot tracks active requests in the web server.
 *  Graceful shutdown waits for this counter to reach zero.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    int  active_count;     /* currently processing            */
    int  total_received;   /* lifetime total                  */
    int  accepting;        /* 1 = accepting new, 0 = paused   */
} RequestTracker;

static void request_accept(RequestTracker *rt, const char *path) {
    if (!rt->accepting) {
        printf("  [Server] Rejected: %s (server paused)\n", path);
        return;
    }
    rt->active_count++;
    rt->total_received++;
    printf("  [Request] Started:    %-30s  active=%d\n", path, rt->active_count);
}

static void request_complete(RequestTracker *rt, const char *path) {
    if (rt->active_count > 0) rt->active_count--;
    printf("  [Request] Completed:  %-30s  active=%d\n", path, rt->active_count);
}

/* ═══════════════════════════════════════════════════════════
 *  LIFECYCLE BEAN — represents a SmartLifecycle component
 *  Java: implements SmartLifecycle { start(), stop(callback), getPhase() }
 *
 *  Phase rules:
 *    startup:  lower phase number starts FIRST
 *    shutdown: higher phase number stops FIRST (reverse of startup)
 *
 *  Spring defaults:
 *    Integer.MIN_VALUE (-2147483648) = earliest startup / latest shutdown
 *    Integer.MAX_VALUE ( 2147483647) = latest startup  / earliest shutdown
 * ═══════════════════════════════════════════════════════════ */
typedef void (*StartFn)(void *bean);
typedef void (*StopFn)(void *bean);

typedef struct {
    const char *name;
    int         phase;         /* SmartLifecycle.getPhase() */
    StartFn     start_fn;
    StopFn      stop_fn;
    int         running;
    void       *data;
} LifecycleBean;

#define MAX_LIFECYCLE_BEANS 16

typedef struct {
    LifecycleBean beans[MAX_LIFECYCLE_BEANS];
    int           count;
} LifecycleRegistry;

/* ─────────────────────────────────────────────────────────────
 *  Registration
 * ──────────────────────────────────────────────────────────── */
static void lifecycle_register(LifecycleRegistry *reg,
                                const char *name, int phase,
                                StartFn start, StopFn stop, void *data) {
    if (reg->count >= MAX_LIFECYCLE_BEANS) return;
    LifecycleBean *b = &reg->beans[reg->count++];
    b->name     = name;
    b->phase    = phase;
    b->start_fn = start;
    b->stop_fn  = stop;
    b->running  = 0;
    b->data     = data;
}

/* ─────────────────────────────────────────────────────────────
 *  Start all lifecycle beans in phase order (lowest first)
 * ──────────────────────────────────────────────────────────── */
static void lifecycle_start_all(LifecycleRegistry *reg) {
    printf("[Lifecycle] Starting all SmartLifecycle beans...\n");

    /* Simple insertion sort by phase ascending */
    int order[MAX_LIFECYCLE_BEANS];
    for (int i = 0; i < reg->count; i++) order[i] = i;
    for (int i = 0; i < reg->count - 1; i++) {
        for (int j = i + 1; j < reg->count; j++) {
            if (reg->beans[order[i]].phase > reg->beans[order[j]].phase) {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }
        }
    }

    for (int i = 0; i < reg->count; i++) {
        LifecycleBean *b = &reg->beans[order[i]];
        printf("  [phase=%-12d] %-30s → start()\n", b->phase, b->name);
        if (b->start_fn) b->start_fn(b->data);
        b->running = 1;
    }
    printf("[Lifecycle] All beans started\n\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Stop all lifecycle beans in REVERSE phase order (highest first)
 * ──────────────────────────────────────────────────────────── */
static void lifecycle_stop_all(LifecycleRegistry *reg) {
    printf("[Lifecycle] Stopping all SmartLifecycle beans (reverse phase order)...\n");

    /* Sort by phase descending (highest stops first) */
    int order[MAX_LIFECYCLE_BEANS];
    for (int i = 0; i < reg->count; i++) order[i] = i;
    for (int i = 0; i < reg->count - 1; i++) {
        for (int j = i + 1; j < reg->count; j++) {
            if (reg->beans[order[i]].phase < reg->beans[order[j]].phase) {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }
        }
    }

    for (int i = 0; i < reg->count; i++) {
        LifecycleBean *b = &reg->beans[order[i]];
        if (!b->running) continue;
        printf("  [phase=%-12d] %-30s → stop()\n", b->phase, b->name);
        if (b->stop_fn) b->stop_fn(b->data);
        b->running = 0;
    }
    printf("[Lifecycle] All beans stopped\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  @PREDESTROY BEAN — destroyed during ApplicationContext.close()
 * ═══════════════════════════════════════════════════════════ */
typedef void (*DestroyFn)(void);

typedef struct {
    const char *bean_name;
    int         creation_order;   /* lower = created earlier = destroyed later */
    DestroyFn   destroy_fn;
} DestroyableBean;

#define MAX_DESTROYABLE 16

typedef struct {
    DestroyableBean beans[MAX_DESTROYABLE];
    int             count;
} DestroyRegistry;

static void destroy_register(DestroyRegistry *reg, const char *name,
                               int order, DestroyFn fn) {
    if (reg->count >= MAX_DESTROYABLE) return;
    reg->beans[reg->count].bean_name      = name;
    reg->beans[reg->count].creation_order = order;
    reg->beans[reg->count].destroy_fn     = fn;
    reg->count++;
}

/* Destroy in reverse creation order */
static void destroy_all(const DestroyRegistry *reg) {
    printf("[Context] Running @PreDestroy (reverse creation order)...\n");

    /* Sort by creation_order descending */
    int order[MAX_DESTROYABLE];
    for (int i = 0; i < reg->count; i++) order[i] = i;
    for (int i = 0; i < reg->count - 1; i++) {
        for (int j = i + 1; j < reg->count; j++) {
            if (reg->beans[order[i]].creation_order < reg->beans[order[j]].creation_order) {
                int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
            }
        }
    }

    for (int i = 0; i < reg->count; i++) {
        const DestroyableBean *b = &reg->beans[order[i]];
        printf("  [#%-2d] @PreDestroy %-30s\n",
               b->creation_order, b->bean_name);
        if (b->destroy_fn) b->destroy_fn();
    }
    printf("[Context] All beans destroyed\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  SAMPLE START/STOP IMPLEMENTATIONS
 * ═══════════════════════════════════════════════════════════ */
void kafka_consumer_start(void *d)  { (void)d; printf("    Starting Kafka consumers (3 partitions)\n"); }
void kafka_consumer_stop(void *d)   { (void)d; printf("    Finishing in-progress messages, closing consumers\n"); }
void scheduler_start(void *d)       { (void)d; printf("    Starting @Scheduled task executor\n"); }
void scheduler_stop(void *d)        { (void)d; printf("    Waiting for running @Scheduled tasks to complete\n"); }
void cache_manager_start(void *d)   { (void)d; printf("    Connecting to Redis cache\n"); }
void cache_manager_stop(void *d)    { (void)d; printf("    Flushing cache, disconnecting\n"); }

void destroy_user_service(void)   { printf("    UserService.cleanup() — clearing in-memory state\n"); }
void destroy_order_service(void)  { printf("    OrderService.cleanup()\n"); }
void destroy_jdbc_template(void)  { printf("    JdbcTemplate: no cleanup needed\n"); }
void destroy_datasource(void)     { printf("    DataSource.close() — returning connections to pool\n"); }
void destroy_hikari_pool(void)    { printf("    HikariCP: closing %d connections\n", 10); }

/* ═══════════════════════════════════════════════════════════
 *  GRACEFUL SHUTDOWN ORCHESTRATOR
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    RequestTracker   *request_tracker;
    LifecycleRegistry *lifecycle;
    DestroyRegistry  *destroyables;
    int               grace_period_ms;
} GracefulShutdown;

static void graceful_shutdown_run(GracefulShutdown *gs) {
    clock_t start = clock();

    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  SIGTERM received — Initiating Graceful Shutdown    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* Step 1: Pause server (stop accepting new requests) */
    printf("[GracefulShutdown] Phase 1: Pausing server connector\n");
    gs->request_tracker->accepting = 0;
    printf("  New connections: REJECTED\n");
    printf("  Active requests: %d\n\n", gs->request_tracker->active_count);

    /* Step 2: Drain in-flight requests */
    printf("[GracefulShutdown] Phase 2: Draining in-flight requests (timeout: %d ms)\n",
           gs->grace_period_ms);

    int waited_ms = 0;
    int drain_interval_ms = 50;

    while (gs->request_tracker->active_count > 0) {
        if (waited_ms >= gs->grace_period_ms) {
            printf("  TIMEOUT: %d request(s) still active after %d ms — forcing shutdown\n",
                   gs->request_tracker->active_count, gs->grace_period_ms);
            break;
        }
        printf("  Waiting... %d active request(s) remaining (%d ms elapsed)\n",
               gs->request_tracker->active_count, waited_ms);
        /* Simulate request completing */
        if (gs->request_tracker->active_count > 0) {
            gs->request_tracker->active_count--;
        }
        waited_ms += drain_interval_ms;
    }

    if (gs->request_tracker->active_count == 0) {
        printf("  All in-flight requests completed\n\n");
    }

    /* Step 3: Stop SmartLifecycle beans */
    lifecycle_stop_all(gs->lifecycle);

    /* Step 4: Destroy @PreDestroy beans */
    destroy_all(gs->destroyables);

    double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC * 1000.0;
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Shutdown complete in %.0f ms                        \n", elapsed);
    printf("║  Total requests served: %d                           \n",
           gs->request_tracker->total_received);
    printf("╚══════════════════════════════════════════════════════╝\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMO
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  shutdown_hook.c — Spring Boot Graceful Shutdown     ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* --- Setup: simulate a running application --- */
    RequestTracker rt = { .active_count = 0, .total_received = 0, .accepting = 1 };
    LifecycleRegistry lifecycle = {0};
    DestroyRegistry destroyables = {0};

    /* Register SmartLifecycle beans in startup order */
    lifecycle_register(&lifecycle, "cacheManager",    -100, cache_manager_start, cache_manager_stop, NULL);
    lifecycle_register(&lifecycle, "schedulerManager",   0, scheduler_start,     scheduler_stop,     NULL);
    lifecycle_register(&lifecycle, "kafkaConsumers",  2147, kafka_consumer_start,kafka_consumer_stop, NULL);

    /* Register @PreDestroy beans in creation order */
    destroy_register(&destroyables, "hikariDataSource", 1, destroy_hikari_pool);
    destroy_register(&destroyables, "jdbcTemplate",     2, destroy_jdbc_template);
    destroy_register(&destroyables, "orderService",     3, destroy_order_service);
    destroy_register(&destroyables, "userService",      4, destroy_user_service);
    destroy_register(&destroyables, "dataSource",       5, destroy_datasource);

    /* Simulate startup */
    printf("=== Application Starting ===\n\n");
    lifecycle_start_all(&lifecycle);
    printf("=== Application Running on port 8080 ===\n\n");

    /* Simulate some in-flight requests at shutdown time */
    printf("=== Simulating active requests ===\n");
    request_accept(&rt, "GET /api/orders/process");
    request_accept(&rt, "POST /api/payment/charge");
    request_accept(&rt, "GET /api/users");
    printf("\n");

    /* Simulate SIGTERM */
    GracefulShutdown gs = {
        .request_tracker = &rt,
        .lifecycle       = &lifecycle,
        .destroyables    = &destroyables,
        .grace_period_ms = 30000,
    };
    graceful_shutdown_run(&gs);

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  server.shutdown=graceful enables drain-before-exit\n");
    printf("  Phase 1: pause connector (no new requests)\n");
    printf("  Phase 2: drain in-flight (up to timeout)\n");
    printf("  Phase 3: stop SmartLifecycle (highest phase first)\n");
    printf("  Phase 4: @PreDestroy (reverse creation order)\n");
    printf("  Kubernetes: terminationGracePeriodSeconds > spring timeout\n");
    return 0;
}
