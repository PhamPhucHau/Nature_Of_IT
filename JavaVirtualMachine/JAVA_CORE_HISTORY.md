# Java Core — History, Pain Points & Evolution

> Learn Java by understanding **why** each feature was created, not just **what** it does.
> Every example in this doc has a corresponding runnable file in `examples/history/`.

---

## Table of Contents

1. [Origin Story (1991–1995)](#1-origin-story)
2. [The Pain of C/C++ — Java's Solutions](#2-the-pain-of-cc)
3. [Java 1.0–1.1 (1996–1997): The Foundation](#3-java-10-11-the-foundation)
4. [Java 1.2–1.4 (1998–2002): Growing Up](#4-java-12-14-growing-up)
5. [Java 5 (2004): The Generics Revolution](#5-java-5-the-generics-revolution)
6. [Java 6–7 (2006–2011): Stability & Polish](#6-java-6-7-stability--polish)
7. [Java 8 (2014): Lambda — The Biggest Shift](#7-java-8-lambda--the-biggest-shift)
8. [Java 9–11 (2017–2018): Modules & LTS](#8-java-9-11-modules--lts)
9. [Java 14–21 (2020–2023): Records, Sealed, Patterns](#9-java-14-21-records-sealed-patterns)
10. [Java vs C: When to Use Which](#10-java-vs-c-when-to-use-which)
11. [Running the Examples](#11-running-the-examples)

---

## 1. Origin Story

### The World in 1991

```
State of software in 1991:
├── C/C++ dominated → Fast but dangerous (pointers, memory leaks)
├── Windows 3.1 ruled desktop
├── Internet was not yet mainstream
└── Embedded devices (set-top boxes, smart TVs) were exploding
```

**James Gosling** and his team at Sun Microsystems were tasked with writing
software for a **cable TV set-top box**. They used C++ and immediately hit
three massive pain points:

| Pain | Symptom | Consequence |
|------|---------|-------------|
| Manual memory management | `malloc/free` everywhere | Device crashed randomly |
| Platform dependency | Different compile per chip | Rewrite for every device |
| Unsafe pointer arithmetic | `*(ptr + n)` unchecked | Buffer overflows, security holes |

**Decision:** Build a new language. First called **Oak**, renamed **Java** in 1995.

The core philosophy: *"Write Once, Run Anywhere"* (WORA).

---

## 2. The Pain of C/C++

### Pain #1: Memory — "Who frees this?"

```c
/* C — the pain */
char* name = malloc(100);
strcpy(name, "Hello");
// ... 500 lines later ...
// Forgot free(name)  → MEMORY LEAK (device slows and crashes)
// free(name) twice   → DOUBLE FREE → undefined behavior → crash
// free() wrong size  → HEAP CORRUPTION
```

```java
// Java — the fix: Garbage Collector handles it
String name = "Hello";
// JVM frees it automatically when no references remain
// You CANNOT leak memory this way
```

**How it works:** The JVM uses **Mark-and-Sweep GC** — traces all live references
from roots (stack, static fields), marks reachable objects, then sweeps (frees) everything else.

### Pain #2: Platform — "Works on my machine"

```
C/C++ compile pipeline:
  hello.c  →  gcc (Linux x86)  →  ./hello      (only runs on Linux x86)
  hello.c  →  MSVC (Windows)   →  hello.exe    (only runs on Windows)
  hello.c  →  gcc (ARM)        →  ./hello      (only runs on ARM)
  → Must recompile for each target platform

Java compile pipeline:
  Hello.java  →  javac  →  Hello.class (bytecode)
  Hello.class →  JVM on Linux   → runs ✓
  Hello.class →  JVM on Windows → runs ✓
  Hello.class →  JVM on ARM     → runs ✓
  → Compile ONCE, run ANYWHERE with a JVM
```

### Pain #3: Pointers — "Segfault with no explanation"

```c
/* C — unsafe pointer arithmetic */
int arr[5] = {1, 2, 3, 4, 5};
int* ptr = arr;
*(ptr + 10) = 999;   // Writes to memory you don't own!
// Result: segfault, or worse — silently corrupts data
// Debugging this takes hours or days
```

```java
// Java — safe array access
int[] arr = {1, 2, 3, 4, 5};
arr[10] = 999;   // → ArrayIndexOutOfBoundsException: Index 10 out of bounds for length 5
// Clear error, predictable, debuggable
// The JVM catches it before it corrupts memory
```

---

## 3. Java 1.0–1.1: The Foundation (1996–1997)

### What Java 1.0 had

```java
// The simplest possible Java program
public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello, World!");
    }
}
```

### What was missing and painful

| Missing | Consequence |
|---------|-------------|
| No inner classes | Event handlers required separate top-level classes |
| No Collections framework | Used `Vector`/`Hashtable` — both synchronized, both slow |
| No proper I/O | Byte streams only; text encoding was a nightmare |
| No proper threading primitives | Race conditions everywhere |

### Java 1.1 — First Improvements

**Inner classes** arrived — the ancestor of lambda:

```java
// Java 1.1 — Anonymous inner class (verbose but a big improvement)
button.addActionListener(new ActionListener() {
    @Override
    public void actionPerformed(ActionEvent e) {
        System.out.println("Button clicked!");
    }
});
// Pain: 5 lines to pass a single function. Java 8 fixes this.
```

**Reflection** — inspect classes at runtime:

```java
// Java 1.1 — Reflection
Class<?> clazz = Class.forName("java.lang.String");
Method[] methods = clazz.getMethods();
for (Method m : methods) {
    System.out.println(m.getName());
}
// Foundation for frameworks like Spring, Hibernate
```

---

## 4. Java 1.2–1.4: Growing Up (1998–2002)

### Java 1.2 — "Java 2" — The Collections Framework

The single biggest quality-of-life improvement up to that point:

```java
// BEFORE Java 1.2 — Vector: synchronized (slow), type-unsafe
Vector v = new Vector();
v.addElement("hello");
v.addElement(42);             // Mixed types — no compiler warning!
String s = (String) v.get(1); // ClassCastException at RUNTIME

// AFTER Java 1.2 — ArrayList: fast, but still no generics yet
List list = new ArrayList();
list.add("hello");
String s = (String) list.get(0); // Still need a cast... pain not fully gone
// Fixed in Java 5 with generics.
```

**New data structures in 1.2:**
```
List  →  ArrayList (fast random access), LinkedList (fast insert/delete)
Map   →  HashMap (fast lookup), TreeMap (sorted), LinkedHashMap (insertion order)
Set   →  HashSet, TreeSet, LinkedHashSet
Queue →  (added later in Java 5)
```

### Java 1.4 — NIO and Assertions

```java
// Java 1.4 — assert keyword
int age = -5;
assert age >= 0 : "Age cannot be negative: " + age;
// Fails with AssertionError when run with: java -ea MyClass

// Java 1.4 — NIO (Non-Blocking I/O)
// BEFORE: 1 thread per connection → 1000 users = 1000 threads = ~1GB RAM
// AFTER:  1 thread handles many connections via Selector
// This became the foundation for high-performance servers
```

---

## 5. Java 5: The Generics Revolution (2004)

The most impactful release between Java 1.0 and Java 8.

### The Core Problem: No Type Safety

```java
// BEFORE Java 5 — Raw types, no safety net
List myList = new ArrayList();
myList.add("hello");
myList.add(42);          // Compiler says nothing
myList.add(new Object()); // Still nothing

// Crash at runtime — hours after the bug was introduced:
String s = (String) myList.get(1); // ClassCastException: Integer cannot be cast to String
```

### Fix #1: Generics

```java
// AFTER Java 5 — Type-safe at compile time
List<String> myList = new ArrayList<String>();
myList.add("hello");
// myList.add(42);       // COMPILE ERROR — caught immediately
// myList.add(new Object()); // COMPILE ERROR

String s = myList.get(0); // No cast needed — compiler knows it's a String
```

### Fix #2: Enhanced for-loop

```java
// BEFORE — verbose and error-prone
for (int i = 0; i < list.size(); i++) {
    String s = (String) list.get(i);
    System.out.println(s);
}

// AFTER Java 5 — clean iteration
for (String s : list) {
    System.out.println(s);
}

// Works on any Iterable — arrays, lists, sets, custom collections
int[] nums = {1, 2, 3, 4, 5};
for (int n : nums) {
    System.out.println(n);
}
```

### Fix #3: Autoboxing / Unboxing

```java
// BEFORE — manual boxing
Integer wrapped = Integer.valueOf(42);
int primitive  = wrapped.intValue();
List<Integer> list = new ArrayList<Integer>();
list.add(Integer.valueOf(5));  // Can't add int to List<Integer> directly
int x = list.get(0).intValue();

// AFTER Java 5 — automatic conversion
Integer wrapped = 42;          // int → Integer (boxing)
int primitive  = wrapped;      // Integer → int (unboxing)
List<Integer> list = new ArrayList<Integer>();
list.add(5);                   // Auto-boxes 5 → Integer(5)
int x = list.get(0);          // Auto-unboxes Integer(5) → 5
```

### Fix #4: Varargs

```java
// BEFORE — ugly array parameter
void log(String format, Object[] args) { ... }
log("Hello %s", new Object[]{"world"});  // Ugly!

// AFTER Java 5 — varargs
void log(String format, Object... args) { ... }
log("Hello %s", "world");             // Clean
log("Sum: %d + %d = %d", 1, 2, 3);  // Any number of args
// Under the hood, Java still wraps them into an array
```

### Fix #5: Enums

```java
// BEFORE — int constants: no type safety
public static final int MONDAY    = 1;
public static final int TUESDAY   = 2;
// Problem: someone can call setDay(999) — compiler won't stop them!
void setDay(int day) { ... }
setDay(999); // Compiles fine — bug at runtime

// AFTER Java 5 — enum: type-safe
public enum Day {
    MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY, SUNDAY;

    public boolean isWeekend() {
        return this == SATURDAY || this == SUNDAY;
    }
}
Day day = Day.MONDAY;
// setDay(999); // COMPILE ERROR — only Day values accepted
System.out.println(day.isWeekend()); // false
```

### Fix #6: Annotations

```java
// Java 5 introduced metadata on code
@Override                    // Tell compiler you intend to override
public String toString() { return "MyClass"; }

@Deprecated                  // Mark as old — IDEs warn users
public void oldMethod() { }

@SuppressWarnings("unchecked")  // Silence a specific warning
public void legacyCode() { ... }

// You can define your own annotations too:
@interface MyAnnotation {
    String value();
}
```

---

## 6. Java 6–7: Stability & Polish (2006–2011)

### Java 7 — "Project Coin" — Small but Valuable

```java
// 1. DIAMOND OPERATOR — Less typing, same safety
// Java 6:
Map<String, List<Integer>> map = new HashMap<String, List<Integer>>();
// Java 7:
Map<String, List<Integer>> map = new HashMap<>();  // Compiler infers the type

// 2. TRY-WITH-RESOURCES — Automatic resource cleanup
// BEFORE — error-prone finally blocks:
Connection conn = null;
Statement stmt = null;
try {
    conn = dataSource.getConnection();
    stmt = conn.createStatement();
    stmt.execute("...");
} catch (SQLException e) {
    e.printStackTrace();
} finally {
    if (stmt != null) try { stmt.close(); } catch (SQLException ignored) {}
    if (conn != null) try { conn.close(); } catch (SQLException ignored) {}
}
// 15 lines of boilerplate — easy to forget one close() call

// AFTER Java 7 — clean and guaranteed:
try (Connection conn = dataSource.getConnection();
     Statement stmt = conn.createStatement()) {
    stmt.execute("...");
}
// Both conn and stmt are ALWAYS closed, even if an exception is thrown
// Requires implementing AutoCloseable

// 3. MULTI-CATCH — DRY exception handling
// BEFORE:
try { riskyOperation(); }
catch (IOException e)   { logger.error(e); }
catch (SQLException e)  { logger.error(e); }   // Duplicate!
catch (ParseException e){ logger.error(e); }   // Duplicate!

// AFTER Java 7:
try { riskyOperation(); }
catch (IOException | SQLException | ParseException e) { logger.error(e); }

// 4. SWITCH ON STRINGS
String cmd = getUserInput();
switch (cmd) {  // Impossible before Java 7!
    case "start": startServer(); break;
    case "stop":  stopServer();  break;
    case "status": printStatus(); break;
    default: System.out.println("Unknown command: " + cmd);
}
```

### Java 7 — NIO.2 (Modern File API)

```java
// BEFORE Java 7 — java.io.File was broken
File file = new File("/tmp/data.txt");
file.delete();  // Returns false on failure — no exception, no reason!
file.mkdir();   // Returns false silently

// AFTER Java 7 — java.nio.file.Files is correct
Path path = Paths.get("/tmp/data.txt");
Files.delete(path);          // Throws IOException if it fails — always know why
Files.createDirectory(path); // Same

// Practical file operations:
Path src = Paths.get("source.txt");
Path dst = Paths.get("backup.txt");
Files.copy(src, dst, StandardCopyOption.REPLACE_EXISTING);

List<String> lines = Files.readAllLines(src, StandardCharsets.UTF_8);

// Walk a directory tree:
Files.walk(Paths.get("/tmp"))
     .filter(Files::isRegularFile)
     .forEach(System.out::println);
```

---

## 7. Java 8: Lambda — The Biggest Shift (2014)

Java 8 is the most transformative release since Java 1.0. It brings
**functional programming** into Java without breaking existing code.

### The Core Problem: Behavior as Data

```java
// BEFORE Java 8 — passing behavior required anonymous inner classes
List<String> names = Arrays.asList("Charlie", "Alice", "Bob");

Collections.sort(names, new Comparator<String>() {
    @Override
    public int compare(String a, String b) {
        return a.compareTo(b);
    }
});
// 6 lines to say "sort alphabetically"
```

### Lambda Expressions

```java
// AFTER Java 8 — lambdas
List<String> names = Arrays.asList("Charlie", "Alice", "Bob");

names.sort((a, b) -> a.compareTo(b));   // Full lambda
names.sort(String::compareTo);           // Method reference — even shorter

// Lambda syntax:
//   (parameters) -> expression
//   (parameters) -> { statements; }

// Examples:
Runnable r         = () -> System.out.println("running");
Comparator<String> c = (a, b) -> a.length() - b.length();
Predicate<String>  p = s -> s.startsWith("A");
Function<String, Integer> f = s -> s.length();
```

### Stream API — Process Collections Functionally

```java
List<Integer> numbers = List.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

// BEFORE Java 8 — imperative (you describe HOW)
List<Integer> evenSquares = new ArrayList<>();
for (Integer n : numbers) {
    if (n % 2 == 0) {
        evenSquares.add(n * n);
    }
}

// AFTER Java 8 — declarative (you describe WHAT)
List<Integer> evenSquares = numbers.stream()
    .filter(n -> n % 2 == 0)           // keep even numbers
    .map(n -> n * n)                   // square each
    .collect(Collectors.toList());     // collect to List

// More stream operations:
int sum = numbers.stream().mapToInt(Integer::intValue).sum();
long count = names.stream().filter(n -> n.startsWith("A")).count();
Optional<String> first = names.stream().filter(n -> n.length() > 4).findFirst();
String joined = names.stream().collect(Collectors.joining(", "));

// Parallel stream — use all CPU cores automatically:
long count = numbers.parallelStream().filter(n -> isPrime(n)).count();
```

### Optional — Eliminate NullPointerException

```java
// NullPointerException is called "The Billion Dollar Mistake" by its inventor

// BEFORE Java 8 — null chains are landmines
public String getCityName(User user) {
    if (user != null) {
        Address addr = user.getAddress();
        if (addr != null) {
            City city = addr.getCity();
            if (city != null) {
                return city.getName();  // Finally!
            }
        }
    }
    return "Unknown";
}
// 8 lines of null checks for one simple operation

// AFTER Java 8 — Optional chains
public String getCityName(User user) {
    return Optional.ofNullable(user)
        .map(User::getAddress)
        .map(Address::getCity)
        .map(City::getName)
        .orElse("Unknown");
}
// 5 lines, no null checks, no NPE possible

// Optional methods:
Optional<String> opt = Optional.of("hello");
opt.isPresent();              // true
opt.get();                    // "hello"
opt.orElse("default");       // "hello"
opt.orElseGet(() -> compute()); // lazy default
opt.ifPresent(System.out::println); // print if present
opt.map(String::toUpperCase); // Optional<String>("HELLO")
opt.filter(s -> s.length() > 3); // Optional<String>("hello")
```

### Default Methods in Interfaces

```java
// BEFORE Java 8 — adding a method to an interface BROKE all implementations
interface Animal {
    String speak();
    // Adding: String breathe();  ← breaks every class that implements Animal!
}

// AFTER Java 8 — default methods let you add without breaking
interface Animal {
    String speak();

    // Safe to add — existing implementations inherit this for free
    default String breathe() {
        return "inhale...exhale";
    }

    // Static utility methods in interfaces
    static Animal create(String type) {
        return switch (type) {
            case "dog" -> new Dog();
            case "cat" -> new Cat();
            default    -> throw new IllegalArgumentException(type);
        };
    }
}
// This is how Java added stream() to Collection<E> without breaking old code!
```

### Built-in Functional Interfaces

```java
// java.util.function package — ready to use

Function<String, Integer>   toLength  = String::length;         // A → B
BiFunction<Integer, Integer, Integer> add = (a, b) -> a + b;  // A, B → C
Predicate<String>           notEmpty  = s -> !s.isEmpty();      // A → boolean
Consumer<String>            printer   = System.out::println;    // A → void
Supplier<List<String>>      newList   = ArrayList::new;         // () → A
UnaryOperator<String>       upper     = String::toUpperCase;    // A → A
BinaryOperator<Integer>     max       = Integer::max;           // A, A → A

// Compose functions:
Function<String, String> trim  = String::trim;
Function<String, String> upper = String::toUpperCase;
Function<String, String> clean = trim.andThen(upper);
System.out.println(clean.apply("  hello world  ")); // "HELLO WORLD"
```

---

## 8. Java 9–11: Modules & LTS (2017–2018)

### Java 9 — Jigsaw: The Module System

```java
// Problem: JDK was monolithic (~500MB); every app loaded ALL of it
// Solution: Module system — declare exactly what you need and expose

// module-info.java (placed at src root)
module com.myapp {
    requires java.base;       // JDK core
    requires java.sql;        // JDBC
    requires java.net.http;   // HTTP client

    exports com.myapp.api;    // Public API — visible to other modules
    // com.myapp.internal is NOT exported — hidden by default!
}

// Build a minimal JRE for your app:
// jlink --add-modules java.base,java.sql \
//       --output myapp-runtime
// Result: ~30MB custom runtime instead of 500MB full JDK!
```

### Java 9 — Other Additions

```java
// Immutable collection factories (finally!)
List<String> list = List.of("a", "b", "c");          // Immutable
Set<Integer>  set  = Set.of(1, 2, 3);                 // Immutable
Map<String, Integer> map = Map.of("a", 1, "b", 2);   // Immutable
// list.add("d");  → UnsupportedOperationException

// Stream improvements
Stream.iterate(1, n -> n < 100, n -> n * 2)  // iterate with predicate
      .forEach(System.out::println);          // 1, 2, 4, 8, 16...

Stream.of(1, null, 2, null, 3)
      .flatMap(Optional::stream)              // remove nulls via Optional
      .forEach(System.out::println);

// Optional improvements
Optional.of("hello").ifPresentOrElse(
    System.out::println,                      // if present
    () -> System.out.println("empty")         // if empty
);
```

### Java 10 — `var`: Local Type Inference

```java
// BEFORE — repeat the type twice
HashMap<String, List<Integer>> frequencyMap = new HashMap<String, List<Integer>>();
ArrayList<Map.Entry<String, Integer>> entries = new ArrayList<Map.Entry<String, Integer>>();

// AFTER Java 10 — let the compiler figure it out
var frequencyMap = new HashMap<String, List<Integer>>();  // Still strongly typed!
var entries      = new ArrayList<Map.Entry<String, Integer>>();
var name         = "Alice";    // String
var count        = 0;          // int
var names        = List.of("Alice", "Bob");  // List<String>

// Rules:
// ✓ Only for local variables (inside methods)
// ✓ Must have an initializer (compiler needs something to infer from)
// ✗ Not for fields, method parameters, or return types
// ✗ Cannot initialize with null

for (var entry : frequencyMap.entrySet()) {  // also works in for-each
    System.out.println(entry.getKey() + ": " + entry.getValue());
}
```

### Java 11 — String Enhancements

```java
// Java 11 added useful String methods
"  hello  ".strip();      // "hello" — Unicode-aware (better than trim())
"  hello  ".stripLeading(); // "hello  "
"  hello  ".stripTrailing(); // "  hello"
"   ".isBlank();           // true — empty or whitespace only
"abc".repeat(3);            // "abcabcabc"
"line1\nline2\nline3".lines()          // Stream<String> of lines
                     .collect(Collectors.toList()); // ["line1", "line2", "line3"]
```

---

## 9. Java 14–21: Records, Sealed, Patterns (2020–2023)

### Java 16 — Records: Eliminate Data Class Boilerplate

```java
// BEFORE — a simple Point class needed 30+ lines
public class Point {
    private final double x;
    private final double y;

    public Point(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public double x() { return x; }
    public double y() { return y; }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Point)) return false;
        Point p = (Point) o;
        return Double.compare(p.x, x) == 0 && Double.compare(p.y, y) == 0;
    }

    @Override
    public int hashCode() { return Objects.hash(x, y); }

    @Override
    public String toString() { return "Point[x=" + x + ", y=" + y + "]"; }
}

// AFTER Java 16 — Record: 1 line!
public record Point(double x, double y) {}
// Java auto-generates: constructor, accessors (x(), y()),
//                      equals(), hashCode(), toString()

Point p1 = new Point(3.0, 4.0);
Point p2 = new Point(3.0, 4.0);
System.out.println(p1);           // Point[x=3.0, y=4.0]
System.out.println(p1.x());       // 3.0
System.out.println(p1.equals(p2)); // true

// Records can have custom methods and validation:
public record Range(int min, int max) {
    // Compact constructor — runs before the auto-generated one
    Range {
        if (min > max) throw new IllegalArgumentException("min > max");
    }

    public int span() { return max - min; }
    public boolean contains(int value) { return value >= min && value <= max; }
}
```

### Java 17 — Sealed Classes: Controlled Inheritance

```java
// Problem: You model a shape hierarchy. Anyone can extend Shape unexpectedly.
// Solution: Sealed classes — you control exactly who can extend/implement

public sealed interface Shape
    permits Circle, Rectangle, Triangle { }

// Only these three classes are permitted:
public record Circle(double radius)              implements Shape { }
public record Rectangle(double width, double height) implements Shape { }
public record Triangle(double base, double height)   implements Shape { }

// Benefit: Exhaustive pattern matching becomes possible (see below)
```

### Java 21 — Pattern Matching in switch

```java
// BEFORE — verbose instanceof + cast
Object obj = getObject();
if (obj instanceof String) {
    String s = (String) obj;
    System.out.println(s.toUpperCase());
} else if (obj instanceof Integer) {
    Integer i = (Integer) obj;
    System.out.println(i * 2);
}

// AFTER Java 16+ — pattern matching instanceof
if (obj instanceof String s) {   // bind directly to 's'
    System.out.println(s.toUpperCase());
} else if (obj instanceof Integer i) {
    System.out.println(i * 2);
}

// AFTER Java 21 — pattern matching in switch
double area = switch (shape) {
    case Circle    c -> Math.PI * c.radius() * c.radius();
    case Rectangle r -> r.width() * r.height();
    case Triangle  t -> 0.5 * t.base() * t.height();
    // No default needed — compiler knows Shape is exhaustive (sealed)
};

// With guards:
String classify = switch (number) {
    case Integer i when i < 0  -> "negative";
    case Integer i when i == 0 -> "zero";
    case Integer i             -> "positive";
    case Double  d             -> "decimal";
    default                    -> "other";
};
```

### Java 21 — Virtual Threads (Project Loom)

```java
// THE concurrency revolution

// Problem: Platform threads are expensive
// 1 platform thread ≈ 1MB of RAM
// 10,000 concurrent requests = 10GB RAM just for threads!
// This is why Node.js (event loop) outperformed Java servers for years

// BEFORE — thread pool limits concurrency
ExecutorService pool = Executors.newFixedThreadPool(200); // Hard cap at 200
IntStream.range(0, 10_000).forEach(i ->
    pool.submit(() -> {
        Thread.sleep(1000); // Blocking — wastes a platform thread
        return "done";
    })
);
// Only 200 run at a time; 9,800 wait in queue

// AFTER Java 21 — Virtual Threads
ExecutorService vt = Executors.newVirtualThreadPerTaskExecutor();
IntStream.range(0, 10_000).forEach(i ->
    vt.submit(() -> {
        Thread.sleep(1000); // Virtual thread is parked, not blocking OS thread
        return "done";
    })
);
// All 10,000 run "concurrently" — virtual threads are cheap (~few KB each)
// When a virtual thread blocks (I/O, sleep), the OS thread is freed
// and runs another virtual thread — same as event-loop but with normal sync code

// Also: structured concurrency (Java 21 preview)
try (var scope = new StructuredTaskScope.ShutdownOnFailure()) {
    Future<String> user  = scope.fork(() -> fetchUser(userId));
    Future<String> order = scope.fork(() -> fetchOrder(orderId));
    scope.join();           // Wait for both
    scope.throwIfFailed();  // Propagate any failure
    process(user.get(), order.get());
}
```

---

## 10. Java vs C: When to Use Which

### The honest trade-off table

| Dimension | C | Java |
|-----------|---|------|
| Raw speed | ★★★★★ | ★★★☆ (JIT narrows gap) |
| Dev speed | ★★☆ | ★★★★★ |
| Memory control | ★★★★★ (exact) | ★★★ (GC managed) |
| Memory safety | ★★ (manual) | ★★★★★ (automatic) |
| Startup time | instant | 0.5–2s (GraalVM fixes this) |
| Ecosystem | OS/embedded | Enterprise/web/Android |
| Team scalability | hard | easy (type safety helps) |

### Same task — lines of code comparison

```c
// C — a safe string-split function: ~50 lines
char** split(const char* str, char delim, int* count) {
    // Count delimiters
    *count = 1;
    for (int i = 0; str[i]; i++) if (str[i] == delim) (*count)++;
    char** result = malloc(*count * sizeof(char*));
    // Tokenize, copy each token, null-terminate...
    // Handle allocation failures, edge cases...
    return result;
}
// Plus: caller must free() each element and the array itself
```

```java
// Java — the same operation: 1 line
String[] parts = "a,b,c,d".split(",");
// GC handles cleanup. No manual memory. No buffer overflow possible.
```

### When C is the right choice

```
Use C when:
├── Writing an OS kernel, device driver, or firmware
├── Targeting a microcontroller with <64KB RAM (no room for JVM)
├── Need sub-millisecond latency with zero GC pauses
├── Interfacing directly with hardware registers
└── Building a language runtime or JVM itself (irony!)
```

### When Java is the right choice

```
Use Java when:
├── Building a web backend, REST API, or microservice
├── Enterprise systems (banking, insurance, government)
├── Android mobile apps
├── Large teams where maintainability > raw performance
├── Need rich ecosystem (Spring, Hibernate, Kafka, Spark...)
└── "Works on every platform" is a hard requirement
```

---

## 11. Running the Examples

All runnable examples are in `examples/history/`. Compile and run with:

```bash
# Compile a single file (Java 8+)
javac examples/history/Era01_Foundation.java
java  -cp examples/history Era01_Foundation

# Compile all examples
javac examples/history/*.java

# Run Java 21 examples (needs --enable-preview on some features)
javac --enable-preview --release 21 examples/history/Era09_Java21.java
java  --enable-preview -cp examples/history Era09_Java21
```

**Example files created:**

| File | Topics |
|------|--------|
| `Era01_Foundation.java` | HelloWorld, arrays, basic OOP |
| `Era02_Java12_Collections.java` | Vector vs ArrayList, basic generics |
| `Era03_Java5_Features.java` | Generics, enums, autoboxing, varargs |
| `Era04_Java7_Features.java` | try-with-resources, multi-catch, diamond |
| `Era05_Java8_Lambda.java` | Lambda, streams, Optional |
| `Era06_Java8_Functional.java` | Function, Predicate, Supplier, Consumer |
| `Era07_Java11_Strings.java` | strip, isBlank, repeat, lines |
| `Era08_Java16_Records.java` | Records, compact constructors |
| `Era09_Java21_Patterns.java` | Pattern matching, sealed classes, virtual threads |

---

## Version Timeline

```
1991 ─── Oak project starts (James Gosling, Sun Microsystems)
1995 ─── Java 1.0 released  — "Write Once, Run Anywhere"
1997 ─── Java 1.1           — Inner classes, JDBC, Reflection
1998 ─── Java 1.2 "Java 2"  — Collections Framework                  ← impact
2004 ─── Java 5             — Generics, Enum, Autoboxing, Annotations ← BIG
2006 ─── Java 6             — Performance improvements
2011 ─── Java 7             — try-with-resources, NIO.2, diamond op
2014 ─── Java 8 LTS         — Lambda, Streams, Optional               ← BIGGEST
2017 ─── Java 9             — Module system (Jigsaw)
2018 ─── Java 10            — var (local type inference)
2018 ─── Java 11 LTS        — String methods, HTTP client
2019 ─── Java 12–13         — Switch expressions (preview)
2020 ─── Java 14            — Records (preview), helpful NPE messages
2021 ─── Java 16            — Records (stable), instanceof patterns
2021 ─── Java 17 LTS        — Sealed classes, pattern matching         ← important
2023 ─── Java 21 LTS        — Virtual threads, sequenced collections   ← NEXT BIG
```
