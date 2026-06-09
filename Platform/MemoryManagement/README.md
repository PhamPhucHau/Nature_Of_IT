# 🧠 Memory Management, Multithreading & Concurrency

> **Goal**: Understand memory and concurrency at the machine level using C,
> then map every concept directly to Java, Spring, and Spring Boot.
>
> If you can manage memory in C, you OWN concurrency in Java.

---

## 🧠 The Core Philosophy

```text
C shows you the MACHINE TRUTH:
    malloc() / free() / pointer arithmetic / cache lines / mutex

Java HIDES it but does the SAME thing:
    new Object() = malloc under the hood
    GC = automatic free() with reachability analysis
    synchronized = OS mutex + JVM bias/thin/fat lock escalation
    volatile = memory barrier + cache coherence protocol
    ThreadPoolExecutor = OS threads + task queue

Spring/Spring Boot ABSTRACTS it further:
    @Transactional = begin/commit/rollback + connection binding per thread
    @Async = submit to ThreadPoolExecutor
    @Cacheable = HashMap + lock per cache region
    Singleton scope = one instance, shared across all threads — RACE CONDITION RISK
```

---

## 🗺️ Learning Path

```text
Phase 1:  JVM Memory Model (Heap, Stack, Metaspace)
Phase 2:  Garbage Collection Algorithms (Mark-Sweep, G1, ZGC)
Phase 3:  Java Memory Model (volatile, happens-before, reordering)
Phase 4:  Thread Fundamentals (OS threads, JVM threads, virtual threads)
Phase 5:  Synchronization Primitives (mutex, semaphore, monitor, lock)
Phase 6:  Java Concurrency Utilities (synchronized, Lock, Atomic)
Phase 7:  Thread Pools (ExecutorService, ForkJoinPool, virtual threads)
Phase 8:  Concurrent Collections (ConcurrentHashMap, BlockingQueue)
Phase 9:  Spring Concurrency (@Async, @Scheduled, request threads)
Phase 10: Spring Boot Production Patterns (thread pools, tuning, race conditions)
```

---

## 📁 Folder Structure

```text
MemoryManagement/
├── README.md
├── OVERVIEW.md
│
├── phase1_jvm_memory/
│   ├── KNOWLEAGE.md
│   ├── heap_stack_model.c         ← Stack vs Heap layout + JVM mapping
│   └── metaspace_classdata.c      ← Class data in memory (Metaspace)
│
├── phase2_garbage_collection/
│   ├── KNOWLEAGE.md
│   ├── mark_sweep.c               ← Mark-Sweep-Compact algorithm
│   ├── generational_gc.c          ← Young/Old generation model (G1)
│   └── gc_roots.c                 ← GC root scanning simulation
│
├── phase3_java_memory_model/
│   ├── KNOWLEAGE.md
│   ├── volatile_visibility.c      ← Cache coherence + memory barriers
│   ├── happens_before.c           ← Happens-before relationship rules
│   └── instruction_reorder.c     ← CPU/compiler reordering dangers
│
├── phase4_threads/
│   ├── KNOWLEAGE.md
│   ├── thread_lifecycle.c         ← NEW→RUNNABLE→BLOCKED→WAITING→DEAD
│   ├── os_vs_jvm_threads.c        ← OS thread vs Java Thread vs Virtual Thread
│   └── thread_local_storage.c    ← ThreadLocal<T> in C
│
├── phase5_synchronization/
│   ├── KNOWLEAGE.md
│   ├── mutex_monitor.c            ← mutex + condition variable (Java monitor)
│   ├── semaphore.c                ← counting semaphore
│   └── lock_escalation.c         ← biased → thin → fat lock (JVM internals)
│
├── phase6_java_concurrency/
│   ├── KNOWLEAGE.md
│   ├── synchronized_block.c       ← synchronized {} internals
│   ├── reentrant_lock.c           ← ReentrantLock + tryLock + fairness
│   └── atomic_operations.c       ← CAS (Compare-And-Swap) atomic ops
│
├── phase7_thread_pools/
│   ├── KNOWLEAGE.md
│   ├── executor_service.c         ← ThreadPoolExecutor internals
│   ├── fork_join_pool.c           ← work-stealing ForkJoinPool
│   └── virtual_threads.c         ← JDK 21 virtual thread model
│
├── phase8_concurrent_collections/
│   ├── KNOWLEAGE.md
│   ├── concurrent_hashmap.c       ← ConcurrentHashMap segment locking
│   ├── blocking_queue.c           ← LinkedBlockingQueue + producer-consumer
│   └── copy_on_write_list.c      ← CopyOnWriteArrayList
│
├── phase9_spring_concurrency/
│   ├── KNOWLEAGE.md
│   ├── async_execution.c          ← @Async + TaskExecutor internals
│   ├── scheduled_tasks.c          ← @Scheduled + ScheduledThreadPoolExecutor
│   └── request_thread_model.c    ← Tomcat thread per request model
│
└── phase10_production_patterns/
    ├── KNOWLEAGE.md
    ├── singleton_race_condition.c ← Spring singleton + thread safety
    ├── connection_pool_threads.c  ← HikariCP thread contention
    └── deadlock_detection.c      ← Deadlock patterns + prevention
```

---

## 🔥 How to Study Each Phase

```text
1. Read KNOWLEAGE.md  → Java/Spring concept + "why it matters"
2. Read .c file       → machine-level implementation with full comments
3. Build:  gcc -Wall -Wextra -pthread -o output file.c && ./output
4. Modify: introduce a race condition, then fix it
5. Map back: "Now I know WHY synchronized exists"
```

---

## 🎯 Final Goal

```text
After all phases:
  ✓ Explain exactly what synchronized does at the CPU level
  ✓ Diagnose memory leaks and GC pauses in production
  ✓ Design thread-safe Spring beans from scratch
  ✓ Tune Tomcat/HikariCP thread pools for high throughput
  ✓ Avoid the 5 most common Java concurrency bugs
  ✓ Understand why Virtual Threads (JDK 21) change everything
```
