# 📊 Phase 6: Spring Boot Actuator

> **Spring Boot Concept**: `/actuator/*` endpoints, `HealthIndicator`, `MeterRegistry`
> **C Equivalent**: A set of diagnostic read-only HTTP endpoints backed by live runtime data

---

## What Actuator Provides

```yaml
# application.yml
management:
  endpoints:
    web:
      exposure:
        include: health,info,metrics,beans,env,loggers
  endpoint:
    health:
      show-details: always
```

```text
GET /actuator/health   → aggregated health status
GET /actuator/info     → application metadata (version, git, etc.)
GET /actuator/metrics  → list of all metric names
GET /actuator/metrics/jvm.memory.used → specific metric value
GET /actuator/beans    → all Spring beans in context
GET /actuator/env      → all environment properties (with masking)
GET /actuator/loggers  → logger names + current log levels
GET /actuator/mappings → all @RequestMapping routes
GET /actuator/threaddump → JVM thread dump
GET /actuator/heapdump → JVM heap dump (binary)
GET /actuator/startup  → startup steps timeline
```

---

## Health Endpoint Deep Dive

```text
GET /actuator/health response:

{
  "status": "UP",                    ← aggregated (worst of all components)
  "components": {
    "db": {
      "status": "UP",
      "details": {
        "database": "MySQL",
        "validationQuery": "SELECT 1"
      }
    },
    "diskSpace": {
      "status": "UP",
      "details": {
        "total": 499963174912,
        "free": 248934862848,
        "threshold": 10485760
      }
    },
    "redis": {
      "status": "DOWN",              ← ONE DOWN → aggregate = DOWN
      "details": {
        "error": "Connection refused"
      }
    }
  }
}
```

Status aggregation: DOWN > OUT_OF_SERVICE > UNKNOWN > UP

---

## Built-in HealthIndicators

Spring Boot auto-registers these when the relevant dependency is present:

| Indicator | Checks |
|-----------|--------|
| `DataSourceHealthIndicator` | DB SELECT 1 |
| `DiskSpaceHealthIndicator` | free disk > threshold |
| `RedisHealthIndicator` | Redis PING |
| `MongoHealthIndicator` | MongoDB `isMaster` |
| `ElasticsearchHealthIndicator` | ES cluster status |
| `RabbitHealthIndicator` | RabbitMQ connection |
| `KafkaHealthIndicator` | Kafka broker reachable |

---

## Custom HealthIndicator

```java
@Component
public class ExternalApiHealthIndicator implements HealthIndicator {
    @Override
    public Health health() {
        try {
            int statusCode = callExternalApi();
            if (statusCode == 200) {
                return Health.up()
                    .withDetail("url", "https://api.example.com")
                    .withDetail("responseTime", "45ms")
                    .build();
            }
            return Health.down()
                .withDetail("statusCode", statusCode)
                .build();
        } catch (Exception e) {
            return Health.down(e).build();
        }
    }
}
```

---

## Metrics with Micrometer

```java
@RestController
public class UserController {
    private final Counter requestCounter;
    private final Timer  responseTimer;

    public UserController(MeterRegistry registry) {
        // Counter: counts occurrences
        this.requestCounter = registry.counter("api.users.requests",
            "method", "GET");

        // Timer: measures duration + count
        this.responseTimer = Timer.builder("api.users.latency")
            .tag("endpoint", "getUsers")
            .register(registry);
    }

    @GetMapping("/users")
    public List<User> getUsers() {
        requestCounter.increment();
        return responseTimer.record(() -> userService.findAll());
    }
}
```

---

## Actuator Security

By default, only `/actuator/health` and `/actuator/info` are accessible.
Sensitive endpoints (env, beans, heapdump) require explicit exposure:

```yaml
management:
  endpoints:
    web:
      exposure:
        include: "*"    # expose all (only in dev!)
  endpoint:
    health:
      show-details: when-authorized  # only for authenticated users
```

In production, secure actuator behind a management port:
```yaml
management:
  server:
    port: 8081  # different port, firewalled from public access
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `health_endpoint.c` | HealthIndicator aggregation + status logic |
| `metrics_registry.c` | Counter, Timer, Gauge — Micrometer registry |
| `endpoint_exposure.c` | Endpoint routing, security, response format |

---

## Build and Run

```bash
gcc -Wall -Wextra -o health_endpoint health_endpoint.c && ./health_endpoint
gcc -Wall -Wextra -o metrics_registry metrics_registry.c && ./metrics_registry
```

---

## Key Insight

```text
Actuator endpoints are just @Component beans that:
  1. Implement a specific interface (HealthIndicator, InfoContributor)
  2. Are registered at /actuator/<id> by ActuatorWebMvcEndpointHandlerMapping
  3. Are gated by @ConditionalOnEnabledEndpoint

Health aggregation is a tree-reduce:
  foreach component: call health()
  if any is DOWN → overall = DOWN
  if any is OUT_OF_SERVICE → overall = OUT_OF_SERVICE
  else if any is UNKNOWN → overall = UNKNOWN
  else → overall = UP

Micrometer is the abstraction layer over metric backends:
  your code → MeterRegistry API → Prometheus / Datadog / CloudWatch / etc.
```
