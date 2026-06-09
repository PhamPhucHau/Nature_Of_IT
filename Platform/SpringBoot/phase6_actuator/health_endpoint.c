/*
 * health_endpoint.c  —  Spring Boot Actuator Health Endpoint in C
 *
 * Build:  gcc -Wall -Wextra -o health_endpoint health_endpoint.c
 * Run:    ./health_endpoint
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS SIMULATES:
 *
 *   org.springframework.boot.actuate.health.HealthEndpoint
 *   org.springframework.boot.actuate.health.HealthIndicator
 *   org.springframework.boot.actuate.health.CompositeHealthContributor
 *
 *   Spring Boot's /actuator/health endpoint:
 *     1. Calls health() on ALL registered HealthIndicator beans
 *     2. Aggregates results using StatusAggregator
 *        (worst status wins: DOWN > OUT_OF_SERVICE > UNKNOWN > UP)
 *     3. Returns the composite Health as JSON
 *
 *   Real source:
 *     HealthEndpoint.health()
 *       → HealthContributorRegistry.stream()
 *         → each HealthIndicator.health()
 *           → StatusAggregator.getAggregateStatus()
 *
 * KEY USE CASE:
 *   Kubernetes liveness/readiness probes hit /actuator/health.
 *   If it returns 503 (DOWN), Kubernetes restarts or removes the pod.
 *   This is why correct health indicators are critical in production.
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  HEALTH STATUS
 *  Java: org.springframework.boot.actuate.health.Status
 *  Ordered from worst to best (lower ordinal = worse)
 * ═══════════════════════════════════════════════════════════ */
typedef enum {
    STATUS_DOWN           = 0,   /* worst — component is failed       */
    STATUS_OUT_OF_SERVICE = 1,   /* deliberately offline (maintenance)*/
    STATUS_UNKNOWN        = 2,   /* cannot determine                  */
    STATUS_UP             = 3,   /* healthy (best)                    */
} HealthStatus;

static const char* status_name(HealthStatus s) {
    static const char* names[] = { "DOWN", "OUT_OF_SERVICE", "UNKNOWN", "UP" };
    return names[(int)s < 4 ? (int)s : 3];
}

/* HTTP status code for each health status
 * Spring Boot: 200 for UP/UNKNOWN, 503 for DOWN/OUT_OF_SERVICE */
static int status_http_code(HealthStatus s) {
    return (s == STATUS_UP || s == STATUS_UNKNOWN) ? 200 : 503;
}

/* ═══════════════════════════════════════════════════════════
 *  HEALTH DETAILS — key-value pairs in the response
 * ═══════════════════════════════════════════════════════════ */
#define MAX_DETAILS 8

typedef struct {
    char key[64];
    char value[256];
} HealthDetail;

/* ═══════════════════════════════════════════════════════════
 *  HEALTH — result of one HealthIndicator.health() call
 *  Java: org.springframework.boot.actuate.health.Health
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    HealthStatus status;
    HealthDetail details[MAX_DETAILS];
    int          detail_count;
    char         error_message[256];   /* set if DOWN with exception */
} Health;

static Health health_up(void) {
    Health h = {0};
    h.status = STATUS_UP;
    return h;
}

static Health health_down(const char *error) {
    Health h = {0};
    h.status = STATUS_DOWN;
    if (error) strncpy(h.error_message, error, 255);
    return h;
}

static void health_add_detail(Health *h, const char *key, const char *value) {
    if (h->detail_count < MAX_DETAILS) {
        strncpy(h->details[h->detail_count].key,   key,   63);
        strncpy(h->details[h->detail_count].value, value, 255);
        h->detail_count++;
    }
}

/* ═══════════════════════════════════════════════════════════
 *  HEALTH INDICATOR — interface each component implements
 *  Java: @Component class XxxHealthIndicator implements HealthIndicator
 * ═══════════════════════════════════════════════════════════ */
typedef Health (*HealthIndicatorFn)(void);

#define MAX_INDICATORS 16

