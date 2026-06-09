# Master Plan: Java Internals in C

> Goal: Understand every major Java class/feature by re-implementing its
> internal mechanism in C. Each file is self-contained, compilable, and
> shows exactly how Java works under the hood.

---

## Progress Tracker

### ✅ DONE — Collections (java_collections/)

| File | Java Class | Key Concepts |
|------|-----------|-------------|
| ✅ 01_ArrayList.c | ArrayList<T> | Dynamic array, grow ×1.5, O(1) amortized add |
| ✅ 02_LinkedList.c | LinkedList<T> | Doubly-linked, O(1) addFirst/Last, O(n) get |
| ✅ 03_HashMap.c | HashMap<K,V> | Hash table, chaining, load factor 0.75, resize |
| ✅ 04_HashSet.c | HashSet<T> | HashMap wrapper, PRESENT dummy value |
| ✅ 05_Stack_Queue.c | Stack/Queue/PriorityQueue | Array stack, circular queue, binary heap |
| ✅ 06_String_StringBuilder.c | String/StringBuilder | Immutable string, mutable buffer, grow ×2 |

### ✅ DONE — Foundation (era01_foundation/)

| File | Java Concept | Key Concepts |
|------|-----------|-------------|
| ✅ 01_memory_pain.c | GC motivation | Memory leak, double free, use-after-free |
| ✅ 02_safe_array.c | int[] / String[] | Object header, bounds check, clone |

### ✅ DONE — Generics (era03_generics/)

| File | Java Concept | Key Concepts |
|------|-----------|-------------|
| ✅ 01_void_ptr_pain.c | Generics motivation | void* pain, tagged union, type erasure |

---

## 🔲 TODO — Collections (java_collections/)

| File | Java Class | Key Concepts | Priority |
|------|-----------|-------------|--------|
| 🔲 07_TreeMap.c | TreeMap<K,V> | Red-Black tree, sorted keys, O(log n) all ops | HIGH |
| 🔲 08_LinkedHashMap.c | LinkedHashMap<K,V> | HashMap + insertion-order doubly-linked list | HIGH |
| 🔲 09_Iterator.c | Iterator<T> | Iterator pattern, hasNext/next, fail-fast | HIGH |
| 🔲 10_Comparable_Comparator.c | Comparable/Comparator | Function ptr based sorting, qsort | MEDIUM |

---

## 🔲 TODO — OOP Internals (java_oop/)

| File | Java Concept | Key Concepts | Priority |
|------|-----------|-------------|--------|
| 🔲 01_vtable.c | Polymorphism / virtual dispatch | vtable (function pointer table), inheritance | HIGH |
| 🔲 02_interface.c | Interface | Pure abstract vtable, multiple dispatch | HIGH |
| 🔲 03_object_header.c | Object (base class) | Mark word, class pointer, GC header | HIGH |
| 🔲 04_inheritance.c | extends | struct embedding, field layout | MEDIUM |
| 🔲 05_exception.c | try/catch/finally | setjmp/longjmp, stack unwinding | HIGH |
| 🔲 06_enum_impl.c | enum | Tagged constants with methods | LOW |

---

## 🔲 TODO — Java 8 Functional (java_functional/)

| File | Java Concept | Key Concepts | Priority |
|------|-----------|-------------|--------|
| 🔲 01_function_ptr.c | Lambda / Function<T,R> | Function pointers, closure simulation | HIGH |
| 🔲 02_stream_pipeline.c | Stream API | filter/map/reduce pipeline with fn ptrs | HIGH |
| 🔲 03_optional.c | Optional<T> | Tagged union, monadic chaining | HIGH |
| 🔲 04_closure.c | Lambda capture | Closure struct (captured env + fn ptr) | MEDIUM |

---

## 🔲 TODO — JVM Internals (java_jvm/)

| File | Java Concept | Key Concepts | Priority |
|------|-----------|-------------|--------|
| 🔲 01_object_lifecycle.c | new / GC | Heap alloc, reference counting GC | HIGH |
| 🔲 02_mark_sweep_gc.c | Garbage Collector | Mark phase, sweep phase, roots | HIGH |
| 🔲 03_stack_frame.c | Method call / stack | Call stack, local vars, operand stack | HIGH |
| 🔲 04_class_loader.c | ClassLoader | Name→class table, lazy loading | MEDIUM |
| 🔲 05_string_pool.c | String.intern() | String interning pool (hashtable) | MEDIUM |

