# 📦 Phase 8: Concurrent Collections

> **Java Concept**: `ConcurrentHashMap`, `BlockingQueue`, `CopyOnWriteArrayList`
> **C Equivalent**: Segment-locked hash table, lock-guarded linked list

---

## Why Not Just Synchronize?

```java
// BAD: wrapping HashMap with synchronized
Map<String, Object> map = Collections.synchronizedMap(new HashMap<>());
synchronized(map) { // must hold lock for compound operations
    if (!map.containsKey(key)) {
        map.put(key, computeValue(key));  // still needs external sync
    }
}
// Entire map is locked → ONE thread at a time → terrible throughput

// GOOD: ConcurrentHashMap — fine-grained locking
ConcurrentHashMap<String, Object> map = new ConcurrentHashMap<>();
map.putIfAbsent(key, computeValue(key));  // atomic, no external sync needed
// Only the affected BUCKET is locked → many threads operate simultaneously
```

---

## ConcurrentHashMap Internals (Java 8+)

```text
Java 7:  16 segments, each a mini-HashMap with its own lock.
         Max 16 concurrent writes.

Java 8+: Node-level locking (synchronized on individual bucket head).
         Max concurrency = number of buckets (~16 to millions).
         Reads: lock-FREE using volatile head pointer.
         Writes: lock only the SPECIFIC bucket being modified.

Structure:
  table[] → array of Node (each bucket)
  Node {
    final int hash;
    final K key;
    volatile V val;       // volatile → lock-free reads
    volatile Node next;   // volatile → lock-free list traversal
  }

  Write:
    head = table[hash(key)];
    synchronized(head) { insert/update node }  // lock just this bucket

  Read:
    follow Node.next volatile chain → no lock needed
```

---

## ConcurrentHashMap Key Methods

```java
ConcurrentHashMap<String, Integer> map = new ConcurrentHashMap<>();

// Atomic: if key absent, compute and put
map.computeIfAbsent(key, k -> expensiveCompute(k));

// Atomic: update existing value
map.computeIfPresent(key, (k, v) -> v + 1);

// Atomic: put only if not present
map.putIfAbsent(key, defaultValue);

// Atomic: conditional remove
map.remove(key, expectedValue);  // only removes if value matches

// Aggregate — uses parallel streams internally
long sum = map.reduceValues(1, Long::sum);
```

---

## BlockingQueue — Thread-Safe Queue with Back-Pressure

```java
// Bounded: blocks producer when full (back-pressure)
BlockingQueue<Task> queue = new LinkedBlockingQueue<>(100);

// Producer:
queue.put(task);        // BLOCKS if queue full
queue.offer(task, 5, SECONDS);  // timeout

// Consumer:
Task t = queue.take();  // BLOCKS if queue empty
Task t = queue.poll(5, SECONDS);  // timeout

// Non-blocking:
boolean added = queue.offer(task);  // returns false if full (no block)
Task t = queue.poll();              // returns null if empty (no block)

// Real usage: ThreadPoolExecutor uses BlockingQueue for its task queue!
```

---

## CopyOnWriteArrayList — Read-Heavy Lists

```java
// For: event listener lists, config caches (many reads, rare writes)
CopyOnWriteArrayList<Listener> listeners = new CopyOnWriteArrayList<>();

// Write: copies the ENTIRE array + adds element + replaces reference (atomic)
listeners.add(newListener);

// Read: iterates the SNAPSHOT (no lock, never throws ConcurrentModificationException)
for (Listener l : listeners) { l.onEvent(event); }

// Cost: writes are O(n) memory allocation → only for write-rare scenarios
// Spring's ApplicationListener list is a CopyOnWriteArrayList
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `concurrent_hashmap.c` | Bucket-level locking + lock-free reads |
| `blocking_queue.c` | Bounded blocking queue with back-pressure |
| `copy_on_write_list.c` | Copy-on-write snapshot semantics |

---

## Build and Run

```bash
gcc -Wall -Wextra -pthread -o concurrent_hashmap concurrent_hashmap.c && ./concurrent_hashmap
gcc -Wall -Wextra -pthread -o blocking_queue blocking_queue.c && ./blocking_queue
```

---

## Key Insight

```text
ConcurrentHashMap is the most important concurrent collection.
Spring's ApplicationContext caches beans in a ConcurrentHashMap.
Spring caches (@Cacheable) default to ConcurrentHashMap-backed stores.

Rules:
  Multiple concurrent reads AND writes  → ConcurrentHashMap
  Queue with blocking + back-pressure   → LinkedBlockingQueue
  Rare writes, many reads (listeners)   → CopyOnWriteArrayList
  Sorted concurrent access              → ConcurrentSkipListMap
  Counting (high contention)            → LongAdder / ConcurrentHashMap.compute()

DON'T use:
  HashMap in multithreaded code         → data corruption / infinite loop
  ArrayList in multithreaded code       → ConcurrentModificationException
  Collections.synchronizedXxx()         → coarse-grained locking, poor throughput
```