typedef struct {
    const char       *name;          /* e.g. "db", "diskSpace", "redis" */
    HealthIndicatorFn check;
} HealthIndicator;

/* ═══════════════════════════════════════════════════════════
 *  BUILT-IN HEALTH INDICATOR IMPLEMENTATIONS
 * ═══════════════════════════════════════════════════════════ */

/* DataSourceHealthIndicator — executes SELECT 1 against each DataSource */
Health check_datasource(void) {
    /* Simulate: DB is UP */
    Health h = health_up();
    health_add_detail(&h, "database", "MySQL");
    health_add_detail(&h, "validationQuery", "isValid()");
    health_add_detail(&h, "result", "1");
    return h;
}

/* DiskSpaceHealthIndicator — checks free disk >= threshold */
Health check_disk_space(void) {
    long total_bytes  = 499963174912L;  /* ~465 GB */
    long free_bytes   = 248934862848L;  /* ~232 GB */
    long threshold    =     10485760L;  /* 10 MB   */

    Health h;
    if (free_bytes >= threshold) {
        h = health_up();
    } else {
        h = health_down("Low disk space");
    }

    char buf[64];
    snprintf(buf, sizeof(buf), "%ld", total_bytes);
    health_add_detail(&h, "total", buf);
    snprintf(buf, sizeof(buf), "%ld", free_bytes);
    health_add_detail(&h, "free", buf);
    snprintf(buf, sizeof(buf), "%ld", threshold);
    health_add_detail(&h, "threshold", buf);
    snprintf(buf, sizeof(buf), "%.1f%%", (double)free_bytes / total_bytes * 100);
    health_add_detail(&h, "freePercent", buf);
    return h;
}

/* RedisHealthIndicator — sends PING command */
Health check_redis(void) {
    /* Simulate: Redis is DOWN (connection refused) */
    Health h = health_down("Redis connection refused: localhost:6379");
    health_add_detail(&h, "error", "java.net.ConnectException: Connection refused");
    return h;
}

/* Custom: External API health indicator */
Health check_external_api(void) {
    /* Simulate: external API responds normally */
    Health h = health_up();
    health_add_detail(&h, "url", "https://api.payment-gateway.com/health");
    health_add_detail(&h, "responseTimeMs", "45");
    health_add_detail(&h, "statusCode", "200");
    return h;
}

/* Custom: Kafka health indicator */
Health check_kafka(void) {
    /* Simulate: Kafka is reachable */
    Health h = health_up();
    health_add_detail(&h, "brokers", "localhost:9092");
    health_add_detail(&h, "topics", "3");
    return h;
}

/* ═══════════════════════════════════════════════════════════
 *  HEALTH ENDPOINT
 *  Aggregates all indicators and produces the composite response.
 * ═══════════════════════════════════════════════════════════ */
typedef struct {
    HealthIndicator indicators[MAX_INDICATORS];
    int             indicator_count;
    int             show_details;   /* management.endpoint.health.show-details */
} HealthEndpoint;

static void health_endpoint_register(HealthEndpoint *ep,
                                      const char *name,
                                      HealthIndicatorFn fn) {
    if (ep->indicator_count >= MAX_INDICATORS) return;
    ep->indicators[ep->indicator_count].name  = name;
    ep->indicators[ep->indicator_count].check = fn;
    ep->indicator_count++;
}

/* StatusAggregator: DOWN < OUT_OF_SERVICE < UNKNOWN < UP */
static HealthStatus aggregate_status(HealthStatus a, HealthStatus b) {
    return (a < b) ? a : b;  /* lower ordinal = worse → wins */
}

