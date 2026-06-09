/*
 * metrics_registry.c  —  Spring Boot Micrometer Metrics Registry in C
 *
 * Build:  gcc -Wall -Wextra -o metrics_registry metrics_registry.c -lm
 * Run:    ./metrics_registry
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   io.micrometer.core.instrument.MeterRegistry
 *   io.micrometer.core.instrument.Counter
 *   io.micrometer.core.instrument.Timer
 *   io.micrometer.core.instrument.Gauge
 *
 *   Micrometer is the metrics facade Spring Boot uses.
 *   It provides a vendor-neutral API that backends (Prometheus, Datadog,
 *   CloudWatch, InfluxDB) implement.
 *
 *   Spring Boot auto-registers JVM metrics automatically:
 *     jvm.memory.used, jvm.gc.pause, jvm.threads.live,
 *     process.cpu.usage, system.cpu.usage,
 *     http.server.requests (via WebMvcMetricsFilter),
 *     hikaricp.connections.* (via HikariCP integration)
 *
 * METER TYPES:
 *   Counter  — monotonically increasing count
 *              e.g., total HTTP requests, total errors, total logins
 *
 *   Gauge    — instantaneous value (can go up or down)
 *              e.g., current queue size, active connections, cache size
 *
 *   Timer    — records duration AND count (histogram + summary)
 *              e.g., HTTP request latency, DB query time, method duration
 *
 *   DistributionSummary — records distribution of values
 *              e.g., request payload size, response body size
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  TAGS — Micrometer's way of dimensioning metrics
 *  Java: Tags.of("method", "GET", "uri", "/api/users", "status", "200")
 *  Tags make one metric useful across many dimensions.
 * ═══════════════════════════════════════════════════════════ */
#define MAX_TAGS 8
#define TAG_LEN  64

typedef struct {
    char key[TAG_LEN];
    char value[TAG_LEN];
} Tag;

typedef struct {
    Tag tags[MAX_TAGS];
    int count;
} TagSet;

static TagSet tags_of(const char *k1, const char *v1) {
    TagSet t = {0};
    strncpy(t.tags[0].key,   k1, TAG_LEN-1);
    strncpy(t.tags[0].value, v1, TAG_LEN-1);
    t.count = 1;
    return t;
}

static TagSet tags_of2(const char *k1, const char *v1,
                        const char *k2, const char *v2) {
    TagSet t = tags_of(k1, v1);
    strncpy(t.tags[1].key,   k2, TAG_LEN-1);
    strncpy(t.tags[1].value, v2, TAG_LEN-1);
    t.count = 2;
    return t;
}

static TagSet tags_of3(const char *k1, const char *v1,
                        const char *k2, const char *v2,
                        const char *k3, const char *v3) {
    TagSet t = tags_of2(k1, v1, k2, v2);
    strncpy(t.tags[2].key,   k3, TAG_LEN-1);
    strncpy(t.tags[2].value, v3, TAG_LEN-1);
    t.count = 3;
    return t;
}

/* ═══════════════════════════════════════════════════════════
 *  METER TYPES
 * ═══════════════════════════════════════════════════════════ */
typedef enum { METER_COUNTER, METER_GAUGE, METER_TIMER } MeterType;

/* ── Counter ── */
typedef struct {
    char   name[128];
    TagSet tags;
    double count;   /* always increasing */
} Counter;

/* ── Gauge ── */
typedef struct {
    char   name[128];
    TagSet tags;
    double value;   /* current snapshot */
} Gauge;

/* ── Timer (simplified histogram) ── */
#define MAX_RECORDINGS 256

typedef struct {
    char   name[128];
    TagSet tags;
    long   count;
    double total_ms;      /* sum of all recorded durations */
    double max_ms;        /* maximum recorded duration     */
    double recordings[MAX_RECORDINGS];
    int    recording_count;
} Timer;

/* ═══════════════════════════════════════════════════════════
 *  METER REGISTRY
 *  Java: SimpleMeterRegistry (in-memory) or PrometheusMeterRegistry etc.
 * ═══════════════════════════════════════════════════════════ */
