# 🏊 Phase 7: Thread Pools

> **Java Concept**: `ThreadPoolExecutor`, `ForkJoinPool`, virtual threads (JDK 21)
> **C Equivalent**: A fixed array of worker threads + a task queue

---

## Why Thread Pools?

```text
WITHOUT thread pool (thread-per-task):
  new Thread(() -> handleRequest(req)).start();
  Cost per request:
    ~1ms to create OS thread
    ~1MB stack memory allocated
    ~1ms to destroy thread after request
  1000 concurrent requests = 1000 OS threads = 1 GB stack memory!

WITH thread pool (ThreadPoolExecutor):
  Thread created ONCE, reused for many tasks.
  Tasks submitted to queue → idle thread picks up task.
  Cost per request: queue put (ns) + task scheduling (µs)
```

---

## ThreadPoolExecutor Parameters

```java
ThreadPoolExecutor executor = new ThreadPoolExecutor(
    corePoolSize,     // min threads always alive (even if idle)
    maximumPoolSize,  // max threads when queue is full
    keepAliveTime,    // how long extra threads stay alive when idle
    TimeUnit.SECONDS,
    new LinkedBlockingQueue<>(queueCapacity),   // task queue
    new ThreadFactory() { ... },                 // how to create threads
    new RejectedExecutionHandler() { ... }       // what to do when full
);
```

---

## Task Submission Flow

```text
submit(task):
  if (active_threads < corePoolSize):
    create new thread → run task immediately
  else if (queue not full):
    put task in queue → idle thread will pick it up
  else if (active_threads < maximumPoolSize):
    create extra thread → run task immediately
  else:
    RejectionHandler.reject(task)  ← queue AND max threads both full
```

---

## Rejection Policies

```java
// AbortPolicy (default): throw RejectedExecutionException
// CallerRunsPolicy: run the task in the CALLING thread (blocks caller, slows producers)
// DiscardPolicy: silently discard the task (dangerous — silent data loss)
// DiscardOldestPolicy: remove oldest queued task, retry submission

// Custom policy (common in production):
new RejectedExecutionHandler() {
    public void rejectedExecution(Runnable r, ThreadPoolExecutor executor) {
        metrics.increment("tasks.rejected");     // alert!
        log.error("Task rejected: pool full");
        // try to queue with timeout, or throw with context
    }
}
```

---

## Spring's @Async Thread Pool

```java
@Configuration
@EnableAsync
public class AsyncConfig implements AsyncConfigurer {
    @Override
    public Executor getAsyncExecutor() {
        ThreadPoolTaskExecutor executor = new ThreadPoolTaskExecutor();
        executor.setCorePoolSize(10);
        executor.setMaxPoolSize(50);
        executor.setQueueCapacity(200);
        executor.setThreadNamePrefix("async-");
        executor.setRejectedExecutionHandler(new CallerRunsPolicy());
        executor.initialize();
        return executor;
    }
}

// Usage:
@Service class UserService {
    @Async
    CompletableFuture<User> findByIdAsync(Long id) { ... }
}
```

---

## Virtual Threads (JDK 21)

```java
// Before JDK 21: platform thread = OS thread (expensive)
Thread.ofPlatform().start(runnable);

// JDK 21: virtual thread (cheap, many per OS thread)
Thread.ofVirtual().start(runnable);

// Virtual thread pool (executor):
ExecutorService executor = Executors.newVirtualThreadPerTaskExecutor();
// Creates ONE virtual thread per task — cheap enough to not pool!

// Spring Boot 3.2:
// application.properties:
// spring.threads.virtual.enabled=true
// → Tomcat request threads become virtual threads
// → @Async uses virtual threads
// → HikariCP may need pool-size tuning
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `executor_service.c` | ThreadPoolExecutor: worker pool + task queue |
| `virtual_threads.c` | Virtual thread cooperative scheduling model |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o executor_service executor_service.c && ./executor_service
```

---

## Key Insight

```text
Thread pool sizing rules:
  I/O-bound tasks (DB, HTTP):  corePoolSize = 2 × CPU cores
  CPU-bound tasks (compute):   corePoolSize = CPU cores

Spring Boot Tomcat default:   server.tomcat.threads.max=200
HikariCP default:             maximumPoolSize=10

If Tomcat has 200 threads but HikariCP only 10 connections:
  → 190 Tomcat threads will wait for a DB connection
  → pool is undersized for the workload
  Rule: maximize-pool-size ≈ Tomcat threads / average DB wait fraction

Virtual threads (JDK 21) change the equation:
  → Can have 10,000 virtual threads all waiting on DB
  → But HikariCP still limits to N concurrent connections
  → Semaphore before DB operations limits concurrency
```
