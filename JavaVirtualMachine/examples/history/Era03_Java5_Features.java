/**
 * Era03_Java5_Features.java — Java 5 (2004) Revolution
 *
 * Compile: javac Era03_Java5_Features.java
 * Run:     java Era03_Java5_Features
 *
 * Topics:
 *  - Generics: type safety at compile time
 *  - Enhanced for-loop
 *  - Autoboxing / Unboxing
 *  - Varargs
 *  - Enum (type-safe constants)
 *  - Annotations
 *  - Static imports
 */

import java.util.*;
import java.util.ArrayList;
import java.util.List;
import java.util.HashMap;
import java.util.Map;

public class Era03_Java5_Features {

    // ─── 1. Generics ──────────────────────────────────────────────────────────

    // Generic class — works for any type T
    static class Box<T> {
        private T value;

        public Box(T value) { this.value = value; }
        public T get() { return value; }
        public void set(T value) { this.value = value; }

        @Override
        public String toString() { return "Box<" + value.getClass().getSimpleName() + ">(" + value + ")"; }
    }

    // Generic method
    static <T extends Comparable<T>> T max(T a, T b) {
        return a.compareTo(b) >= 0 ? a : b;
    }

    // Bounded wildcards
    static double sumList(List<? extends Number> list) {
        double sum = 0;
        for (Number n : list) sum += n.doubleValue();
        return sum;
    }

    static void genericsDemo() {
        System.out.println("=== 1. Generics ===");

        // Type-safe collections
        List<String>  strings  = new ArrayList<>();
        List<Integer> integers = new ArrayList<>();

        strings.add("hello");
        strings.add("world");
        // strings.add(42);   // COMPILE ERROR — caught at compile time, not runtime

        integers.add(1);
        integers.add(2);
        integers.add(3);

        // No cast needed when reading — compiler knows the type
        String first = strings.get(0);  // No (String) cast
        int    sum   = integers.get(0) + integers.get(1);

        System.out.println("strings: " + strings);
        System.out.println("first string: " + first);
        System.out.println("sum: " + sum);

        // Generic class
        Box<String>  strBox = new Box<>("Hello Generics!");
        Box<Integer> intBox = new Box<>(42);
        Box<Double>  dblBox = new Box<>(3.14);

        System.out.println(strBox);
        System.out.println(intBox);
        System.out.println(dblBox);

        // Generic method
        System.out.println("max(3, 7) = " + max(3, 7));
        System.out.println("max(\"apple\", \"banana\") = " + max("apple", "banana"));

        // Bounded wildcard
        List<Integer> ints    = List.of(1, 2, 3, 4, 5);
        List<Double>  doubles = List.of(1.5, 2.5, 3.5);
        System.out.println("sum of ints: "    + sumList(ints));
        System.out.println("sum of doubles: " + sumList(doubles));

        // Generic Map
        Map<String, Integer> wordCount = new HashMap<>();
        wordCount.put("hello", 3);
        wordCount.put("world", 1);
        for (Map.Entry<String, Integer> entry : wordCount.entrySet()) {
            System.out.println("  " + entry.getKey() + " → " + entry.getValue());
        }

        System.out.println();
    }

    // ─── 2. Enhanced For-Loop ─────────────────────────────────────────────────

    static void forEachDemo() {
        System.out.println("=== 2. Enhanced For-Loop ===");

        // Over array
        int[] numbers = {10, 20, 30, 40, 50};
        System.out.print("array: ");
        for (int n : numbers) {
            System.out.print(n + " ");
        }
        System.out.println();

        // Over List
        List<String> names = List.of("Alice", "Bob", "Charlie", "Diana");
        System.out.print("names: ");
        for (String name : names) {
            System.out.print(name + " ");
        }
        System.out.println();

        // Over Map entries
        Map<String, Integer> scores = new LinkedHashMap<>();
        scores.put("Alice", 95);
        scores.put("Bob",   87);
        scores.put("Charlie", 92);

        System.out.println("scores:");
        for (Map.Entry<String, Integer> e : scores.entrySet()) {
            System.out.printf("  %-10s %d%n", e.getKey(), e.getValue());
        }

        System.out.println();
    }

    // ─── 3. Autoboxing / Unboxing ─────────────────────────────────────────────

