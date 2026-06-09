# 🌱 Phase 9: Spring Concurrency

> **Spring Concept**: `@Async`, `@Scheduled`, request thread model, `TransactionSynchronizationManager`
> **C Equivalent**: Task queues + ThreadLocal storage + signal-based scheduling

---

## Tomcat Request Thread Model

```text
HTTP Request arrives at TCP socket
  ↓
Tomcat Connector (NIO thread, non-blocking accept)
  ↓
Task submitted to Tomcat thread pool
  ↓
Worker thread picks up request
  ↓ (entire processing happens on this ONE thread)
  │
  ├── DispatcherServlet.doDispatch()
  │     ↓
  ├── SecurityFilter (reads SecurityContextHolder ThreadLocal)
  │     ↓
  ├── @Controller method executes
  │     ↓
  ├── @Service method executes
  │     ↓
  ├── @Repository → JDBC (connection bound to THIS thread via ThreadLocal)
  │     ↓
  └── Response written
  ↓
Thread RETURNS to Tomcat pool (NOT destroyed)
→ SecurityContext CLEARED  (Spring calls SecurityContextHolder.clearContext())
→ Transaction context CLEARED
→ Request attributes CLEARED
```

---

## @Async — Asynchronous Execution

```java
@Service
public class EmailService {

    @Async("emailExecutor")   // run on "emailExecutor" thread pool
    public CompletableFuture<Void> sendWelcomeEmail(String email) {
        // This method runs on a DIFFERENT thread (not the request thread)
        emailClient.send(email, "Welcome!");
        return CompletableFuture.completedFuture(null);
    }
}

// Config:
@Configuration
@EnableAsync
public class AsyncConfig {
    @Bean("emailExecutor")
    public Executor emailExecutor() {
        ThreadPoolTaskExecutor e = new ThreadPoolTaskExecutor();
        e.setCorePoolSize(5);
        e.setMaxPoolSize(10);
        e.setQueueCapacity(100);
        e.setThreadNamePrefix("email-");
        return e;
    }
}

// Gotcha: @Async doesn't propagate SecurityContext or TransactionContext by default!
// SecurityContextHolder uses ThreadLocal → different thread → empty context
// Fix: SecurityContextHolder.setStrategyName(MODE_INHERITABLETHREADLOCAL)
//      or manually copy context to the @Async thread
```

---

## @Scheduled — Periodic Tasks

```java
@Component
public class CleanupJob {

    @Scheduled(cron = "0 0 2 * * *")          // every day at 2AM
    public void cleanupExpiredSessions() { }

    @Scheduled(fixedRate = 60000)              // every 60 seconds
    public void refreshCache() { }

    @Scheduled(fixedDelay = 5000)             // 5 seconds AFTER last run completes
    public void pollMessageQueue() { }

    @Scheduled(initialDelay = 30000, fixedRate = 60000)
    public void warmupTask() { }
}

// Config:
@Configuration
@EnableScheduling
public class SchedulingConfig implements SchedulingConfigurer {
    @Override
    public void configureTasks(ScheduledTaskRegistrar registrar) {
        // Default: single-threaded! Tasks queue up if one is running long
        // Fix: provide a TaskScheduler with a thread pool
        registrar.setTaskScheduler(new ThreadPoolTaskScheduler() {{
            setPoolSize(5);
            setThreadNamePrefix("scheduled-");
            initialize();
        }});
    }
}
```

---

## TransactionSynchronizationManager — Spring's ThreadLocal Store

```java
// Spring's internal: binds ONE connection per thread per DataSource
// This is how @Transactional shares the SAME connection across all repo calls

// Simplified Spring internals:
class TransactionSynchronizationManager {
    // ThreadLocal<Map<DataSource, ConnectionHolder>>
    private static final ThreadLocal<Map<Object,Object>> resources = new ThreadLocal<>();

    public static void bindResource(Object key, Object value) {
        Map<Object,Object> map = resources.get();
        if (map == null) { map = new HashMap<>(); resources.set(map); }
        map.put(key, value);
    }

    public static Object getResource(Object key) {
        Map<Object,Object> map = resources.get();
        return map != null ? map.get(key) : null;
    }
}

// When @Transactional starts:
//   1. DataSourceTransactionManager.doBegin()
//   2. connection = dataSource.getConnection()   ← borrow from pool
//   3. connection.setAutoCommit(false)           ← begin transaction
//   4. TransactionSynchronizationManager.bindResource(dataSource, connection)
//
// Each @Repository call:
//   1. DataSourceUtils.getConnection(dataSource)
//   2. → TransactionSynchronizationManager.getResource(dataSource)
//   3. → returns THE SAME connection (already bound!)
//
// @Transactional ends:
//   1. commit() or rollback()
//   2. TransactionSynchronizationManager.unbindResource(dataSource)
//   3. connection.close() → returned to pool
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `async_execution.c` | @Async submission to thread pool + context propagation |
| `request_thread_model.c` | Full Tomcat request thread lifecycle with ThreadLocals |
| `scheduled_tasks.c` | @Scheduled cron/fixed-rate scheduling |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o async_execution async_execution.c && ./async_execution
gcc -Wall -Wextra -pthread -o request_thread_model request_thread_model.c && ./request_thread_model
```

---

## Key Insight

```text
Spring's @Transactional REQUIRES thread affinity:
  The same thread must call begin(), execute SQL, commit().
  This is enforced by ThreadLocal connection binding.

This is WHY Spring WebFlux (reactive) requires a different transaction model:
  Reactive: request processing jumps between threads (non-blocking)
  → ThreadLocal-based @Transactional doesn't work
  → Uses R2DBC + Project Reactor's context (reactor.util.context.Context)

Virtual threads (JDK 21) + Spring Boot 3.2:
  Virtual threads ARE compatible with ThreadLocal.
  @Transactional works without changes.
  Virtual thread parks (unmounts) when waiting for DB — OS thread not blocked.
```
