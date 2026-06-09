# 🔭 Phase 10: Spring Boot Observability

> **Spring Boot Concept**: Micrometer Tracing, structured logging, `@Observed`
> **C Equivalent**: Thread-local context propagation + structured log formatting

---

## The Three Pillars of Observability

```text
┌────────────────────────────────────────────────────────────┐
│                    OBSERVABILITY                           │
│                                                            │
│  METRICS          TRACES            LOGS                   │
│  ─────────        ─────────         ─────                  │
│  What?            Where exactly?    What happened?         │
│  Aggregated       Per request       Per event              │
│  Numbers          Path              Text                   │
│                                                            │
│  Micrometer       Micrometer        Logback /              │
│  → Prometheus     Tracing           Log4j2                 │
│  → Grafana        → Zipkin          → ELK Stack            │
│                   → Jaeger          → Loki                 │
└────────────────────────────────────────────────────────────┘
```

---

## Distributed Tracing (Micrometer Tracing)

```text
Request: POST /api/orders

  ┌─────────────────────────────────────────────────────────┐
  │ TraceId: a1b2c3d4e5f6a7b8  (same across all services)  │
  │                                                         │
  │  Span: api-gateway          SpanId: 0001  0ms  → 5ms   │
  │    └─ Span: order-service   SpanId: 0002  5ms  → 45ms  │
  │         └─ Span: db.insert  SpanId: 0003  8ms  → 15ms  │
  │         └─ Span: inventory  SpanId: 0004  15ms → 40ms  │
  │              └─ Span: redis SpanId: 0005  16ms → 18ms  │
  └─────────────────────────────────────────────────────────┘

  Total time: 45ms
  DB insert took: 7ms
  Inventory service took: 25ms
  Redis lookup: 2ms
```

---

## Trace Context Propagation

```text
HTTP request between services carries trace context in headers:

  traceparent: 00-a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6-0f1e2d3c4b5a6978-01
               version  traceId(32 hex)              spanId(16 hex)   flags

  Spring Boot auto-injects these headers via:
    ObservationRegistry → MicrometerTracingObservationHandler
    → propagates TraceId/SpanId in HTTP headers automatically
```

---

## @Observed — Declarative Tracing

```java
@Observed(name = "user.creation",
          contextualName = "Creating new user",
          lowCardinalityKeyValues = {"userType", "standard"})
public User createUser(CreateUserRequest request) {
    // This method automatically gets:
    // - A span created and closed around it
    // - Duration recorded as a timer
    // - TraceId/SpanId in any logs emitted within this method
    return userRepository.save(new User(request));
}
```

---

## Structured Logging

```text
Traditional log:
  2024-01-15 10:23:45 INFO  UserService - Creating user alice@example.com

Structured JSON log:
  {
    "timestamp": "2024-01-15T10:23:45.123Z",
    "level": "INFO",
    "logger": "com.example.UserService",
    "message": "Creating user",
    "traceId": "a1b2c3d4e5f6a7b8",
    "spanId":  "0f1e2d3c",
    "userId": "alice@example.com",
    "requestId": "req-abc123",
    "service": "order-service",
    "env": "prod"
  }
```

Structured logs enable:
- Full-text search in Elasticsearch/Loki
- Correlation by traceId across services
- Alerting on specific log fields

---

## Spring Boot 3.x Observability Stack

```yaml
dependencies:
  spring-boot-starter-actuator    # MeterRegistry + endpoints
  micrometer-tracing-bridge-brave # Micrometer → Brave/Zipkin bridge
  zipkin-reporter-brave           # sends traces to Zipkin
  micrometer-registry-prometheus  # exports metrics to Prometheus

# application.yml
management:
  tracing:
    sampling:
      probability: 1.0   # trace 100% of requests (dev)
      # probability: 0.1 # trace 10% in production (cost control)
  zipkin:
    tracing:
      endpoint: http://zipkin:9411/api/v2/spans
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `micrometer_registry.c` | *(in phase 6 — reuse)* |
| `tracing_context.c` | Trace/Span propagation across simulated service calls |
| `structured_logging.c` | JSON log format with trace context injection |

---

## Build and Run

```bash
gcc -Wall -Wextra -o tracing_context tracing_context.c && ./tracing_context
gcc -Wall -Wextra -o structured_logging structured_logging.c && ./structured_logging
```

---

## Key Insight

```text
Observability = answer to "what is happening right now in production?"

Metrics  → "99th percentile latency is 2.3s" (something is wrong)
Traces   → "the slow path is in the inventory service DB query" (found it)
Logs     → "NullPointerException in InventoryRepository.findById()" (root cause)

All three linked by TraceId.
Spring Boot 3.x wires all three automatically when you include the right starters.
@Observed is the single annotation that instruments a method for all three.
```
