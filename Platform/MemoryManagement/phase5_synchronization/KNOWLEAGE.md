# 🔒 Phase 5: Synchronization Primitives

> **Java Concept**: `synchronized`, `wait()/notify()`, `Lock`, `Condition`, JVM lock escalation
> **C Equivalent**: `pthread_mutex_t`, `pthread_cond_t`, compare-and-swap

---

## Java Monitor — Every Object Is a Lock

```text
In Java, EVERY object has a built-in monitor.
A monitor = mutex + condition variable combined.

Java:  synchronized(obj) { ... }   ← acquire obj's monitor
C:     pthread_mutex_lock(&obj->mutex)

Java:  obj.wait()                  ← release lock + wait for notify
C:     pthread_cond_wait(&obj->cond, &obj->mutex)

Java:  obj.notify()                ← wake one waiting thread
C:     pthread_cond_signal(&obj->cond)

Java:  obj.notifyAll()             ← wake all waiting threads
C:     pthread_cond_broadcast(&obj->cond)
```

---

## JVM Lock Escalation (3 Levels)

```text
The JVM starts with cheap locks and escalates only when necessary.

Level 1 — BIASED LOCK (deprecated JDK 15+):
  Most objects are accessed by one thread 95% of the time.
  First thread that locks object → "biases" it toward that thread.
  Subsequent lock acquisitions by same thread: NO atomic operation.
  Cost: ~1 clock cycle. But: revocation is expensive when another thread needs it.

Level 2 — THIN LOCK (stack lock / lightweight lock):
  Used when 2+ threads contend, but not simultaneously.
  JVM does CAS on object header: sets pointer to a "lock record" on stack.
  No OS involvement. Cost: ~10 ns per lock/unlock.
  
  Mark Word during thin lock:
    [lock record pointer | 00]  ← last 2 bits = 00 = thin locked

Level 3 — FAT LOCK (inflated / heavyweight lock):
  Used when thread must BLOCK (another thread holds the lock).
  JVM inflates to a real OS mutex (futex on Linux).
  Cost: ~100-500 ns + OS context switch if blocking.
  
  Mark Word during fat lock:
    [monitor pointer | 10]      ← last 2 bits = 10 = fat locked
```

---

## synchronized vs ReentrantLock

```java
// synchronized: simple, built-in, cannot timeout
synchronized(lock) {
    // critical section
}

// ReentrantLock: flexible, can timeout, can be fair
Lock lock = new ReentrantLock(true);  // fair: FIFO ordering

if (lock.tryLock(500, TimeUnit.MILLISECONDS)) {  // timeout!
    try {
        // critical section
    } finally {
        lock.unlock();  // MUST be in finally
    }
}

// Multiple conditions:
Condition notEmpty = lock.newCondition();
Condition notFull  = lock.newCondition();
// Can signal specific condition (not possible with wait/notify)
notEmpty.await();      // like wait()
notEmpty.signal();     // like notify()
```

---

## Semaphore — Counting Permits

```java
Semaphore sem = new Semaphore(3);  // 3 permits = 3 concurrent users allowed

// Acquire permit (blocks if 0 available)
sem.acquire();
try {
    // use shared resource (e.g., DB connection, rate limiting)
} finally {
    sem.release();  // return permit
}

// Use cases:
//   Rate limiting: Semaphore(100) = max 100 concurrent requests
//   Connection pool: Semaphore(pool_size) = max pool_size active connections
//   Resource throttling
```

---

## ReadWriteLock — Multiple Readers, One Writer

```java
ReadWriteLock rwLock = new ReentrantReadWriteLock();

// Multiple threads can read concurrently:
rwLock.readLock().lock();
try { read(data); }
finally { rwLock.readLock().unlock(); }

// Only ONE thread can write (and no readers while writing):
rwLock.writeLock().lock();
try { write(data); }
finally { rwLock.writeLock().unlock(); }

// Use case: Spring cache (many reads, rare writes)
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `mutex_monitor.c` | Java monitor: mutex + condition variable (wait/notify) |
| `semaphore.c` | Semaphore with acquire/release + use cases |
| `lock_escalation.c` | JVM lock escalation: biased→thin→fat |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o mutex_monitor mutex_monitor.c && ./mutex_monitor
gcc -Wall -Wextra -pthread -o semaphore semaphore.c && ./semaphore
```

---

## Key Insight

```text
synchronized is SIMPLE but can be SLOW when heavily contended.
  Use it for short critical sections (microseconds).
  Don't call blocking operations inside synchronized blocks.

Lock/ReentrantLock gives you:
  tryLock(timeout)     → avoid deadlock
  lockInterruptibly()  → allow interruption while waiting
  fair=true            → prevent starvation (FIFO)
  Multiple conditions  → more fine-grained signaling

ReadWriteLock is optimal for read-heavy workloads:
  10 readers + 1 writer: synchronized blocks all; ReadWriteLock allows 10 concurrent reads.

Spring singleton beans are shared across ALL request threads.
If a Spring service has mutable state → MUST synchronize.
Best practice: make Spring beans STATELESS (no instance fields that change).
```
