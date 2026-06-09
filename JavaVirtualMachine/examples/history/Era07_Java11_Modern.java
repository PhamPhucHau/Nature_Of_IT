/**
 * Era07_Java11_Modern.java — Java 7 + 10 + 11 Features
 *
 * Compile: javac Era07_Java11_Modern.java
 * Run:     java Era07_Java11_Modern
 *
 * Topics:
 *  - Java 7: try-with-resources, multi-catch, diamond operator, switch on string
 *  - Java 10: var (local type inference)
 *  - Java 11: strip, isBlank, repeat, lines
 */

import java.util.*;
import java.util.stream.*;

public class Era07_Java11_Modern {

    // ─── 1. Java 7 — try-with-resources ──────────────────────────────────────

    // A resource that implements AutoCloseable
    static class DatabaseConnection implements AutoCloseable {
        String url;
        boolean open;

        DatabaseConnection(String url) {
            this.url = url;
            this.open = true;
            System.out.println("  [DB] Opened connection to " + url);
        }

        void query(String sql) {
            if (!open) throw new IllegalStateException("Connection already closed!");
            System.out.println("  [DB] Executing: " + sql);
        }

        @Override
        public void close() {
            open = false;
            System.out.println("  [DB] Closed connection to " + url);
        }
    }

    static class FileReader implements AutoCloseable {
        String filename;

        FileReader(String filename) {
            System.out.println("  [File] Opened " + filename);
        }

        String read() { return "file contents..."; }

        @Override
        public void close() {
            System.out.println("  [File] Closed " + filename);
        }
    }

    static void tryWithResourcesDemo() {
        System.out.println("=== 1. Try-With-Resources (Java 7) ===");

        // Single resource
        System.out.println("Single resource:");
        try (DatabaseConnection conn = new DatabaseConnection("jdbc:postgresql://localhost/mydb")) {
            conn.query("SELECT * FROM users");
            conn.query("SELECT * FROM orders");
        } // conn.close() called AUTOMATICALLY here, even if exception thrown
        System.out.println("  After try block — connection definitely closed");

        // Multiple resources — closed in reverse order
        System.out.println("\nMultiple resources (closed in reverse order):");
        try (DatabaseConnection conn = new DatabaseConnection("jdbc:mysql://localhost/db");
             FileReader reader = new FileReader("data.csv")) {
            conn.query("INSERT INTO log VALUES ('" + reader.read() + "')");
        } // reader.close() called first, then conn.close()

        // Exception during execution
        System.out.println("\nException handling:");
        try (DatabaseConnection conn = new DatabaseConnection("jdbc:h2://mem")) {
            conn.query("VALID SQL");
            throw new RuntimeException("Something went wrong!");
        } catch (RuntimeException e) {
            System.out.println("  Caught: " + e.getMessage());
        }
        // conn.close() was still called before the catch!

        System.out.println();
    }

    // ─── 2. Java 7 — Multi-catch ─────────────────────────────────────────────

    static void parseAndDivide(String numStr, String denomStr) throws Exception {
        try {
            int num   = Integer.parseInt(numStr);
            int denom = Integer.parseInt(denomStr);
            System.out.println("  " + numStr + " / " + denomStr + " = " + (num / denom));
        }
        // Multi-catch — eliminates duplicate handlers
        catch (NumberFormatException | ArithmeticException e) {
            System.out.println("  Error: " + e.getClass().getSimpleName() + ": " + e.getMessage());
        }
    }

    static void multiCatchDemo() {
        System.out.println("=== 2. Multi-Catch (Java 7) ===");
        try {
            parseAndDivide("10", "2");
            parseAndDivide("abc", "2");  // NumberFormatException
            parseAndDivide("10", "0");   // ArithmeticException
        } catch (Exception e) {
            e.printStackTrace();
        }
        System.out.println();
    }

    // ─── 3. Java 7 — Switch on String ────────────────────────────────────────

    static String describeDay(String day) {
        return switch (day.toUpperCase()) {  // Note: using Java 14+ switch expression here
            case "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY" -> "Weekday";
            case "SATURDAY", "SUNDAY" -> "Weekend";
            default -> "Unknown day: " + day;
        };
    }

    static void switchStringDemo() {
        System.out.println("=== 3. Switch on String (Java 7+) ===");

        String[] days = {"Monday", "Saturday", "wednesday", "SUNDAY", "holiday"};
        for (String day : days) {
            System.out.println("  " + day + " → " + describeDay(day));
        }
        System.out.println();
    }

    // ─── 4. Java 10 — var ────────────────────────────────────────────────────