static void health_endpoint_call(const HealthEndpoint *ep) {
    printf("  GET /actuator/health\n\n");

    HealthStatus overall = STATUS_UP;  /* start optimistic */

    /* Evaluate all indicators */
    Health results[MAX_INDICATORS];
    for (int i = 0; i < ep->indicator_count; i++) {
        results[i] = ep->indicators[i].check();
        overall = aggregate_status(overall, results[i].status);
    }

    int http_code = status_http_code(overall);

    /* Print JSON response */
    printf("  HTTP %d %s\n", http_code, http_code == 200 ? "OK" : "Service Unavailable");
    printf("  Content-Type: application/json\n\n");
    printf("  {\n");
    printf("    \"status\": \"%s\",\n", status_name(overall));

    if (ep->show_details) {
        printf("    \"components\": {\n");
        for (int i = 0; i < ep->indicator_count; i++) {
            Health *h = &results[i];
            printf("      \"%s\": {\n", ep->indicators[i].name);
            printf("        \"status\": \"%s\"", status_name(h->status));

            if (h->error_message[0]) {
                printf(",\n        \"details\": {\n");
                printf("          \"error\": \"%s\"\n", h->error_message);
                printf("        }");
            } else if (h->detail_count > 0) {
                printf(",\n        \"details\": {\n");
                for (int d = 0; d < h->detail_count; d++) {
                    printf("          \"%s\": \"%s\"%s\n",
                           h->details[d].key,
                           h->details[d].value,
                           d < h->detail_count - 1 ? "," : "");
                }
                printf("        }");
            }

            printf("\n      }%s\n",
                   i < ep->indicator_count - 1 ? "," : "");
        }
        printf("    }\n");
    }
    printf("  }\n\n");

    /* Summary */
    printf("  ── Status Summary ───────────────────────────────────\n");
    for (int i = 0; i < ep->indicator_count; i++) {
        printf("    %-20s → %s %s\n",
               ep->indicators[i].name,
               status_name(results[i].status),
               results[i].status == STATUS_UP ? "✓" : "✗");
    }
    printf("    %-20s → %s  (HTTP %d)\n",
           "OVERALL", status_name(overall), http_code);
    printf("\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_all_up(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 1: All Components Healthy\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    HealthEndpoint ep = {0};
    ep.show_details = 1;
    health_endpoint_register(&ep, "db",          check_datasource);
    health_endpoint_register(&ep, "diskSpace",   check_disk_space);
    health_endpoint_register(&ep, "kafka",        check_kafka);
    health_endpoint_register(&ep, "externalApi", check_external_api);
    health_endpoint_call(&ep);
}

void demo_redis_down(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 2: Redis DOWN → Overall DOWN → HTTP 503\n");
    printf("Kubernetes readiness probe returns 503 → traffic stops\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    HealthEndpoint ep = {0};
    ep.show_details = 1;
    health_endpoint_register(&ep, "db",        check_datasource);
    health_endpoint_register(&ep, "diskSpace", check_disk_space);
    health_endpoint_register(&ep, "redis",     check_redis);        /* DOWN */
    health_endpoint_register(&ep, "kafka",     check_kafka);
    health_endpoint_call(&ep);
}

void demo_no_details(void) {
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("DEMO 3: Production Config (show-details: never)\n");
    printf("management.endpoint.health.show-details=NEVER\n");
    printf("→ Only status shown, no component details (security)\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");

    HealthEndpoint ep = {0};
    ep.show_details = 0;   /* show-details: NEVER */
    health_endpoint_register(&ep, "db",        check_datasource);
    health_endpoint_register(&ep, "diskSpace", check_disk_space);
    health_endpoint_register(&ep, "redis",     check_redis);
    health_endpoint_call(&ep);
}

/* ═══════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════ */
int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  health_endpoint.c — Spring Boot Actuator Health    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_all_up();
    demo_redis_down();
    demo_no_details();

    printf("\n━━━ Key Takeaways ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  HealthEndpoint calls ALL registered HealthIndicators\n");
    printf("  Status aggregation: DOWN < OUT_OF_SERVICE < UNKNOWN < UP\n");
    printf("  ONE DOWN component → entire app returns HTTP 503\n");
    printf("  show-details=NEVER: hides internals in production\n");
    printf("  Kubernetes readiness probe → /actuator/health (503 = remove from LB)\n");
    printf("  Custom HealthIndicator: @Component + implements HealthIndicator\n");
    return 0;
}
