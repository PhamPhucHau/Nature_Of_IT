/*
 * tracing_context.c  —  Spring Boot Distributed Tracing in C
 *
 * Build:  gcc -Wall -Wextra -o tracing_context tracing_context.c
 * Run:    ./tracing_context
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   io.micrometer.tracing.Tracer
 *   io.micrometer.tracing.Span
 *   io.micrometer.tracing.TraceContext
 *
 *   Spring Boot 3.x auto-registers an ObservationRegistry.
 *   Every HTTP request, DB query, and @Observed method gets:
 *     - A Span: {traceId, spanId, parentSpanId, startTime, endTime, tags}
 *     - Automatic propagation via HTTP headers (W3C traceparent)
 *
 *   Key: TraceId is the SAME across ALL spans in a request,
 *        even across microservice boundaries.
 *        SpanId is unique per operation.
 *        ParentSpanId links spans into a tree.
 *
 * C APPROACH:
 *   We use a thread-local-like struct (single-threaded simulation)
 *   to propagate trace context through function calls.
 *   This mirrors how Spring uses ThreadLocal<TraceContext>.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════
 *  TRACE CONTEXT
 *  Java: io.micrometer.tracing.TraceContext
 *        Stored in ThreadLocal<Span> in real Spring.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    char trace_id[33];    /* 128-bit hex = 32 chars + null */
    char span_id[17];     /* 64-bit hex  = 16 chars + null */
    char parent_span_id[17];
    int  sampled;         /* 1 = record this trace, 0 = skip (sampling) */
} TraceContext;

/* ═══════════════════════════════════════════════════════════
 *  SPAN  — one unit of work in the trace
 *  Java: io.micrometer.tracing.Span
 * ═══════════════════════════════════════════════════════════ */
#define MAX_TAGS 8
#define MAX_EVENTS 8

typedef struct {
    char         name[64];
    TraceContext ctx;
    clock_t      start;
    clock_t      end;
    double       duration_ms;

    /* Tags (low cardinality key-value labels) */
    char tag_keys[MAX_TAGS][32];
    char tag_vals[MAX_TAGS][64];
    int  tag_count;

    /* Events (timestamped log entries inside the span) */
    char event_names[MAX_EVENTS][64];
    int  event_count;

    int  is_error;
    char error_message[128];
} Span;

/* ═══════════════════════════════════════════════════════════
 *  SPAN REGISTRY — holds completed spans for reporting
 * ═══════════════════════════════════════════════════════════ */
#define MAX_SPANS 32

typedef struct {
    Span spans[MAX_SPANS];
    int  count;
} SpanRegistry;

static SpanRegistry g_registry = {0};

/* ═══════════════════════════════════════════════════════════
 *  TRACER — creates and manages spans
 *  Java: Tracer (Brave/OTEL implementation)
 * ═══════════════════════════════════════════════════════════ */

/* Pseudo-random hex generator (deterministic for demo) */
static unsigned int seed_val = 0xdeadbeef;
static unsigned int next_rand(void) {
    seed_val ^= seed_val << 13;
    seed_val ^= seed_val >> 17;
    seed_val ^= seed_val << 5;
    return seed_val;
}

static void gen_hex(char *out, int bytes) {
    static const char *hex = "0123456789abcdef";
    for (int i = 0; i < bytes * 2; i++) {
        out[i] = hex[next_rand() & 0xf];
    }
    out[bytes * 2] = '\0';
}

/* Start a new root span (new trace) */
static Span* span_start_root(const char *name) {
    if (g_registry.count >= MAX_SPANS) return NULL;
    Span *s = &g_registry.spans[g_registry.count++];
    memset(s, 0, sizeof(Span));

    strncpy(s->name, name, 63);
    gen_hex(s->ctx.trace_id,  16);  /* 128-bit trace ID */
    gen_hex(s->ctx.span_id,    8);  /*  64-bit span ID  */
    s->ctx.parent_span_id[0] = '\0';  /* no parent */
    s->ctx.sampled = 1;
    s->start = clock();
    return s;
}

/* Start a child span (inherits traceId, parent = current spanId) */
static Span* span_start_child(const char *name, const TraceContext *parent_ctx) {
    if (g_registry.count >= MAX_SPANS) return NULL;
    Span *s = &g_registry.spans[g_registry.count++];
    memset(s, 0, sizeof(Span));

    strncpy(s->name, name, 63);
    strncpy(s->ctx.trace_id,       parent_ctx->trace_id,  32);  /* SAME trace ID */
    gen_hex(s->ctx.span_id, 8);                                   /* new span ID  */
    strncpy(s->ctx.parent_span_id, parent_ctx->span_id,   16);  /* link to parent */
    s->ctx.sampled = parent_ctx->sampled;
    s->start = clock();
    return s;
}

/* Add tag to span */
static void span_tag(Span *s, const char *key, const char *val) {
    if (s->tag_count < MAX_TAGS) {
        strncpy(s->tag_keys[s->tag_count], key, 31);
        strncpy(s->tag_vals[s->tag_count], val, 63);
        s->tag_count++;
    }
}

