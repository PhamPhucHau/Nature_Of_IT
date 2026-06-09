# 🌍 Memory & Concurrency — The Big Picture

---

## How Memory Works: C → Java → Spring

```text
LEVEL 1 — Operating System (physical truth):
  RAM is a flat array of bytes.
  OS gives each process a virtual address space.
  Virtual address → physical address via MMU + page tables.

LEVEL 2 — Process Memory Layout (C visible):
  ┌──────────────────────────────────────┐  High address
  │  Stack (grows ↓)                    │  local vars, function frames
  │  ...                                │
  │  ...                                │
  │  Heap (grows ↑)                     │  malloc()/free()
  ├──────────────────────────────────────┤
  │  BSS segment (uninitialized globals)│
  │  Data segment (initialized globals) │
  │  Text segment (code, read-only)     │
  └──────────────────────────────────────┘  Low address

LEVEL 3 — JVM Memory Layout (Java visible):
  ┌──────────────────────────────────────┐
  │  JVM Heap                           │  new Object() allocates here
  │    Young Generation                 │
  │      Eden Space      ← new objects  │
  │      Survivor S0                    │
  │      Survivor S1                    │
  │    Old Generation (Tenured)         │  long-lived objects
  ├──────────────────────────────────────┤
  │  Metaspace (native memory)          │  class data, method bytecode
  │  Code Cache (native memory)         │  JIT-compiled machine code
  ├──────────────────────────────────────┤
  │  JVM Stack (per thread)             │  method frames, local vars
  │  Native Stack (per thread)          │  JNI calls
  └──────────────────────────────────────┘

LEVEL 4 — Spring Memory Behavior:
  ApplicationContext = lives on JVM Heap
  Singleton beans    = one object on Heap, shared by ALL threads → MUST BE THREAD-SAFE
  Request-scoped     = new object per HTTP request thread
  ThreadLocal values = TransactionSynchronizationManager, SecurityContextHolder
```

---

## The Concurrency Stack: OS → Java → Spring

```text
OS Level:
  pthread_create()  → OS kernel thread (1:1 with Java thread, pre JDK21)
  pthread_mutex_t   → OS mutex (used by JVM fat locks)
  futex (Linux)     → fast userspace mutex (used by JVM thin locks)

Java Level:
  new Thread()      → wraps OS thread
  synchronized      → JVM monitor (object header mark word)
  volatile          → memory barrier instruction (MFENCE/LFENCE on x86)
  java.util.concurrent.*  → high-level abstractions over OS primitives

Spring Level:
  @Async            → submits Runnable to ThreadPoolTaskExecutor
  @Scheduled        → ScheduledThreadPoolExecutor with cron/fixed-rate
  @Transactional    → binds Connection to current Thread via ThreadLocal
  Spring singleton  → shared state across request threads → synchronize if mutable
```

---

## Memory Problems → Java Solutions → Spring Patterns

| C Problem | Java Solution | Spring Pattern |
|-----------|--------------|----------------|
| Memory leak | GC + reference tracking | N/A (GC handles) |
| Stack overflow | JVM stack limit + StackOverflowError | Virtual threads (larger stack) |
| Data race | synchronized / volatile / Atomic | @Synchronized, ConcurrentHashMap |
| Deadlock | ReentrantLock + tryLock timeout | @Transactional isolation levels |
| Memory visibility | volatile + happens-before | @Async result propagation |
| False sharing | cache line padding | Contended annotation |
| Thread starvation | ThreadPoolExecutor sizing | server.tomcat.threads.max |
| Context switching | Virtual threads (JDK 21) | Spring WebFlux (reactive) |

---

## The Five Critical Java Concurrency Bugs

```text
1. RACE CONDITION
   Two threads read-modify-write same variable without synchronization.
   Fix: synchronized block, AtomicInteger, or immutable design.
   Spring: Spring singleton beans with mutable fields.

2. DEADLOCK
   Thread A holds lock X, wants Y.
   Thread B holds lock Y, wants X.
   Both wait forever.
   Fix: consistent lock ordering, tryLock with timeout.
   Spring: @Transactional nested calls with different isolation.

3. LIVELOCK
   Threads keep yielding to each other, no progress.
   Fix: randomized backoff strategy.

4. MEMORY VISIBILITY
   Thread A writes a value. Thread B never sees it.
   Reason: CPU caches — B reads from L1 cache, not main memory.
   Fix: volatile keyword → forces write to main memory + memory fence.
   Spring: happens-before guaranteed by synchronized/volatile.

5. THREAD STARVATION
   High-priority threads monopolize CPU.
   Low-priority threads never run.
   Fix: fair locks (new ReentrantLock(true)), proper thread pool sizing.
   Spring: properly sized TaskExecutor, separate pools per task type.
```

---

## JVM Object Header — The Key to synchronized

```text
Every Java object has a 16-byte header (64-bit JVM):

  ┌─────────────────────────────────────────────┐
  │  Mark Word (8 bytes)                        │
  │  ┌──────────────────────────────────────┐   │
  │  │  State    │ Contents                 │   │
  │  ├───────────┼──────────────────────────┤   │
  │  │ Unlocked  │ identity hashcode (25b)  │   │
  │  │           │ age (4b) | bias(1b)| 01  │   │
  │  ├───────────┼──────────────────────────┤   │
  │  │ Biased    │ thread ID (54b) | epoch  │   │  ← JDK 15: deprecated
  │  │           │ age (4b) | 1 | 01        │   │
  │  ├───────────┼──────────────────────────┤   │
  │  │ Thin Lock │ lock record pointer(62b) │   │  ← CAS on stack
  │  │ (Inflated)│ | 00                     │   │
  │  ├───────────┼──────────────────────────┤   │
  │  │ Fat Lock  │ monitor pointer (62b)    │   │  ← OS mutex
  │  │ (Monitor) │ | 10                     │   │
  │  └───────────┴──────────────────────────┘   │
  │  Klass pointer (4 bytes, compressed)        │  → class metadata
  │  [Array length (4 bytes, if array)]         │
  └─────────────────────────────────────────────┘

  synchronized(obj) {  ← JVM inspects mark word
    try to CAS: unlocked → thin lock (fast, no OS call)
    if contended: inflate to fat lock (OS mutex, expensive)
  }
```

---

## Virtual Threads (JDK 21) — Why They Change Everything

```text
BEFORE JDK 21 (platform threads):
  Java thread = OS thread (expensive, ~1MB stack each)
  Server: 200 Tomcat threads = 200MB stack memory minimum
  Blocking I/O (DB, HTTP) = OS thread BLOCKED = CPU wasted

WITH JDK 21 virtual threads:
  Virtual thread = JVM-managed, ~1KB initial stack
  Blocking I/O: virtual thread UNMOUNTS from OS thread (carrier thread)
  OS thread continues running other virtual threads
  Result: 1 million virtual threads on 8 OS threads

  Spring Boot 3.2+:
    spring.threads.virtual.enabled=true
    Tomcat serves each request on a virtual thread
    @Async uses virtual thread pool by default
```

---

## How to Read This Folder

```text
Phase 1–3: Memory (JVM model, GC, Java Memory Model)
Phase 4–6: Thread fundamentals and synchronization
Phase 7–8: High-level concurrency utilities
Phase 9–10: Spring-specific concurrency patterns

Each phase: KNOWLEAGE.md (concept) + .c files (implementation)
The .c files build understanding from the bottom up.
```
