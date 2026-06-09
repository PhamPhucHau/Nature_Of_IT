# C vs Java — Implementing Java's Improvements in C

> Each folder shows the **C pain** that drove a Java feature,
> then the C equivalent of how Java solved it internally.
>
> This teaches you to think about what the JVM actually does under the hood.

---

## Folder Map

```
c_vs_java/
├── era01_foundation/     Java 1.0 — Memory safety, arrays, strings
│   ├── 01_memory_pain.c        Raw malloc/free — the problem
│   ├── 02_safe_array.c         Bounds-checked array (what Java does)
│   └── 03_string_builder.c     Immutable string + StringBuilder in C
│
├── era03_generics/       Java 5 — Type-safe containers
│   ├── 01_void_ptr_pain.c      void* containers — the C pain
│   ├── 02_typed_list.c         Type-safe list using macros (like generics)
│   └── 03_typed_hashmap.c      Type-safe hashmap
│
├── era05_collections/    Java 1.2–5 — ArrayList, LinkedList, HashMap
│   ├── 01_arraylist.c          Dynamic array (ArrayList in C)
│   ├── 02_linkedlist.c         Doubly-linked list (LinkedList in C)
│   └── 03_hashmap.c            Hash map with chaining (HashMap in C)
│
├── era07_lambda/         Java 8 — Lambdas, Streams, Optional
│   ├── 01_func_ptrs.c          C function pointers (the raw mechanism)
│   ├── 02_stream.c             Stream pipeline (filter/map/reduce) in C
│   └── 03_optional.c           Option type (Optional<T>) in C
│
└── era09_modern/         Java 16–21 — Records, Sealed, Pattern matching
    ├── 01_record.c             Immutable struct (Record) in C
    ├── 02_sealed_union.c       Tagged union (Sealed class) in C
    └── 03_pattern_match.c      Pattern matching switch in C
```

## Build All

```bash
cd c_vs_java
make all    # builds every example
make run    # runs all examples
make clean  # removes all binaries
```

## Build One Era

```bash
cd era01_foundation
make
./01_memory_pain
./02_safe_array
./03_string_builder
```

## Learning Sequence

1. Run the C `_pain` file → feel the problem
2. Run the C `_fixed` / implementation file → see the solution
3. Read the matching Java file in `examples/history/` → see how Java packages it
4. Compare: Java just wraps the C solution and adds compile-time safety

---

## Key Insight

Java didn't invent new algorithms. It **packaged** well-known C patterns:

| Java Feature | C Equivalent | Java adds... |
|---|---|---|
| GC | `malloc/free` + reference counting | Automatic, no leaks |
| `ArrayList<T>` | Dynamic array with `realloc` | Type safety, bounds checks |
| `HashMap<K,V>` | Hash table with chaining | Type safety, auto-resize |
| Lambda | Function pointer | Closure capture, type inference |
| `Optional<T>` | Tagged union / sentinel value | Chaining, no NPE |
| `Record` | `const struct` | Auto equals/hashCode/toString |
| Sealed class | Tagged union (enum + union) | Exhaustiveness checking |
| Virtual thread | `ucontext`/coroutine | OS-transparent scheduling |