#define MAX_METERS 64

typedef struct {
    MeterType type;
    union {
        Counter counter;
        Gauge   gauge;
        Timer   timer;
    } meter;
} MeterEntry;

typedef struct {
    MeterEntry entries[MAX_METERS];
    int        count;
    const char *name;   /* e.g. "PrometheusMeterRegistry" */
} MeterRegistry;

/* ─────────────────────────────────────────────────────────────
 *  Registry operations
 * ──────────────────────────────────────────────────────────── */
static Counter* registry_counter(MeterRegistry *r, const char *name, TagSet tags) {
    /* Find existing */
    for (int i = 0; i < r->count; i++) {
        if (r->entries[i].type == METER_COUNTER &&
            strcmp(r->entries[i].meter.counter.name, name) == 0) {
            return &r->entries[i].meter.counter;
        }
    }
    /* Create new */
    if (r->count >= MAX_METERS) return NULL;
    MeterEntry *e = &r->entries[r->count++];
    e->type = METER_COUNTER;
    strncpy(e->meter.counter.name, name, 127);
    e->meter.counter.tags  = tags;
    e->meter.counter.count = 0;
    return &e->meter.counter;
}

static Gauge* registry_gauge(MeterRegistry *r, const char *name, TagSet tags) {
    for (int i = 0; i < r->count; i++) {
        if (r->entries[i].type == METER_GAUGE &&
            strcmp(r->entries[i].meter.gauge.name, name) == 0) {
            return &r->entries[i].meter.gauge;
        }
    }
    if (r->count >= MAX_METERS) return NULL;
    MeterEntry *e = &r->entries[r->count++];
    e->type = METER_GAUGE;
    strncpy(e->meter.gauge.name, name, 127);
    e->meter.gauge.tags  = tags;
    e->meter.gauge.value = 0;
    return &e->meter.gauge;
}

static Timer* registry_timer(MeterRegistry *r, const char *name, TagSet tags) {
    for (int i = 0; i < r->count; i++) {
        if (r->entries[i].type == METER_TIMER &&
            strcmp(r->entries[i].meter.timer.name, name) == 0) {
            return &r->entries[i].meter.timer;
        }
    }
    if (r->count >= MAX_METERS) return NULL;
    MeterEntry *e = &r->entries[r->count++];
    e->type = METER_TIMER;
    memset(&e->meter.timer, 0, sizeof(Timer));
    strncpy(e->meter.timer.name, name, 127);
    e->meter.timer.tags = tags;
    return &e->meter.timer;
}

static void counter_increment(Counter *c, double amount) {
    c->count += amount;
}

static void gauge_set(Gauge *g, double value) {
    g->value = value;
}

static void timer_record(Timer *t, double duration_ms) {
    t->count++;
    t->total_ms += duration_ms;
    if (duration_ms > t->max_ms) t->max_ms = duration_ms;
    if (t->recording_count < MAX_RECORDINGS) {
        t->recordings[t->recording_count++] = duration_ms;
    }
}

/* Compute percentile from recorded values */
static double timer_percentile(const Timer *t, double p) {
    if (t->recording_count == 0) return 0.0;
    /* Simple: copy + sort */
    double sorted[MAX_RECORDINGS];
    memcpy(sorted, t->recordings,
           (size_t)t->recording_count * sizeof(double));
    /* Bubble sort (simple for small arrays) */
    for (int i = 0; i < t->recording_count - 1; i++) {
        for (int j = 0; j < t->recording_count - 1 - i; j++) {
            if (sorted[j] > sorted[j+1]) {
                double tmp = sorted[j]; sorted[j] = sorted[j+1]; sorted[j+1] = tmp;
            }
        }
    }
    int idx = (int)(p * (t->recording_count - 1));
    return sorted[idx];
}

/* ─────────────────────────────────────────────────────────────
 *  Print metrics in Prometheus text format
 *  Real: PrometheusMeterRegistry.scrape() → /actuator/prometheus
 * ──────────────────────────────────────────────────────────── */
