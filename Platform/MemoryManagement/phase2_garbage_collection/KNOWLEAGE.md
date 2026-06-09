# ♻️ Phase 2: Garbage Collection Algorithms

> **Java Concept**: Mark-Sweep, G1GC, ZGC, GC tuning
> **C Equivalent**: Manually walking a pointer graph to find unreachable nodes

---

## Why GC Exists

```text
C:    YOU decide when memory is freed: free(ptr)
Java: GC decides when memory is freed: when nothing points to the object

GC solves:
  - Memory leaks (forgot to free)
  - Double-free (free called twice)
  - Use-after-free (use after free)

GC introduces:
  - GC pauses (stop-the-world)
  - CPU overhead (marking + copying)
  - Non-deterministic pause timing
```

---

## Algorithm 1: Mark-Sweep (Serial GC, old default)

```text
Phase 1 — MARK (find all reachable objects):
  Start from GC roots:
    - Thread stacks (local variables)
    - Static fields
    - JNI references
  Walk the object graph: if A references B → B is reachable
  Mark each reachable object (set a bit in object header)

Phase 2 — SWEEP (free unreachable objects):
  Walk the entire heap linearly
  Any object NOT marked → free it
  Reset marks on live objects

Phase 3 — COMPACT (optional):
  Slide all live objects to one end of heap
  Fixes heap fragmentation
  Cost: moves objects → must update all references

Weakness:
  Stop-the-world: entire application pauses during mark+sweep
  Large heaps = long pauses
```

---

## Algorithm 2: Generational GC (G1GC, default since Java 9)

```text
OBSERVATION: Most objects die young.
  "90% of objects die before their first GC" — Generational Hypothesis

Two generations:
  Young Gen (Eden + 2 Survivors):
    - Minor GC: collect Eden, small pause (~10ms)
    - Only young objects, small region → fast

  Old Gen (Tenured):
    - Major GC: collect old objects, longer pause (~100ms)
    - Objects promoted after 15 Minor GCs

G1GC (Garbage First, Java 9+ default):
  - Divides heap into equal-sized REGIONS (~1-32MB each)
  - Prioritizes collecting regions with most garbage first
  - Concurrent marking (runs while app runs)
  - Target: -XX:MaxGCPauseMillis=200 (pause goal)
```

---

## Algorithm 3: ZGC (Java 15+, low-latency)

```text
Goal: pauses < 1ms even on TB heaps

Key techniques:
  Colored pointers: encode GC state IN the pointer bits
  Load barriers:    on every pointer load, check if remapping needed
  Concurrent:       nearly all phases run alongside application

Phases (all concurrent except initial/final mark):
  1. Initial Mark       (pause < 1ms)
  2. Concurrent Mark    (runs with app)
  3. Remap/Relocation   (concurrent)
  4. Final Remap        (pause < 1ms)

When to use: latency-sensitive apps (trading, gaming, real-time APIs)
When NOT to use: throughput-sensitive batch jobs (G1 is better)
```

---

## GC Tuning Flags

```text
# Choose GC algorithm
-XX:+UseSerialGC          # single-threaded, small heaps
-XX:+UseParallelGC        # throughput-focused, parallel GC threads
-XX:+UseG1GC              # default since Java 9 (balanced)
-XX:+UseZGC               # ultra-low latency, Java 15+
-XX:+UseShenandoahGC      # alternative low-latency (Red Hat)

# Heap sizing
-Xms2g -Xmx4g             # initial=2GB, max=4GB

# G1 tuning
-XX:MaxGCPauseMillis=200  # target max pause
-XX:G1HeapRegionSize=8m   # region size (1-32MB, power of 2)
-XX:ConcGCThreads=4       # concurrent GC threads

# Logging (Java 11+)
-Xlog:gc*:file=gc.log:time,uptime,level,tags
```

---

## Reading GC Logs

```text
[0.134s][info][gc] GC(0) Pause Young (Normal) (G1 Evacuation Pause) 24M->8M(256M) 5.234ms
                                                                      ^before ^after ^heap   ^pause

Important metrics:
  Pause time:     should be < MaxGCPauseMillis
  Heap before:    how full was heap
  Heap after:     how much was freed
  Frequency:      Minor GC every N seconds (too frequent = heap too small)
  Full GC:        ALARM — usually means memory pressure or leak
```

---

## Memory Leak in Java (yes, they exist)

```java
// Classic: static collection holds references forever
class EventBus {
    private static final List<Listener> listeners = new ArrayList<>();
    public static void register(Listener l) { listeners.add(l); }
    // MISSING: unregister() method!
    // Every registered listener is held by static → GC can never collect them
}

// Fix: use WeakReference
private static final List<WeakReference<Listener>> listeners = new ArrayList<>();
// WeakReference allows GC to collect the listener if nobody else holds it
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `mark_sweep.c` | Full mark-sweep-compact algorithm on a simulated heap |
| `generational_gc.c` | Young/Old gen collection with promotion |
| `gc_roots.c` | GC root scanning from thread stacks + static fields |

---

## Build and Run

```bash
gcc -Wall -Wextra -o mark_sweep mark_sweep.c && ./mark_sweep
gcc -Wall -Wextra -o generational_gc generational_gc.c && ./generational_gc
```

---

## Key Insight

```text
GC is NOT free — it costs CPU time.
Long GC pauses = application appears frozen.

The GC tuning goal:
  THROUGHPUT apps:  maximize objects collected per GC pause (batch jobs)
  LATENCY apps:     minimize individual pause times (web APIs, trading)

In Spring Boot:
  Default G1GC usually good enough.
  Watch /actuator/metrics/jvm.gc.pause → alert if p99 > 500ms.
  Memory leak symptoms: increasing Old Gen usage, frequent Full GC.
```