/* Add event (annotation) to span */
static void span_event(Span *s, const char *name) {
    if (s->event_count < MAX_EVENTS) {
        strncpy(s->event_names[s->event_count++], name, 63);
    }
}

/* End the span */
static void span_end(Span *s) {
    s->end        = clock();
    s->duration_ms = (double)(s->end - s->start) / CLOCKS_PER_SEC * 1000.0;
}

/* Mark span as error */
static void span_error(Span *s, const char *message) {
    s->is_error = 1;
    strncpy(s->error_message, message, 127);
    span_tag(s, "error", "true");
}

/* ─────────────────────────────────────────────────────────────
 *  Print the full trace as a waterfall diagram
 *  (Like Zipkin/Jaeger UI)
 * ──────────────────────────────────────────────────────────── */
static void print_trace(const SpanRegistry *reg) {
    if (reg->count == 0) return;

    /* Find root span (no parent) */
    const Span *root = NULL;
    for (int i = 0; i < reg->count; i++) {
        if (reg->spans[i].ctx.parent_span_id[0] == '\0') {
            root = &reg->spans[i];
            break;
        }
    }
    if (!root) root = &reg->spans[0];

    printf("\n  Trace: %s\n", root->ctx.trace_id);
    printf("  ┌─────────────────────────────────────────────────────────────\n");
    printf("  │  %-30s  %-8s  %-8s  %s\n",
           "Span Name", "SpanId", "Duration", "Tags");
    printf("  │  %-30s  %-8s  %-8s  %s\n",
           "──────────────────────────────",
           "────────", "────────", "────");

    /* Print spans in registration order (which is call order) */
    for (int i = 0; i < reg->count; i++) {
        const Span *s = &reg->spans[i];
        int depth = 0;
        /* Calculate indentation by counting ancestors */
        const char *pid = s->ctx.parent_span_id;
        while (pid[0] != '\0') {
            depth++;
            /* Find parent span */
            int found = 0;
            for (int j = 0; j < reg->count; j++) {
                if (strcmp(reg->spans[j].ctx.span_id, pid) == 0) {
                    pid = reg->spans[j].ctx.parent_span_id;
                    found = 1;
                    break;
                }
            }
            if (!found) break;
            if (depth > 10) break;
        }

        char indent[64] = "";
        for (int d = 0; d < depth; d++) strcat(indent, "  ");
        char full_name[64];
        snprintf(full_name, sizeof(full_name), "%s%s%s",
                 indent,
                 depth > 0 ? "└─ " : "",
                 s->name);

        char tag_str[64] = "";
        for (int t = 0; t < s->tag_count && t < 2; t++) {
            if (t > 0) strcat(tag_str, " ");
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%s=%s", s->tag_keys[t], s->tag_vals[t]);
            strncat(tag_str, tmp, sizeof(tag_str) - strlen(tag_str) - 1);
        }

        printf("  │  %-30s  %.6s  %5.1fms  %s%s\n",
               full_name,
               s->ctx.span_id,
               s->duration_ms,
               tag_str,
               s->is_error ? " ❌ ERROR" : "");
    }
    printf("  └─────────────────────────────────────────────────────────────\n\n");
}

/* ─────────────────────────────────────────────────────────────
 *  Emit a structured log line with trace context
 * ──────────────────────────────────────────────────────────── */
static void structured_log(const char *level, const char *logger,
                             const char *message, const TraceContext *ctx) {
    printf("  {\"level\":\"%s\",\"logger\":\"%s\",\"message\":\"%s\","
           "\"traceId\":\"%s\",\"spanId\":\"%s\"}\n",
           level, logger, message,
           ctx ? ctx->trace_id : "",
           ctx ? ctx->span_id  : "");
}

/* ═══════════════════════════════════════════════════════════
 *  SIMULATED SERVICE CALLS
 *  Each function represents a layer in the application.
 * ═══════════════════════════════════════════════════════════ */

static void db_query(const TraceContext *parent_ctx, const char *sql,
                      double sim_duration_ms) {
    Span *span = span_start_child("db.query", parent_ctx);
    span_tag(span, "db.system",    "mysql");
    span_tag(span, "db.statement", sql);
    structured_log("DEBUG", "JdbcTemplate", sql, &span->ctx);

    /* Simulate query time */
    (void)sim_duration_ms;

    span_end(span);
    structured_log("DEBUG", "JdbcTemplate", "Query completed", &span->ctx);
}

static void inventory_service_call(const TraceContext *parent_ctx,
                                    const char *product_id) {
    Span *span = span_start_child("inventory-service.check", parent_ctx);
    span_tag(span, "peer.service", "inventory-service");
    span_tag(span, "http.method",  "GET");

    char url[64];
    snprintf(url, sizeof(url), "/inventory/%s", product_id);
    span_tag(span, "http.url", url);

    structured_log("INFO", "RestTemplate",
                   "Calling inventory service", &span->ctx);

    /* Nested: Redis cache check inside inventory service */
    Span *redis_span = span_start_child("redis.get", &span->ctx);
    span_tag(redis_span, "db.system", "redis");
    span_tag(redis_span, "db.operation", "GET");
    structured_log("DEBUG", "RedisTemplate", "Cache lookup", &redis_span->ctx);
    span_end(redis_span);

    span_event(span, "cache.miss");
    structured_log("DEBUG", "RestTemplate", "Cache miss, fetching from DB", &span->ctx);

    span_end(span);
}

