# 🛑 Phase 8: Graceful Shutdown & Lifecycle

> **Spring Boot Concept**: `server.shutdown=graceful`, `SmartLifecycle`, `@PreDestroy`
> **C Equivalent**: `signal(SIGTERM, handler)` + drain counter + ordered teardown

---

## Why Graceful Shutdown Matters

```text
WITHOUT graceful shutdown (abrupt kill):
  SIGTERM received → JVM exits immediately
  - In-flight HTTP requests: connection reset (502/504 at the load balancer)
  - Active DB transactions: rolled back mid-operation
  - Message queue consumers: messages redelivered (duplicates)
  - File writes: partial/corrupt files
  → Users see errors. Data may be inconsistent.

WITH graceful shutdown (Spring Boot 2.3+):
  SIGTERM received
  → Tomcat stops accepting new connections
  → Wait for in-flight requests to complete (up to timeout)
  → Destroy all beans in reverse order (@PreDestroy)
  → JVM exits cleanly
  → Users don't notice the restart.
```

---

## Configuration

```yaml
server:
  shutdown: graceful        # default is "immediate"

spring:
  lifecycle:
    timeout-per-shutdown-phase: 30s   # max wait per SmartLifecycle phase
```

---

## Shutdown Sequence

```text
SIGTERM (or actuator /actuator/shutdown POST)
  ↓
JVM Shutdown Hook registered by Spring Boot
  ↓
SpringApplication.getShutdownHandlers().run()
  ↓
WebServer.shutDownGracefully()            ← phase 1: stop accepting new requests
  → connector.pause()                     ← Tomcat stops accepting connections
  → wait for active-requests == 0         ← drain in-flight (up to timeout)
  ↓
ApplicationContext.close()                ← phase 2: destroy beans
  → SmartLifecycle beans.stop() in reverse @Order
  → @PreDestroy methods (reverse creation order)
  → DisposableBeans.destroy()
  → BeanFactory.destroySingletons()
  ↓
JVM shutdown hooks (non-Spring)
  ↓
Process exits with code 0
```

---

## SmartLifecycle

```java
@Component
public class KafkaConsumerManager implements SmartLifecycle {
    private volatile boolean running = false;

    @Override
    public void start() {
        running = true;
        startConsumers();
    }

    @Override
    public void stop(Runnable callback) {
        // Async graceful stop: finish current messages, then call callback
        stopConsumers(() -> {
            running = false;
            callback.run();   // ← signals "I'm done" to Spring
        });
    }

    @Override public boolean isRunning() { return running; }
    @Override public int getPhase() { return Integer.MAX_VALUE - 1; }
    // Higher phase = starts LATER, stops FIRST
}
```

---

## Shutdown Hooks in Spring

```java
// 1. @PreDestroy (most common)
@Service
public class CacheManager {
    @PreDestroy
    public void cleanup() {
        cache.flush();   // persist cache before shutdown
        cache.close();
    }
}

// 2. DisposableBean interface
@Service
public class ConnectionPool implements DisposableBean {
    @Override
    public void destroy() {
        pool.closeAllConnections();
    }
}

// 3. destroyMethod attribute
@Bean(destroyMethod = "close")
public DataSource dataSource() { ... }

// 4. SmartLifecycle (for ordered, async shutdown)
// See above
```

---

## Kubernetes Deployment Considerations

```yaml
# k8s deployment: configure terminationGracePeriodSeconds
spec:
  terminationGracePeriodSeconds: 60  # must be > spring timeout

# k8s sends SIGTERM, waits terminationGracePeriodSeconds, then SIGKILL
# Spring Boot must complete within that window
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `shutdown_hook.c` | JVM shutdown hook, signal handling, ordered teardown |
| `request_drain.c` | In-flight request draining with timeout |
| `lifecycle_phases.c` | SmartLifecycle phase ordering |

---

## Build and Run

```bash
gcc -Wall -Wextra -o shutdown_hook shutdown_hook.c && ./shutdown_hook
```

---

## Key Insight

```text
Graceful shutdown = 3 stages:

1. STOP ACCEPTING: close server port (no new requests in)
2. DRAIN:          wait for current requests to finish (up to timeout)
3. DESTROY:        call @PreDestroy on all beans in reverse order

The timeout is critical:
  - Too short: in-flight requests get cut off (user sees errors)
  - Too long: deployment takes forever (rolling restart is slow)
  - Rule of thumb: set to your 99th percentile request time + 5s buffer

In Kubernetes: terminationGracePeriodSeconds must be LONGER than Spring's timeout.
Otherwise: k8s sends SIGKILL before Spring finishes → same as no graceful shutdown.
```
