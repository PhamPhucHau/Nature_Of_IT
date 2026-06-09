# ⚛️ Phase 6: Java Concurrency Utilities

> **Java Concept**: `synchronized`, `ReentrantLock`, `AtomicInteger`, `CountDownLatch`, `CyclicBarrier`
> **C Equivalent**: mutex, CAS operations, semaphores, barrier

---

## Atomic Operations — Lock-Free Thread Safety

```java
// Thread-safe counter WITHOUT locks:
AtomicInteger count = new AtomicInteger(0);
count.incrementAndGet();  // atomic: LOCK XADD on x86

// Thread-safe reference update WITHOUT locks:
AtomicReference<Node> head = new AtomicReference<>(null);

// CAS (Compare-And-Swap) — the fundamental operation:
//   if (current == expected) { current = update; return true; }
//   else { return false; }
// On x86: LOCK CMPXCHG instruction
head.compareAndSet(expectedNode, newNode);

// AtomicLong for high-precision counters (metrics, IDs)
AtomicLong requestCount = new AtomicLong(0);
long id = requestCount.getAndIncrement();  // thread-safe ID generation

// LongAdder — better than AtomicLong for HIGH contention:
// Uses per-thread cells, sums them on get()
// AtomicLong: 1 shared counter → heavy contention
// LongAdder:  N thread-local cells → no contention → faster for counters
LongAdder hitCount = new LongAdder();
hitCount.increment();
long total = hitCount.sum();
```

---

## CountDownLatch — Wait for N Events

```java
CountDownLatch latch = new CountDownLatch(3);  // 3 events needed

// 3 threads each do work and count down:
executor.submit(() -> { doWork(); latch.countDown(); });
executor.submit(() -> { doWork(); latch.countDown(); });
executor.submit(() -> { doWork(); latch.countDown(); });

// Main thread waits until all 3 done:
latch.await();              // blocks until count reaches 0
latch.await(30, SECONDS);   // with timeout

// Use cases:
//   - Wait for all microservices to start before sending traffic
//   - Wait for N threads to finish initialization
//   - Integration test: wait for async operations to complete
```

---

## CyclicBarrier — Synchronize N Threads at a Point

```java
CyclicBarrier barrier = new CyclicBarrier(3, () -> {
    System.out.println("All 3 threads reached barrier!");  // run once all arrive
});

// Each thread does phase 1, then waits at barrier:
Runnable task = () -> {
    doPhase1();
    barrier.await();   // WAIT until all 3 arrive
    doPhase2();        // all 3 start phase 2 simultaneously
};

// Unlike CountDownLatch: CyclicBarrier resets → can be reused
// Use cases: parallel algorithms (each phase must sync), simulation steps
```

---

## StampedLock — Optimistic Reads

```java
StampedLock lock = new StampedLock();

// Optimistic read (no actual lock taken):
long stamp = lock.tryOptimisticRead();
int x = field.x;
int y = field.y;
if (!lock.validate(stamp)) {     // was it modified while reading?
    // retry with real read lock
    stamp = lock.readLock();
    try { x = field.x; y = field.y; }
    finally { lock.unlockRead(stamp); }
}
// Best performance for read-heavy workloads (cache, configuration)
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `atomic_operations.c` | CAS, AtomicInteger, LongAdder pattern |
| `reentrant_lock.c` | ReentrantLock + Condition variables |
| `synchronized_block.c` | synchronized internals with lock escalation |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o atomic_operations atomic_operations.c && ./atomic_operations
gcc -Wall -Wextra -pthread -o reentrant_lock reentrant_lock.c && ./reentrant_lock
```

---

## Key Insight

```text
Rule of thumb for concurrency primitives:

  Simple counter/flag?     → AtomicInteger / AtomicBoolean
  Short critical section?  → synchronized
  Need tryLock/timeout?    → ReentrantLock
  Read-heavy shared state? → ReadWriteLock / StampedLock
  High-throughput counter? → LongAdder
  N threads to coordinate? → CountDownLatch / CyclicBarrier
  Limit concurrent access? → Semaphore

Spring singleton beans share state across ALL request threads.
If a Spring @Service has a counter or cache field → use Atomic or ConcurrentHashMap.
The #1 Spring concurrency bug: mutable instance field in a singleton bean.
```
