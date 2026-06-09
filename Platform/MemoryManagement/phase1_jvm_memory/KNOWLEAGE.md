# 🧠 Phase 1: JVM Memory Model

> **Java Concept**: Heap, Stack, Metaspace, Code Cache
> **C Equivalent**: malloc() regions + stack frames + BSS/data segment

---

## Why This Matters

```text
When you write:   String s = new String("hello");
JVM does:
  1. Allocates 40 bytes on the Heap (String object + char[] + header)
  2. Points local variable s (on the Stack) to that Heap address
  3. Stores String class metadata in Metaspace
  4. JIT may compile String() constructor → Code Cache

When s goes out of scope:
  Stack frame is popped (s local var is gone)
  Heap object is NOT freed immediately
  GC eventually marks it unreachable → collects it
```

---

## JVM Memory Regions

### 1. Heap (where objects live)

```text
┌────────────────────────────────────────────────┐
│  Young Generation  (default: ~1/3 of heap)    │
│  ┌────────────┐ ┌──────────┐ ┌──────────┐     │
│  │   Eden     │ │Survivor 0│ │Survivor 1│     │
│  │  (~80%)    │ │  (~10%)  │ │  (~10%)  │     │
│  └────────────┘ └──────────┘ └──────────┘     │
├────────────────────────────────────────────────┤
│  Old Generation / Tenured  (~2/3 of heap)     │
│  Long-lived objects (survived >15 GCs)        │
└────────────────────────────────────────────────┘

Tuning:
  -Xms512m   → initial heap size
  -Xmx2g     → max heap size
  -Xmn256m   → young generation size
```

### 2. JVM Stack (per-thread, NOT shared)

```text
Each thread has its own stack.
Each method call pushes a Stack Frame containing:
  - Local variable table  (primitives + object references)
  - Operand stack         (working area for bytecode)
  - Frame data            (class reference, return address)

Stack overflow:
  -Xss512k   → stack size per thread (default: 512k-1m)
  Deep recursion → StackOverflowError

Important: Objects are on the HEAP. The Stack holds REFERENCES to them.
  int x = 5;       ← x lives on stack (primitive)
  String s = "hi"; ← "hi" lives on heap; s (reference) lives on stack
```

### 3. Metaspace (class metadata, since Java 8)

```text
Before Java 8: PermGen (fixed size → OutOfMemoryError: PermGen)
Java 8+:       Metaspace (native memory, grows dynamically)

Contains:
  - Class structures (method names, field names, bytecode)
  - Static fields (class-level, not instance-level)
  - String pool (interned strings)
  - JIT compiled class code references

Tuning:
  -XX:MetaspaceSize=128m       → initial metaspace
  -XX:MaxMetaspaceSize=256m    → cap (prevent native OOM)

OutOfMemoryError: Metaspace → too many classes loaded (class leaks, OSGi)
```

### 4. Code Cache (JIT compiled code)

```text
JVM interprets bytecode first (slow).
After ~10,000 invocations → JIT compiles to native machine code.
Compiled code stored in Code Cache (native memory).

Flags:
  -XX:ReservedCodeCacheSize=256m

Code Cache full → JIT compilation disabled → performance degrades
```

---

## Object Memory Layout

```text
Java object on the heap:
  ┌─────────────────────────────┐
  │  Object Header (16 bytes)   │
  │    Mark Word (8 bytes)      │  → GC age, lock state, hash
  │    Klass pointer (4-8 bytes)│  → class metadata in Metaspace
  ├─────────────────────────────┤
  │  Instance fields            │
  │    (in declaration order,   │
  │     JVM may reorder)        │
  └─────────────────────────────┘

Minimum object size: 16 bytes (even for new Object())
Field padding: JVM aligns fields to their natural alignment.

Example: class Point { int x; int y; }
  Header: 16 bytes
  x:       4 bytes
  y:       4 bytes
  Total:  24 bytes
```

---

## C Files in This Phase

| File | What It Simulates |
|------|------------------|
| `heap_stack_model.c` | Stack frame management + heap simulation + JVM region model |
| `metaspace_classdata.c` | Class metadata storage (Metaspace simulation) |

---

## Build and Run

```bash
gcc -Wall -Wextra -o heap_stack_model heap_stack_model.c && ./heap_stack_model
gcc -Wall -Wextra -o metaspace_classdata metaspace_classdata.c && ./metaspace_classdata
```

---

## Key Insight

```text
C gives you ONE flat heap and ONE stack per thread.
JVM gives you a STRUCTURED heap (Young/Old) + per-thread stacks.

The structure exists to support GC efficiency:
  Young gen = short-lived objects → collected frequently (Minor GC, fast)
  Old gen   = long-lived objects → collected rarely (Major/Full GC, slow)

The Stack is always thread-local → no synchronization needed for locals.
The Heap is shared across threads → synchronization IS needed for objects.
```
