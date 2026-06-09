# ⚡ Phase 3: Java Memory Model (JMM)

> **Java Concept**: volatile, happens-before, memory visibility, instruction reordering
> **C Equivalent**: Memory barriers, cache flush instructions, `__sync_synchronize()`

---

## The Problem: CPU Caches

```text
Modern CPU architecture (L1/L2/L3 cache hierarchy):

  Core 0          Core 1
  ┌───────┐       ┌───────┐
  │  L1   │       │  L1   │   ← each core has its own L1 cache
  │ cache │       │ cache │     reads/writes go here FIRST
  └───┬───┘       └───┬───┘
      │               │
  ┌───┴───┐       ┌───┴───┐
  │  L2   │       │  L2   │
  └───┬───┘       └───┴───┘
      │               │
      └───────┬────────┘
          ┌───┴───┐
          │  L3   │  ← shared last-level cache
          └───┬───┘
              │
          ┌───┴───┐
          │  RAM  │  ← main memory (slow)
          └───────┘

Problem:
  Thread 0 writes x = 1 → goes to Core 0's L1 cache
  Thread 1 reads  x     → reads from Core 1's L1 cache → sees x = 0 !!!

  The write to main memory happens LATER (cache eviction).
  Thread 1 sees a STALE value from its own cache.
  This is the MEMORY VISIBILITY PROBLEM.
```

---

## The Solution: volatile + Memory Barriers

```java
// BAD — race condition on visibility:
class Flag {
    boolean ready = false;          // not volatile!
    Object data;

    void produce() {
        data  = createData();      // step 1
        ready = true;              // step 2: CPU may reorder this BEFORE step 1!
    }

    void consume() {               // on another thread
        while (!ready) {}          // busy-wait
        use(data);                 // data may be null! (visibility issue)
    }
}

// GOOD — volatile guarantees visibility:
class Flag {
    volatile boolean ready = false; // volatile!
    Object data;

    void produce() {
        data  = createData();      // step 1 (always before step 2 — memory barrier)
        ready = true;              // volatile write → memory barrier → flushes to RAM
    }

    void consume() {
        while (!ready) {}          // volatile read → always reads from RAM
        use(data);                 // data is definitely initialized
    }
}
```

---

## What volatile Does Under the Hood

```text
volatile write:
  1. Write value to L1 cache
  2. Issue STORE FENCE (SFENCE on x86, dmb st on ARM)
  3. Flush write buffers → value propagates to L3/RAM
  4. Invalidate same cache line in ALL other cores

volatile read:
  1. Issue LOAD FENCE (LFENCE on x86)
  2. Re-fetch from main memory (bypasses stale L1 cache)
  3. Return fresh value

Cost: ~10-50 ns per volatile access (vs ~1 ns for regular field access)
When to use: only for flags/state vars; NOT for compound operations
```

---

## Happens-Before Rules

The JMM guarantees "happens-before" (HB) in these situations:

```text
1. Program order:
   action A before action B in same thread → A HB B

2. Monitor lock:
   unlock(m) HB lock(m) on same monitor
   → writes before unlock are visible after lock

3. volatile:
   write to volatile v HB read of same volatile v
   → writes before volatile write are visible after volatile read

4. Thread start:
   Thread.start() HB all actions in the started thread

5. Thread join:
   All actions in T HB Thread.join(T) returns

6. Transitivity:
   if A HB B and B HB C then A HB C
```

---

## Common Visibility Bugs

```java
// BUG 1: Non-volatile singleton (double-checked locking)
class Singleton {
    private static Singleton instance;  // NOT volatile!

    public static Singleton getInstance() {
        if (instance == null) {             // first check
            synchronized (Singleton.class) {
                if (instance == null) {     // second check
                    instance = new Singleton();  // BROKEN: partial construction visible
                }
            }
        }
        return instance;  // may return PARTIALLY CONSTRUCTED object!
    }
}

// FIX: make instance volatile
private static volatile Singleton instance;
// volatile ensures: construction completes before reference is visible

// BUG 2: Loop exit condition not visible
class Worker {
    boolean running = true;  // NOT volatile!
    void run() {
        while (running) { /* work */ }  // thread may NEVER see running=false
    }
    void stop() { running = false; }    // write may stay in another core's cache
}
// FIX: volatile boolean running = true;
```

---

## volatile vs synchronized vs Atomic

```text
volatile:
  + Visibility guarantee (always reads latest value)
  + Ordering guarantee (no reordering around volatile access)
  - NOT atomic for compound operations (read-modify-write)
  - Wrong for: counter++, if(x==0) x=1

synchronized:
  + Full mutual exclusion (only one thread at a time)
  + Visibility + ordering
  - Slow (lock acquisition, thread blocking)

Atomic (java.util.concurrent.atomic):
  + Lock-free atomic operations using CAS (Compare-And-Swap)
  + AtomicInteger.incrementAndGet() → thread-safe counter
  + No blocking, no context switching
  - Limited to single variable operations
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `volatile_visibility.c` | Cache coherence problem + memory barrier solution |
| `happens_before.c` | Happens-before rule demonstrations |
| `instruction_reorder.c` | CPU/compiler reordering dangers |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o volatile_visibility volatile_visibility.c -lpthread && ./volatile_visibility
```

---

## Key Insight

```text
"volatile" is NOT about performance — it's about CORRECTNESS.
Without volatile, your multithreaded code may work on YOUR machine
(x86 has strong memory ordering) but break on ARM or PowerPC
(weaker memory models).

The JMM exists to define consistent semantics across ALL hardware.
volatile guarantees the JMM minimum: visibility + no reordering.
synchronized guarantees the JMM maximum: exclusive access + visibility.
```
