# 🏭 Phase 10: Spring Boot Production Concurrency Patterns

> **Spring Boot Concept**: Thread-safe singletons, HikariCP tuning, deadlock detection, GC pressure
> **Focus**: The bugs and patterns that matter in real production Spring Boot applications

---

## The #1 Bug: Mutable Singleton State

```java
// DANGEROUS — Spring singleton + mutable field = RACE CONDITION
@Service
public class UserService {
    // This list is SHARED across ALL request threads!
    private List<String> recentUsers = new ArrayList<>();  // NOT thread-safe

    @GetMapping("/users")
    public void processUser(String username) {
        recentUsers.add(username);   // ArrayList.add() is NOT thread-safe
        // Two threads adding simultaneously → ConcurrentModificationException
        // Or worse: silent data corruption
    }
}

// FIX 1: Use concurrent collection
private List<String> recentUsers = new CopyOnWriteArrayList<>();

// FIX 2: Make the bean stateless (BEST)
@Service
public class UserService {
    // NO instance fields that change after construction
    // All state is in parameters, method locals, or injected repos
    public void processUser(String username) {
        // all local — no shared state
    }
}

// FIX 3: Use request scope
@Service @Scope("request")  // new instance per HTTP request
public class UserContextService {
    private String currentUser;  // safe — one instance per request
}
```

---

## HikariCP Thread Pool Sizing

```yaml
# application.yml
spring:
  datasource:
    hikari:
      maximum-pool-size: 20   # formula: (CPU × 2) + effective_spindle_count
      minimum-idle: 10
      connection-timeout: 30000  # 30s — reduce if DB is fast

server:
  tomcat:
    threads:
      max: 200     # Tomcat threads
```

```text
DEADLOCK SCENARIO (connection pool starvation):
  Tomcat: 200 threads
  HikariCP: 10 connections

  All 200 threads try to get DB connection simultaneously.
  10 get connections.
  190 wait for a connection.
  Each waiting thread holds a Tomcat thread → no more threads for new requests.
  New requests queue up → timeout → 503 errors.

  RULE: HikariCP.maximumPoolSize should be close to Tomcat.threads.max
        (or the DB can't handle that many connections → find the bottleneck)
```

---

## @Transactional Isolation Pitfalls

```java
// NESTED @Transactional — common source of deadlocks
@Transactional
public void outerMethod() {
    lockRecord(id1);            // locks row id1
    innerService.innerMethod();  // if innerMethod locks id2 first... deadlock
}

@Transactional
public void innerMethod() {
    lockRecord(id2);            // locks row id2
    lockRecord(id1);            // tries to lock id1 — already held by outer!
    // DEADLOCK with any thread that has the locks in different order
}

// FIX: consistent lock ordering
// Always lock id1 before id2, regardless of call order.

// PROPAGATION.REQUIRES_NEW deadlock:
@Transactional(propagation = REQUIRES_NEW)
// This suspends the outer transaction, opens a new one.
// But: outer holds DB locks → new inner TX may wait for those locks → deadlock!
// Use REQUIRES_NEW only when outer TX must NOT see inner TX's changes.
```

---

## GC Pause Impact on Web Applications

```text
GC pause during HTTP request = request appears to hang:

  Request received    : t=0ms
  Processing begins   : t=1ms
  ← GC PAUSE (200ms) →
  Processing resumes  : t=201ms
  Response sent       : t=220ms

  From the client: 220ms latency. Appears to be "slow API".
  Reality: GC pause inflated it from 20ms to 220ms.

SYMPTOMS:
  - Periodic latency spikes (every N seconds → GC frequency)
  - /actuator/metrics/jvm.gc.pause showing p99 > 100ms
  - Timeouts in distributed calls (one service GCs → upstream times out)

SOLUTIONS:
  1. Tune heap size (-Xms/-Xmx) — larger heap = less frequent GC
  2. Switch to G1GC: -XX:+UseG1GC -XX:MaxGCPauseMillis=100
  3. Switch to ZGC (JDK 15+): pauses < 1ms
  4. Reduce object allocation (object pooling, avoiding temporary objects)
  5. Memory profiling: find what's creating short-lived objects
```

---

## Thread Starvation in @Scheduled

```java
// PROBLEM: @Scheduled uses single thread by default!
// If one task takes too long → ALL other tasks are blocked

@Component
public class Jobs {
    @Scheduled(fixedRate = 1000)
    public void fastJob() { /* 10ms */ }

    @Scheduled(fixedRate = 1000)
    public void slowJob() {
        Thread.sleep(5000);  // 5 seconds!
        // fastJob will not run during these 5 seconds
    }
}

// FIX: configure task scheduler with multiple threads
@Bean
public TaskScheduler taskScheduler() {
    ThreadPoolTaskScheduler scheduler = new ThreadPoolTaskScheduler();
    scheduler.setPoolSize(10);  // independent threads per job
    scheduler.setThreadNamePrefix("scheduled-");
    return scheduler;
}
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `singleton_race_condition.c` | Mutable Spring singleton race condition + fixes |
| `connection_pool_threads.c` | HikariCP starvation scenario |
| `deadlock_detection.c` | Deadlock patterns + prevention strategies |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o singleton_race_condition singleton_race_condition.c && ./singleton_race_condition
gcc -Wall -Wextra -pthread -o connection_pool_threads connection_pool_threads.c && ./connection_pool_threads
```

---

## Key Insight

```text
The BEST Spring concurrency strategy: make beans STATELESS.

Stateless bean = no mutable instance fields after construction.
  - No synchronization needed
  - No race conditions possible
  - Scales perfectly across threads

If you NEED state:
  - Per-request state → @Scope("request") or method parameters
  - Shared counters   → AtomicInteger / LongAdder
  - Shared cache      → ConcurrentHashMap
  - Shared list       → CopyOnWriteArrayList or ConcurrentLinkedQueue

Never:
  - ArrayList/HashMap as instance field in @Service
  - Non-volatile, non-atomic "flag" fields in @Service
  - Calling Thread.sleep() inside @Transactional (holds DB connection!)
```
