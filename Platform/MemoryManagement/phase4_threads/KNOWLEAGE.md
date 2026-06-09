# 🧵 Phase 4: Thread Fundamentals

> **Java Concept**: `Thread`, `Runnable`, thread lifecycle, `ThreadLocal`, virtual threads
> **C Equivalent**: `pthread_create`, `pthread_key_t` (thread-local storage)

---

## OS Thread vs JVM Thread vs Virtual Thread

```text
PLATFORM THREAD (Java 1.0 – present):
  Java Thread = one OS kernel thread (1:1 mapping)
  Creating 1 Java Thread = creating 1 OS thread (~1-2MB stack by default)
  Blocking I/O = OS thread is blocked = CPU wasted on context switching
  Max practical threads per JVM: ~1000-5000 (memory limited)

VIRTUAL THREAD (JDK 21, Project Loom):
  Many virtual threads share a small pool of carrier (OS) threads
  Virtual thread = ~1KB initial stack, on heap (not OS stack)
  Blocking I/O: virtual thread "parks" (unmounts from OS thread)
  OS thread continues running other virtual threads
  Max practical virtual threads: millions (memory = heap, not stack)

  Spring Boot 3.2+: spring.threads.virtual.enabled=true
    → @Async uses virtual threads
    → Tomcat uses virtual threads per request
```

---

## Java Thread Lifecycle

```text
NEW
  Thread t = new Thread(runnable);
  OS thread NOT yet created.
    ↓ t.start()
RUNNABLE
  OS thread created and submitted to OS scheduler.
  May be RUNNING (on CPU) or READY (waiting for CPU).
    ↓ Thread.sleep() / wait() / blocking I/O
TIMED_WAITING        ← Thread.sleep(n), wait(n), LockSupport.parkNanos()
WAITING              ← wait(), join(), LockSupport.park()
BLOCKED              ← waiting to acquire synchronized lock
    ↓ sleep expires / notify() / lock acquired
RUNNABLE (again)
    ↓ run() method returns or throws exception
TERMINATED
  Thread is dead. Cannot be restarted.
```

---

## ThreadLocal — Per-Thread State

```java
// ThreadLocal: each thread has its own independent copy
ThreadLocal<User> currentUser = new ThreadLocal<>();

// Spring uses this everywhere:
//   SecurityContextHolder.getContext()  ← ThreadLocal<SecurityContext>
//   TransactionSynchronizationManager  ← ThreadLocal<Map<DataSource, Connection>>
//   RequestContextHolder               ← ThreadLocal<RequestAttributes>

// Usage:
currentUser.set(authenticatedUser);  // set in request-processing thread

// anywhere in the same thread's call stack:
User user = currentUser.get();       // always returns THIS thread's user

// MUST clean up after use:
currentUser.remove();                // prevent memory leaks!
```

---

## Thread Safety Levels

```java
// Level 1: Immutable (safest — no state to protect)
final class ImmutablePoint { final int x, y; }

// Level 2: Thread-safe (explicit synchronization)
class SafeCounter { private int count; public synchronized void increment() { count++; } }

// Level 3: Conditionally thread-safe (document which methods need external sync)
class ArrayList { /* not thread-safe for writes, ok for reads with no writes */ }

// Level 4: Not thread-safe (caller must synchronize)
class HashMap { /* must use ConcurrentHashMap or synchronize externally */ }
```

---

## Thread Interruption

```java
Thread worker = new Thread(() -> {
    while (!Thread.currentThread().isInterrupted()) {
        doWork();
    }
});
worker.start();

// From another thread:
worker.interrupt();   // sets interrupted flag + wakes from sleep/wait
// Thread.sleep() throws InterruptedException if interrupted
// Thread must check isInterrupted() in loops
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `thread_lifecycle.c` | Full thread state machine: NEW→RUNNABLE→WAITING→TERMINATED |
| `thread_local_storage.c` | ThreadLocal<T> simulation with pthread_key_t |
| `os_vs_jvm_threads.c` | Platform thread vs virtual thread model comparison |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o thread_lifecycle thread_lifecycle.c && ./thread_lifecycle
gcc -Wall -Wextra -pthread -o thread_local_storage thread_local_storage.c && ./thread_local_storage
```

---

## Key Insight

```text
Every Java Thread is an OS thread (pre-JDK 21).
Creating threads is EXPENSIVE: ~1ms + ~1MB stack memory.
Thread pools reuse threads → amortize creation cost.

Virtual threads (JDK 21) change the model:
  Blocking is cheap because OS thread is not blocked.
  You can have 1 million "concurrent" requests.
  Spring WebFlux (reactive) was the old workaround — virtual threads replace it.

Spring's @Transactional relies on ThreadLocal to bind connections.
Virtual threads MUST handle ThreadLocal correctly for this to work.
Spring Boot 3.2 handles this correctly.
```