    static void autoboxingDemo() {
        System.out.println("=== 3. Autoboxing / Unboxing ===");

        // Autoboxing: primitive → wrapper
        Integer a = 42;         // int → Integer (auto)
        Double  d = 3.14;       // double → Double (auto)
        Boolean b = true;       // boolean → Boolean (auto)

        System.out.println("Boxed:    " + a + ", " + d + ", " + b);

        // Unboxing: wrapper → primitive
        int    ia = a;          // Integer → int (auto)
        double id = d;          // Double → double (auto)
        boolean ib = b;         // Boolean → boolean (auto)

        System.out.println("Unboxed:  " + ia + ", " + id + ", " + ib);

        // Used in collections naturally
        List<Integer> list = new ArrayList<>();
        list.add(1);     // autoboxed to Integer(1)
        list.add(2);     // autoboxed to Integer(2)
        list.add(3);
        int total = 0;
        for (int n : list) total += n;   // auto-unboxed
        System.out.println("sum: " + total);

        // WARNING — Integer cache (-128 to 127)
        Integer x = 127;
        Integer y = 127;
        System.out.println("127 == 127 (cached):    " + (x == y));   // true — same cached object

        Integer p = 128;
        Integer q = 128;
        System.out.println("128 == 128 (not cached): " + (p == q));   // false — different objects!
        System.out.println("128.equals(128):          " + p.equals(q)); // true — always use equals()

        System.out.println();
    }

    // ─── 4. Varargs ───────────────────────────────────────────────────────────

    static int sum(int... numbers) {
        int total = 0;
        for (int n : numbers) total += n;
        return total;
    }

    static void log(String level, String format, Object... args) {
        String message = String.format(format, args);
        System.out.printf("[%s] %s%n", level, message);
    }

    static void varargsDemo() {
        System.out.println("=== 4. Varargs ===");

        System.out.println("sum()           = " + sum());
        System.out.println("sum(1)          = " + sum(1));
        System.out.println("sum(1,2,3)      = " + sum(1, 2, 3));
        System.out.println("sum(1,2,3,4,5)  = " + sum(1, 2, 3, 4, 5));

        log("INFO",  "Server started on port %d", 8080);
        log("WARN",  "User %s has %d failed attempts", "alice", 3);
        log("ERROR", "Connection to %s:%d failed", "db.example.com", 5432);

        System.out.println();
    }

    // ─── 5. Enum ──────────────────────────────────────────────────────────────

    enum Direction {
        NORTH, SOUTH, EAST, WEST;

        public Direction opposite() {
            return switch (this) {
                case NORTH -> SOUTH;
                case SOUTH -> NORTH;
                case EAST  -> WEST;
                case WEST  -> EAST;
            };
        }
    }

    enum Planet {
        MERCURY(3.303e+23, 2.4397e6),
        VENUS  (4.869e+24, 6.0518e6),
        EARTH  (5.976e+24, 6.37814e6),
        MARS   (6.421e+23, 3.3972e6);

        private final double mass;    // kg
        private final double radius;  // meters
        private static final double G = 6.67300E-11;

        Planet(double mass, double radius) {
            this.mass = mass;
            this.radius = radius;
        }

        double surfaceGravity() {
            return G * mass / (radius * radius);
        }

        double surfaceWeight(double otherMass) {
            return otherMass * surfaceGravity();
        }
    }

    static void enumDemo() {
        System.out.println("=== 5. Enum ===");

        // Basic enum
        Direction dir = Direction.NORTH;
        System.out.println("Direction: " + dir);
        System.out.println("Opposite:  " + dir.opposite());

        // Enum in switch
        for (Direction d : Direction.values()) {
            System.out.printf("  %-6s → %s%n", d, d.opposite());
        }

        // Enum with fields and methods
        double earthWeight = 75.0; // kg
        double mass = earthWeight / Planet.EARTH.surfaceGravity();
        System.out.println("\nWeight on each planet (for 75kg person):");
        for (Planet p : Planet.values()) {
            System.out.printf("  %-8s %.2f N%n", p, p.surfaceWeight(mass));
        }

        // Enum utilities
        Direction fromString = Direction.valueOf("EAST");
        int ordinal = Direction.NORTH.ordinal(); // 0
        System.out.println("\nfromString: " + fromString);
        System.out.println("ordinal of NORTH: " + ordinal);

        System.out.println();
    }

    // ─── 6. Annotations (built-in ones) ──────────────────────────────────────

    static class Parent {
        public String describe() { return "I am Parent"; }

        @Deprecated
        public void oldMethod() {
            System.out.println("Old way of doing things");
        }
    }

    static class Child extends Parent {
        @Override  // Tells compiler: I intend to override — error if I don't
        public String describe() { return "I am Child"; }
    }

    @SuppressWarnings("deprecation")
    static void annotationsDemo() {
        System.out.println("=== 6. Annotations ===");

        Parent p = new Child();
        System.out.println(p.describe()); // Child.describe() via polymorphism

        // @Deprecated — compiler warns you, but it still runs
        p.oldMethod(); // IDE/compiler shows warning

        System.out.println();
    }

    // ─── main ─────────────────────────────────────────────────────────────────

    public static void main(String[] args) {
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║  Era 03 — Java 5 Revolution (2004)      ║");
        System.out.println("╚══════════════════════════════════════════╝\n");

        genericsDemo();
        forEachDemo();
        autoboxingDemo();
        varargsDemo();
        enumDemo();
        annotationsDemo();

        System.out.println("✓ All Era03 examples complete.");
    }
}