    static void varDemo() {
        System.out.println("=== 4. var — Local Type Inference (Java 10) ===");

        // Instead of repeating the type, let compiler infer it
        var name    = "Alice";                          // String
        var count   = 42;                               // int
        var pi      = 3.14159;                         // double
        var flag    = true;                             // boolean
        var list    = new ArrayList<String>();          // ArrayList<String>
        var map     = new HashMap<String, Integer>();   // HashMap<String,Integer>

        // var is STILL strongly typed — these would fail:
        // name = 42;    // COMPILE ERROR — name is String
        // count = "x";  // COMPILE ERROR — count is int

        list.add("hello");
        list.add("world");
        map.put("a", 1);
        map.put("b", 2);

        System.out.println("name:  " + name + " (type: " + ((Object)name).getClass().getSimpleName() + ")");
        System.out.println("count: " + count);
        System.out.println("list:  " + list);
        System.out.println("map:   " + map);

        // var in for-each
        System.out.print("for-each: ");
        for (var s : list) {
            System.out.print(s + " ");
        }
        System.out.println();

        // var with complex generics — the main benefit
        var frequencyMap = new HashMap<String, List<Map<String, Integer>>>();
        // vs:
        // HashMap<String, List<Map<String, Integer>>> frequencyMap = new HashMap<String, List<Map<String, Integer>>>();
        System.out.println("Complex generic type inferred: " + frequencyMap.getClass().getSimpleName());

        // RULES — where var CANNOT be used:
        // var field = "hello";      // ❌ class field
        // void method(var x) {}    // ❌ method parameter
        // var arr = {1,2,3};       // ❌ array initializer
        // var nothing = null;      // ❌ null has no type to infer
        System.out.println();
    }

    // ─── 5. Java 11 — String methods ─────────────────────────────────────────

    static void java11StringDemo() {
        System.out.println("=== 5. Java 11 String Methods ===");

        // strip() vs trim() — strip is Unicode-aware
        String withSpaces = "   Hello, World!   ";
        System.out.println("original:        '" + withSpaces + "'");
        System.out.println("trim():          '" + withSpaces.trim() + "'");
        System.out.println("strip():         '" + withSpaces.strip() + "'");
        System.out.println("stripLeading():  '" + withSpaces.stripLeading() + "'");
        System.out.println("stripTrailing(): '" + withSpaces.stripTrailing() + "'");

        // isBlank() — true if empty or only whitespace
        System.out.println("\nisBlank() examples:");
        System.out.println("  '':         " + "".isBlank());           // true
        System.out.println("  '   ':      " + "   ".isBlank());        // true
        System.out.println("  '  a ':     " + "  a ".isBlank());       // false
        System.out.println("  'hello':    " + "hello".isBlank());      // false

        // repeat(n) — repeat a string n times
        System.out.println("\nrepeat() examples:");
        System.out.println("  'abc'.repeat(3):   " + "abc".repeat(3));
        System.out.println("  '-'.repeat(30):    " + "-".repeat(30));
        System.out.println("  'ha'.repeat(5):    " + "ha".repeat(5));

        // lines() — splits into stream of lines
        String multiline = "line one\nline two\nline three\n  indented line  ";
        System.out.println("\nlines() example:");
        multiline.lines()
                 .map(l -> "  [" + l + "]")
                 .forEach(System.out::println);

        // lines() with stream operations
        long nonBlankLines = multiline.lines()
                .filter(l -> !l.isBlank())
                .count();
        System.out.println("Non-blank lines: " + nonBlankLines);

        System.out.println();
    }

    // ─── 6. Java 9 — Collection factory methods ──────────────────────────────

    static void collectionFactoriesDemo() {
        System.out.println("=== 6. Immutable Collections (Java 9) ===");

        // Before Java 9 — verbose
        List<String> oldWay = new ArrayList<>(Arrays.asList("a", "b", "c"));

        // Java 9 — concise factory methods
        List<String>        list = List.of("apple", "banana", "cherry");
        Set<Integer>        set  = Set.of(1, 2, 3, 4, 5);
        Map<String, Integer> map = Map.of("one", 1, "two", 2, "three", 3);

        System.out.println("List:  " + list);
        System.out.println("Set:   " + set);
        System.out.println("Map:   " + map);

        // Immutable — cannot modify
        try {
            list.add("date");  // UnsupportedOperationException
        } catch (UnsupportedOperationException e) {
            System.out.println("Cannot modify List.of() — immutable!");
        }

        // Map.copyOf, List.copyOf — copy from existing
        List<String> mutable   = new ArrayList<>(List.of("x", "y", "z"));
        List<String> immutable = List.copyOf(mutable);
        mutable.add("w");                    // Modifies mutable
        System.out.println("mutable:   " + mutable);
        System.out.println("immutable: " + immutable);  // Not affected

        System.out.println();
    }

    // ─── main ─────────────────────────────────────────────────────────────────

    public static void main(String[] args) {
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║  Era 07 — Java 7/9/10/11 Modern Java    ║");
        System.out.println("╚══════════════════════════════════════════╝\n");

        tryWithResourcesDemo();
        multiCatchDemo();
        switchStringDemo();
        varDemo();
        java11StringDemo();
        collectionFactoriesDemo();

        System.out.println("✓ All Era07 examples complete.");
    }
}