static void registry_scrape(const MeterRegistry *r) {
    printf("# Prometheus text format — GET /actuator/prometheus\n\n");

    for (int i = 0; i < r->count; i++) {
        const MeterEntry *e = &r->entries[i];

        if (e->type == METER_COUNTER) {
            const Counter *c = &e->meter.counter;
            printf("# TYPE %s counter\n", c->name);
            printf("%s", c->name);
            if (c->tags.count > 0) {
                printf("{");
                for (int t = 0; t < c->tags.count; t++) {
                    printf("%s=\"%s\"", c->tags.tags[t].key, c->tags.tags[t].value);
                    if (t < c->tags.count - 1) printf(",");
                }
                printf("}");
            }
            printf(" %.0f\n\n", c->count);
        }
        else if (e->type == METER_GAUGE) {
            const Gauge *g = &e->meter.gauge;
            printf("# TYPE %s gauge\n", g->name);
            printf("%s", g->name);
            if (g->tags.count > 0) {
                printf("{");
                for (int t = 0; t < g->tags.count; t++) {
                    printf("%s=\"%s\"", g->tags.tags[t].key, g->tags.tags[t].value);
                    if (t < g->tags.count - 1) printf(",");
                }
                printf("}");
            }
            printf(" %.2f\n\n", g->value);
        }
        else if (e->type == METER_TIMER) {
            const Timer *tm = &e->meter.timer;
            if (tm->count == 0) continue;
            double mean = tm->count > 0 ? tm->total_ms / tm->count : 0;
            printf("# TYPE %s summary\n", tm->name);
            printf("%s_count %.0f\n",    tm->name, (double)tm->count);
            printf("%s_sum %.3f\n",      tm->name, tm->total_ms);
            printf("%s_mean %.3f\n",     tm->name, mean);
            printf("%s_max %.3f\n",      tm->name, tm->max_ms);
            printf("%s{quantile=\"0.50\"} %.3f\n", tm->name, timer_percentile(tm, 0.50));
            printf("%s{quantile=\"0.90\"} %.3f\n", tm->name, timer_percentile(tm, 0.90));
            printf("%s{quantile=\"0.95\"} %.3f\n", tm->name, timer_percentile(tm, 0.95));
            printf("%s{quantile=\"0.99\"} %.3f\n\n", tm->name, timer_percentile(tm, 0.99));
        }
    }
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_http_metrics(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: HTTP Request Metrics (auto-registered by Spring Boot)\n");
    printf("Java: WebMvcMetricsFilter auto-records every HTTP request\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    MeterRegistry registry = {0};
    registry.name = "PrometheusMeterRegistry";

    /* Simulate requests across different endpoints */
    struct { const char *method; const char *uri; int status; double ms; } requests[] = {
        {"GET",    "/api/users",     200,  45.2},
        {"GET",    "/api/users",     200,  38.7},
        {"GET",    "/api/users",     200, 120.5},
        {"POST",   "/api/users",     201, 210.3},
        {"GET",    "/api/users/{id}",200,  52.1},
        {"GET",    "/api/users/{id}",404,  12.8},
        {"DELETE", "/api/users/{id}",200, 188.4},
        {"GET",    "/api/users",     200,  41.9},
        {"GET",    "/api/users",     500, 501.2},  /* error */
    };

    printf("Simulating %d HTTP requests...\n\n", 9);

    for (int i = 0; i < 9; i++) {
        char status_str[8];
        snprintf(status_str, sizeof(status_str), "%d", requests[i].status);

        /* http.server.requests — auto-recorded by WebMvcMetricsFilter */
        TagSet t = tags_of3("method",  requests[i].method,
                             "uri",     requests[i].uri,
                             "status",  status_str);
        Timer *timer = registry_timer(&registry, "http_server_requests", t);
        timer_record(timer, requests[i].ms);
    }

    /* JVM gauges (auto-registered by JvmMetricsAutoConfiguration) */
    TagSet jvm_tags = tags_of("area", "heap");
    Gauge *heap_used = registry_gauge(&registry, "jvm_memory_used_bytes", jvm_tags);
    gauge_set(heap_used, 125829120.0);   /* ~120 MB */

    Gauge *heap_max = registry_gauge(&registry, "jvm_memory_max_bytes", jvm_tags);
    gauge_set(heap_max, 536870912.0);    /* ~512 MB */

    /* Active HTTP connections gauge */
    TagSet conn_tags = {0};
    Gauge *active_conn = registry_gauge(&registry, "tomcat_connections_active", conn_tags);
    gauge_set(active_conn, 42.0);

    /* /actuator/metrics/http.server.requests */
    printf("GET /actuator/metrics/http.server.requests\n\n");
    for (int i = 0; i < registry.count; i++) {
        if (registry.entries[i].type == METER_TIMER) {
            const Timer *t = &registry.entries[i].meter.timer;
            printf("  %-12s %-25s → count=%-4ld mean=%.1fms p95=%.1fms max=%.1fms\n",
                   t->tags.count >= 1 ? t->tags.tags[0].value : "",
                   t->tags.count >= 2 ? t->tags.tags[1].value : "",
                   t->count,
                   t->count > 0 ? t->total_ms / t->count : 0,
                   timer_percentile(t, 0.95),
                   t->max_ms);
        }
    }
    printf("\n");

    printf("Prometheus scrape output:\n");
    registry_scrape(&registry);
}

void demo_custom_metrics(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Custom Business Metrics\n");
    printf("Java: inject MeterRegistry → register your own counters/timers\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    MeterRegistry registry = {0};

    /* Counter: track login attempts by result */
    Counter *login_success = registry_counter(&registry, "auth_login_attempts",
                                               tags_of("result", "success"));
    Counter *login_fail    = registry_counter(&registry, "auth_login_attempts",
                                               tags_of("result", "failure"));

    /* Gauge: active sessions */
    Gauge *active_sessions = registry_gauge(&registry, "auth_active_sessions",
                                             (TagSet){0});

    /* Timer: payment processing */
    Timer *payment_timer = registry_timer(&registry, "payment_processing_seconds",
                                           tags_of("gateway", "stripe"));

    /* Simulate activity */
    printf("Simulating application activity...\n");
    counter_increment(login_success, 1450);
    counter_increment(login_fail,      87);
    gauge_set(active_sessions, 342.0);

    double payment_durations[] = {123.5, 234.1, 98.7, 456.2, 187.3,
                                   210.0, 145.8, 312.4, 89.1, 567.8};
    for (int i = 0; i < 10; i++) {
        timer_record(payment_timer, payment_durations[i]);
    }

    printf("\nMetrics registered: %d\n\n", registry.count);
    printf("GET /actuator/metrics summary:\n\n");

    for (int i = 0; i < registry.count; i++) {
        const MeterEntry *e = &registry.entries[i];
        if (e->type == METER_COUNTER) {
            printf("  counter: %-40s = %.0f\n",
                   e->meter.counter.name, e->meter.counter.count);
        } else if (e->type == METER_GAUGE) {
            printf("  gauge:   %-40s = %.0f\n",
                   e->meter.gauge.name, e->meter.gauge.value);
        } else if (e->type == METER_TIMER) {
            const Timer *t = &e->meter.timer;
            printf("  timer:   %-40s count=%ld mean=%.1fms p99=%.1fms\n",
                   t->name, t->count,
                   t->count > 0 ? t->total_ms / t->count : 0,
                   timer_percentile(t, 0.99));
        }
    }
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  metrics_registry.c — Spring Boot Micrometer in C   ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_http_metrics();
    demo_custom_metrics();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Counter = monotonically increasing count\n");
    printf("  Gauge   = current value snapshot (can decrease)\n");
    printf("  Timer   = duration + count + percentiles\n");
    printf("  Tags    = dimensions for slicing metrics (method, uri, status)\n");
    printf("  Spring Boot auto-registers JVM + HTTP metrics\n");
    printf("  MeterRegistry = abstract API; Prometheus/Datadog implement it\n");
    printf("  /actuator/prometheus → Prometheus scrape endpoint\n");
    return 0;
}
