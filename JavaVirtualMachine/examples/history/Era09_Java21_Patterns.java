/**
 * Era09_Java21_Patterns.java — Java 16–21: Records, Sealed, Pattern Matching
 *
 * Compile: javac --enable-preview --release 21 Era09_Java21_Patterns.java
 * Run:     java  --enable-preview Era09_Java21_Patterns
 *
 * NOTE: If you're on Java 16+, most features here work without --enable-preview.
 * Try: javac Era09_Java21_Patterns.java first.
 *
 * Topics:
 *  - Records (Java 16)
 *  - Sealed classes (Java 17)
 *  - Pattern matching instanceof (Java 16)
 *  - Pattern matching switch (Java 21)
 *  - Text blocks (Java 15)
 *  - Virtual threads concept
 */

import java.util.*;
import java.util.stream.*;

public class Era09_Java21_Patterns {

    // ─── 1. Records (Java 16) ─────────────────────────────────────────────────

    // A record is an immutable data carrier — auto-generates:
    // constructor, accessors, equals(), hashCode(), toString()
    record Point(double x, double y) {
        // Compact constructor — validation before fields are assigned
        Point {
            if (Double.isNaN(x) || Double.isNaN(y))
                throw new IllegalArgumentException("Coordinates cannot be NaN");
        }

        // Custom instance methods
        double distanceTo(Point other) {
            double dx = this.x - other.x;
            double dy = this.y - other.y;
            return Math.sqrt(dx * dx + dy * dy);
        }

        Point translate(double dx, double dy) {
            return new Point(x + dx, y + dy);
        }
    }

    record Person(String firstName, String lastName, int age) {
        // Derived accessor
        String fullName() { return firstName + " " + lastName; }

        // Static factory
        static Person of(String fullName, int age) {
            String[] parts = fullName.split(" ", 2);
            return new Person(parts[0], parts.length > 1 ? parts[1] : "", age);
        }
    }

    // Records work great as Map keys (stable hashCode/equals)
    record Coordinate(int row, int col) {}

    static void recordsDemo() {
        System.out.println("=== 1. Records (Java 16) ===");

        Point p1 = new Point(0, 0);
        Point p2 = new Point(3, 4);
        Point p3 = new Point(3, 4);

        System.out.println("p1: " + p1);            // Point[x=0.0, y=0.0]
        System.out.println("p2: " + p2);            // Point[x=3.0, y=4.0]
        System.out.println("p2 == p3: " + (p2 == p3));        // false (different objects)
        System.out.println("p2.equals(p3): " + p2.equals(p3)); // true (value equality)
        System.out.printf("distance p1→p2: %.2f%n", p1.distanceTo(p2)); // 5.00

        Point moved = p1.translate(1.5, 2.5);
        System.out.println("translated: " + moved);

        // Records as data carriers
        List<Person> people = List.of(
            new Person("Alice", "Smith", 30),
            Person.of("Bob Jones", 25),
            new Person("Charlie", "Brown", 35)
        );

        people.stream()
              .sorted(Comparator.comparing(Person::age))
              .forEach(p -> System.out.printf("  %s (age %d)%n", p.fullName(), p.age()));

        // Records as Map keys
        var grid = new HashMap<Coordinate, String>();
        grid.put(new Coordinate(0, 0), "start");
        grid.put(new Coordinate(3, 4), "treasure");
        System.out.println("grid[0,0]: " + grid.get(new Coordinate(0, 0))); // "start"

        // Destructuring in instanceof (Java 16+)
        Object obj = new Point(1.0, 2.0);
        if (obj instanceof Point(var x, var y)) {
            System.out.println("Destructured: x=" + x + ", y=" + y);
        }

        System.out.println();
    }

    // ─── 2. Sealed Classes (Java 17) ──────────────────────────────────────────

    sealed interface Shape permits Circle, Rectangle, Triangle {
        double area();
        double perimeter();
        String name();
    }

    record Circle(double radius) implements Shape {
        public double area()      { return Math.PI * radius * radius; }
        public double perimeter() { return 2 * Math.PI * radius; }
        public String name()      { return "Circle"; }
    }