static void order_service_create(const TraceContext *parent_ctx,
                                  const char *order_id) {
    Span *span = span_start_child("OrderService.createOrder", parent_ctx);
    span_tag(span, "order.id", order_id);

    structured_log("INFO", "OrderService", "Creating order", &span->ctx);

    /* DB: save order */
    db_query(&span->ctx, "INSERT INTO orders ...", 8.0);

    /* External: check inventory */
    inventory_service_call(&span->ctx, "prod-abc-123");

    /* DB: update inventory */
    db_query(&span->ctx, "UPDATE inventory SET quantity = quantity - 1 ...", 5.0);

    span_event(span, "order.created");
    structured_log("INFO", "OrderService", "Order created successfully", &span->ctx);

    span_end(span);
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_full_trace(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: Distributed Trace — POST /api/orders\n");
    printf("Shows: TraceId propagation across service layers + DB calls\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    g_registry.count = 0;  /* reset */

    /* Root span: HTTP request hits the controller */
    Span *root = span_start_root("POST /api/orders");
    span_tag(root, "http.method", "POST");
    span_tag(root, "http.url",    "/api/orders");

    structured_log("INFO", "DispatcherServlet",
                   "Received POST /api/orders", &root->ctx);

    /* Service layer */
    order_service_create(&root->ctx, "ord-2024-001");

    span_end(root);

    structured_log("INFO", "DispatcherServlet",
                   "Response: 201 Created", &root->ctx);

    printf("\n  Structured logs emitted during this request:\n");
    printf("  (note: all share the same traceId = '%s')\n\n",
           root->ctx.trace_id);

    printf("\n  Waterfall view (Zipkin/Jaeger):\n");
    print_trace(&g_registry);

    printf("  Total spans recorded: %d\n", g_registry.count);
    printf("  Same traceId across all spans: %s\n\n",
           root->ctx.trace_id);
}

void demo_error_trace(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Error Trace — exception in service layer\n");
    printf("Shows: error span marking + exception details in trace\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    g_registry.count = 0;

    Span *root = span_start_root("GET /api/users/999");
    span_tag(root, "http.method", "GET");
    span_tag(root, "http.url",    "/api/users/999");

    structured_log("INFO", "DispatcherServlet",
                   "Received GET /api/users/999", &root->ctx);

    Span *svc = span_start_child("UserService.findById", &root->ctx);
    span_tag(svc, "user.id", "999");

    db_query(&svc->ctx, "SELECT * FROM users WHERE id = 999", 3.0);

    /* Exception: user not found */
    span_error(svc, "UserNotFoundException: User 999 not found");
    structured_log("WARN", "UserService",
                   "UserNotFoundException: user 999 not found", &svc->ctx);
    span_end(svc);

    span_error(root, "HTTP 404 Not Found");
    span_tag(root, "http.status", "404");
    structured_log("WARN", "DispatcherServlet",
                   "Response: 404 Not Found", &root->ctx);
    span_end(root);

    printf("  Waterfall view:\n");
    print_trace(&g_registry);
}

void demo_traceparent_header(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: W3C traceparent Header Propagation\n");
    printf("When service A calls service B, it injects trace context\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    g_registry.count = 0;
    Span *span = span_start_root("order-service.createOrder");
    span_end(span);

    printf("  Outgoing HTTP request to inventory-service:\n");
    printf("  Headers injected by Spring Boot ObservationRegistry:\n\n");
    printf("    traceparent: 00-%s-%s-01\n",
           span->ctx.trace_id, span->ctx.span_id);
    printf("    b3: %s-%s-1\n\n",  /* Zipkin B3 format */
           span->ctx.trace_id, span->ctx.span_id);

    printf("  inventory-service receives this request:\n");
    printf("    → Extracts traceId: %s\n", span->ctx.trace_id);
    printf("    → Creates child span with parentSpanId: %s\n",
           span->ctx.span_id);
    printf("    → All logs emitted in inventory-service share traceId\n");
    printf("    → Trace appears as ONE connected tree in Zipkin/Jaeger\n\n");
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  tracing_context.c — Spring Boot Tracing in C       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_full_trace();
    demo_error_trace();
    demo_traceparent_header();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  TraceId = one ID shared across an entire request chain\n");
    printf("  SpanId  = one operation unit within a trace\n");
    printf("  Parent SpanId = links spans into a tree\n");
    printf("  W3C traceparent header propagates context between services\n");
    printf("  Spring Boot auto-creates spans for HTTP, DB, and @Observed methods\n");
    printf("  All logs within a request include traceId → correlated in Loki/ELK\n");
    return 0;
}