---

## 🔲 TODO — Modern Java (java_modern/)

| File | Java Concept | Key Concepts | Priority |
|------|-----------|-------------|--------|
| 🔲 01_record.c | Record (Java 16) | Immutable const struct, auto-equals | HIGH |
| 🔲 02_sealed_union.c | Sealed class (Java 17) | Tagged union, exhaustive switch | HIGH |
| 🔲 03_virtual_thread.c | Virtual Thread (Java 21) | Cooperative coroutine with ucontext | HIGH |
| 🔲 04_pattern_match.c | Pattern matching | Switch on type tag | MEDIUM |

---

## Build System

```
c_vs_java/
├── java_collections/   makefile  ← build all collection C files
├── java_oop/           makefile
├── java_functional/    makefile
├── java_jvm/           makefile
├── java_modern/        makefile
└── makefile            ← build everything
```

---

## Learning Sequence (Recommended Order)

### Week 1 — Collections (the most-used in daily Java)
```
Day 1: 01_ArrayList.c       → understand grow, amortized O(1)
Day 2: 02_LinkedList.c      → nodes, pointers, trade-offs vs ArrayList
Day 3: 03_HashMap.c         → hash, buckets, load factor, resize
Day 4: 04_HashSet.c         → see it's just HashMap with PRESENT value
Day 5: 05_Stack_Queue.c     → circular buffer, binary heap
Day 6: 06_String_StringBuilder.c → immutable vs mutable buffer
Day 7: 07_TreeMap.c         → red-black tree concepts
```

### Week 2 — OOP Internals
```
Day 1: 01_vtable.c          → how virtual dispatch works (the JVM's core)
Day 2: 02_interface.c       → pure vtable, multiple interface simulation
Day 3: 03_object_header.c   → every Java object's hidden 8-16 bytes
Day 4: 04_inheritance.c     → struct embedding = Java extends
Day 5: 05_exception.c       → setjmp/longjmp = try/catch
Day 6: java_jvm/01_object_lifecycle.c → GC basics
Day 7: java_jvm/02_mark_sweep_gc.c    → full GC simulation
```

### Week 3 — Functional & Modern
```
Day 1: java_functional/01_function_ptr.c  → lambdas are function pointers
Day 2: java_functional/02_stream_pipeline.c → Stream in C
Day 3: java_functional/03_optional.c     → Optional as tagged union
Day 4: java_modern/01_record.c           → immutable struct
Day 5: java_modern/02_sealed_union.c     → tagged union = sealed class
Day 6: java_jvm/03_stack_frame.c         → method call internals
Day 7: java_modern/03_virtual_thread.c   → coroutines = virtual threads
```

---

## How to Use Each File

```
1. Read the header comment — it shows the Java source equivalent
2. Compile and run: gcc -Wall file.c -o out && ./out
3. Read the output — it prints what Java hides from you
4. Compare with the matching Java file in examples/history/
5. Ask: "What does Java do automatically that I did manually here?"
```

---

## Key Insight Pattern

Every file follows this pattern:
```
JAVA PAIN (before)          C SOLUTION (how Java fixes it)
─────────────────           ─────────────────────────────
Raw arrays → overflow   →   bounds-checked array struct
void* → no type safety  →   typed containers (generics)
malloc/free → leaks     →   reference counting / GC
anonymous class → verbose → function pointer (lambda)
null → NPE              →   Optional (tagged union)
```

---

## Files Remaining to Create

**Next session priority order:**

1. `java_oop/01_vtable.c`       ← Most important! Explains all Java OOP
2. `java_oop/05_exception.c`    ← try/catch with setjmp/longjmp
3. `java_functional/01_function_ptr.c` ← Lambda = function pointer + closure
4. `java_functional/02_stream_pipeline.c` ← Stream pipeline
5. `java_jvm/02_mark_sweep_gc.c` ← Full GC simulation
6. `java_jvm/03_stack_frame.c`  ← Method call stack
7. `java_collections/07_TreeMap.c` ← Red-black tree
8. `java_modern/02_sealed_union.c` ← Sealed class as tagged union
9. `java_modern/03_virtual_thread.c` ← Coroutine = virtual thread

**Total remaining: ~15 files**