    record Rectangle(double width, double height) implements Shape {
        public double area()      { return width * height; }
        public double perimeter() { return 2 * (width + height); }
        public String name()      { return "Rectangle"; }
    }

    record Triangle(double a, double b, double c) implements Shape {
        Triangle {  // Validate triangle inequality
            if (a + b <= c || a + c <= b || b + c <= a)
                throw new IllegalArgumentException("Invalid triangle sides");
        }
        public double area() {
            double s = (a + b + c) / 2;   // semi-perimeter
            return Math.sqrt(s * (s-a) * (s-b) * (s-c));  // Heron's formula
        }
        public double perimeter() { return a + b + c; }
        public String name()      { return "Triangle"; }
    }

    static void sealedClassesDemo() {
        System.out.println("=== 2. Sealed Classes (Java 17) ===");

        List<Shape> shapes = List.of(
            new Circle(5),
            new Rectangle(4, 6),
            new Triangle(3, 4, 5)
        );

        System.out.println("Shapes:");
        for (Shape s : shapes) {
            System.out.printf("  %-10s area=%.2f  perimeter=%.2f%n",
                s.name(), s.area(), s.perimeter());
        }

        // Total area using Stream
        double totalArea = shapes.stream()
            .mapToDouble(Shape::area)
            .sum();
        System.out.printf("Total area: %.2f%n", totalArea);

        System.out.println();
    }

    // ─── 3. Pattern Matching — instanceof (Java 16) ──────────────────────────

    static void patternMatchingInstanceof() {
        System.out.println("=== 3. Pattern Matching instanceof (Java 16) ===");

        Object[] objects = {"hello", 42, 3.14, true, List.of(1,2,3), null};

        for (Object obj : objects) {
            // Old way:
            // if (obj instanceof String) { String s = (String) obj; ... }

            // New way — pattern variable bound directly:
            if      (obj instanceof String  s) System.out.println("String:  '" + s.toUpperCase() + "' (len=" + s.length() + ")");
            else if (obj instanceof Integer i) System.out.println("Integer: " + i + " (doubled=" + (i*2) + ")");
            else if (obj instanceof Double  d) System.out.println("Double:  " + d + " (rounded=" + Math.round(d) + ")");
            else if (obj instanceof Boolean b) System.out.println("Boolean: " + b + " (negated=" + !b + ")");
            else if (obj instanceof List<?> l) System.out.println("List:    size=" + l.size());
            else                               System.out.println("null or unknown");
        }

        // Pattern matching with guard condition
        Object value = 42;
        if (value instanceof Integer i && i > 0) {
            System.out.println("Positive integer: " + i);
        }

        System.out.println();
    }

    // ─── 4. Pattern Matching switch (Java 21) ────────────────────────────────

    static String describeShape(Shape shape) {
        // Exhaustive switch — compiler knows all sealed subtypes
        return switch (shape) {
            case Circle    c -> String.format("Circle with radius %.1f, area %.2f",
                                              c.radius(), c.area());
            case Rectangle r -> String.format("Rectangle %s×%s, area %.2f",
                                              r.width(), r.height(), r.area());
            case Triangle  t -> String.format("Triangle sides %.0f,%.0f,%.0f, area %.2f",
                                              t.a(), t.b(), t.c(), t.area());
            // No default needed! Compiler verifies exhaustiveness.
        };
    }

    static String classify(Object obj) {
        return switch (obj) {
            case null                        -> "null";
            case Integer i when i < 0        -> "negative int: " + i;
            case Integer i when i == 0       -> "zero";
            case Integer i                   -> "positive int: " + i;
            case String  s when s.isBlank()  -> "blank string";
            case String  s                   -> "string: '" + s + "'";
            case Double  d                   -> "double: " + d;
            case int[]   arr                 -> "int array of length " + arr.length;
            default                          -> "other: " + obj.getClass().getSimpleName();
        };
    }

