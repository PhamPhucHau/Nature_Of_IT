/**
 * Era01_Foundation.java — Java 1.0 / 1.1 Basics
 *
 * Compile: javac Era01_Foundation.java
 * Run:     java Era01_Foundation
 *
 * Topics:
 *  - HelloWorld, arrays, basic control flow
 *  - The pain of no generics (raw types)
 *  - Anonymous inner class (ancestor of Lambda)
 *  - Basic exception handling
 *  - String operations
 */

import java.util.Vector;

public class Era01_Foundation {

    // ─── 1. The very beginning ────────────────────────────────────────────────

    static void helloWorld() {
        System.out.println("=== 1. Hello World ===");
        System.out.println("Hello, World!");
        System.out.println();
    }

    // ─── 2. Arrays — safe compared to C ──────────────────────────────────────

    static void arrayDemo() {
        System.out.println("=== 2. Arrays ===");

        int[] numbers = {10, 20, 30, 40, 50};

        // Read
        System.out.println("numbers[0] = " + numbers[0]);
        System.out.println("Length     = " + numbers.length);

        // Safe out-of-bounds — compare to C's silent buffer overflow
        try {
            int bad = numbers[10];  // ArrayIndexOutOfBoundsException
        } catch (ArrayIndexOutOfBoundsException e) {
            System.out.println("Caught safely: " + e.getMessage());
            System.out.println("In C, this would silently corrupt memory!");
        }

        // 2D array
        int[][] grid = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
        System.out.println("grid[1][2] = " + grid[1][2]); // 6

        System.out.println();
    }

    // ─── 3. Vector — the painful pre-Collections era ─────────────────────────

    static void vectorPain() {
        System.out.println("=== 3. Vector (Java 1.0 — painful) ===");

        // No generics — accepts anything, type-unsafe
        Vector v = new Vector();
        v.addElement("hello");
        v.addElement(42);           // Mix of String and Integer — allowed!
        v.addElement(3.14);         // And double too!

        System.out.println("Vector contents (raw):");
        for (int i = 0; i < v.size(); i++) {
            System.out.println("  [" + i + "] = " + v.get(i)
                    + " (type: " + v.get(i).getClass().getSimpleName() + ")");
        }

        // The danger: ClassCastException at runtime (not compile time!)
        try {
            String s = (String) v.get(1);   // 42 is NOT a String
        } catch (ClassCastException e) {
            System.out.println("ClassCastException caught: " + e.getMessage());
            System.out.println("This was a RUNTIME error — Java 5 Generics fixed it.");
        }

        System.out.println();
    }

    // ─── 4. Anonymous Inner Class (Java 1.1) — ancestor of Lambda ────────────

    interface Greeting {
        void greet(String name);
    }

    static void anonymousInnerClass() {
        System.out.println("=== 4. Anonymous Inner Class (Java 1.1) ===");

        // Create a "function" by implementing an interface inline
        // This is VERBOSE — Java 8 Lambda fixes this in 1 line
        Greeting formal = new Greeting() {
            @Override
            public void greet(String name) {
                System.out.println("Good day, " + name + ".");
            }
        };

        Greeting casual = new Greeting() {
            @Override
            public void greet(String name) {
                System.out.println("Hey " + name + "!");
            }
        };

        formal.greet("Sir Arthur");
        casual.greet("Bob");

        System.out.println("  (Java 8 version:  name -> \"Hey \" + name + \"!\")");
        System.out.println();
    }

    // ─── 5. Basic OOP ─────────────────────────────────────────────────────────

    static abstract class Shape {
        abstract double area();
        abstract String name();

        @Override
        public String toString() {
            return name() + " with area=" + String.format("%.2f", area());
        }
    }

    static class Circle extends Shape {
        private final double radius;
        Circle(double radius) { this.radius = radius; }

        @Override double area() { return Math.PI * radius * radius; }
        @Override String name() { return "Circle(r=" + radius + ")"; }
    }

    static class Rectangle extends Shape {
        private final double w, h;
        Rectangle(double w, double h) { this.w = w; this.h = h; }

        @Override double area() { return w * h; }
        @Override String name() { return "Rectangle(" + w + "x" + h + ")"; }
    }

    static void oopDemo() {
        System.out.println("=== 5. OOP — Polymorphism ===");

        Shape[] shapes = {
            new Circle(5),
            new Rectangle(4, 6),
            new Circle(3),
        };

        for (Shape s : shapes) {
            System.out.println("  " + s);  // Calls the correct area() via polymorphism
        }

        System.out.println();
    }

    // ─── 6. String operations ─────────────────────────────────────────────────

    static void stringDemo() {
        System.out.println("=== 6. String Operations ===");

        String s = "  Hello, Java World!  ";
        System.out.println("Original:   '" + s + "'");
        System.out.println("trimmed:    '" + s.trim() + "'");
        System.out.println("upper:      '" + s.trim().toUpperCase() + "'");
        System.out.println("contains 'Java': " + s.contains("Java"));
        System.out.println("replace:    '" + s.trim().replace("Java", "C") + "'");
        System.out.println("split on space: ");

        for (String word : s.trim().split("\\s+")) {
            System.out.println("  - '" + word + "'");
        }

        // String is immutable — each operation creates a new object
        String a = "hello";
        String b = a.toUpperCase();
        System.out.println("a=" + a + ", b=" + b);  // a unchanged

        System.out.println();
    }

    // ─── 7. Basic exception handling ─────────────────────────────────────────

    static int safeDivide(int a, int b) {
        if (b == 0) throw new ArithmeticException("Cannot divide by zero");
        return a / b;
    }

    static void exceptionDemo() {
        System.out.println("=== 7. Exception Handling ===");

        // Try-catch-finally
        try {
            System.out.println("10 / 2 = " + safeDivide(10, 2));
            System.out.println("10 / 0 = " + safeDivide(10, 0));  // throws
        } catch (ArithmeticException e) {
            System.out.println("Caught: " + e.getMessage());
        } finally {
            System.out.println("Finally block always runs");
        }

        // Checked vs unchecked
        // ArithmeticException = unchecked (extends RuntimeException)
        // IOException = checked (must catch or declare throws)

        System.out.println();
    }

    // ─── main ─────────────────────────────────────────────────────────────────

    public static void main(String[] args) {
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║  Era 01 — Java Foundation (1.0 / 1.1)   ║");
        System.out.println("╚══════════════════════════════════════════╝\n");

        helloWorld();
        arrayDemo();
        vectorPain();
        anonymousInnerClass();
        oopDemo();
        stringDemo();
        exceptionDemo();

        System.out.println("✓ All Era01 examples complete.");
    }
}