    static void patternMatchingSwitch() {
        System.out.println("=== 4. Pattern Matching switch (Java 21) ===");

        List<Shape> shapes = List.of(
            new Circle(3),
            new Rectangle(4, 5),
            new Triangle(3, 4, 5)
        );

        shapes.forEach(s -> System.out.println("  " + describeShape(s)));

        System.out.println("\nclassify examples:");
        Object[] samples = {null, -5, 0, 42, "hello", "  ", 3.14, new int[]{1,2,3}};
        for (Object sample : samples) {
            System.out.println("  " + classify(sample));
        }

        System.out.println();
    }

    // ─── 5. Text Blocks (Java 15) ─────────────────────────────────────────────

    static void textBlocksDemo() {
        System.out.println("=== 5. Text Blocks (Java 15) ===");

        // Before text blocks — string concatenation nightmare
        String jsonOld = "{\n" +
            "  \"name\": \"Alice\",\n" +
            "  \"age\": 30,\n" +
            "  \"city\": \"San Francisco\"\n" +
            "}";

        // With text blocks — natural formatting
        String json = """
                {
                  "name": "Alice",
                  "age": 30,
                  "city": "San Francisco"
                }
                """;

        String html = """
                <html>
                    <body>
                        <h1>Hello, World!</h1>
                    </body>
                </html>
                """;

        String sql = """
                SELECT u.name, o.total
                FROM   users u
                JOIN   orders o ON u.id = o.user_id
                WHERE  o.total > 100
                ORDER BY o.total DESC
                """;

        System.out.println("JSON text block:");
        System.out.println(json);

        System.out.println("HTML text block:");
        System.out.println(html);

        System.out.println("SQL text block:");
        System.out.println(sql);
    }

    // ─── 6. Virtual Threads concept demo (Java 21) ───────────────────────────

    static void virtualThreadsDemo() throws InterruptedException {
        System.out.println("=== 6. Virtual Threads (Java 21) ===");
        System.out.println("(Simulating the concept — actual virtual thread code shown below)");

        // Concept: Platform threads are expensive (~1MB each)
        // Virtual threads are cheap (~few KB each)
        // Same API — just a different ExecutorService

        // Platform threads — limited by memory/OS
        System.out.println("Platform thread example (up to ~10,000 before OOM):");
        System.out.println("  Thread.ofPlatform().start(() -> doWork());");
        System.out.println("  ExecutorService = Executors.newFixedThreadPool(200);");

        System.out.println("\nVirtual thread example (can have millions):");
        System.out.println("  Thread.ofVirtual().start(() -> doWork());");
        System.out.println("  ExecutorService = Executors.newVirtualThreadPerTaskExecutor();");

        // Demo: creating actual virtual threads (Java 21)
        // Uncomment if running on Java 21:
        /*
        try (var executor = Executors.newVirtualThreadPerTaskExecutor()) {
            var futures = IntStream.range(0, 100_000)
                .mapToObj(i -> executor.submit(() -> {
                    Thread.sleep(100);  // Simulate I/O
                    return i;
                }))
                .toList();

            long count = futures.stream().mapToLong(f -> {
                try { return f.get(); } catch (Exception e) { return -1; }
            }).count();

            System.out.println("Completed " + count + " virtual thread tasks");
        }
        */

        // Simple virtual thread demo (works on Java 21)
        System.out.println("\nCreating 5 virtual threads:");
        List<Thread> threads = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            final int id = i;
            Thread vt = Thread.ofVirtual()
                .name("vt-" + id)
                .start(() -> System.out.println("  Virtual thread " + id + " running: " + Thread.currentThread()));
            threads.add(vt);
        }
        for (Thread t : threads) t.join();

        System.out.println();
    }

    // ─── main ─────────────────────────────────────────────────────────────────

    public static void main(String[] args) throws InterruptedException {
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║  Era 09 — Java 16–21 Modern Features    ║");
        System.out.println("╚══════════════════════════════════════════╝\n");

        recordsDemo();
        sealedClassesDemo();
        patternMatchingInstanceof();
        patternMatchingSwitch();
        textBlocksDemo();
        virtualThreadsDemo();

        System.out.println("✓ All Era09 examples complete.");
    }
}
