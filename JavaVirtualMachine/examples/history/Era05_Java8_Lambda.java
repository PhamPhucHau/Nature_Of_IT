/**
 * Era05_Java8_Lambda.java — Java 8 (2014) Lambda & Streams
 *
 * Compile: javac Era05_Java8_Lambda.java
 * Run:     java Era05_Java8_Lambda
 *
 * Topics:
 *  - Lambda expressions
 *  - Method references
 *  - Stream API (filter, map, reduce, collect)
 *  - Optional
 *  - Default methods in interfaces
 *  - Built-in functional interfaces
 */

import java.util.*;
import java.util.stream.*;
import java.util.function.*;

public class Era05_Java8_Lambda {

    // ─── 1. Lambda Basics ─────────────────────────────────────────────────────

    @FunctionalInterface
    interface MathOperation {
        int operate(int a, int b);
    }

    @FunctionalInterface
    interface StringTransform {
        String transform(String s);
    }

    static void lambdaBasics() {
        System.out.println("=== 1. Lambda Basics ===");

        // Before Java 8 — anonymous inner class
        MathOperation addOld = new MathOperation() {
            @Override
            public int operate(int a, int b) { return a + b; }
        };

        // Java 8 — lambda (parameters) -> expression
        MathOperation add      = (a, b) -> a + b;
        MathOperation subtract = (a, b) -> a - b;
        MathOperation multiply = (a, b) -> a * b;
        MathOperation divide   = (a, b) -> {
            if (b == 0) throw new ArithmeticException("divide by zero");
            return a / b;
        };

        System.out.println("add(5, 3)      = " + add.operate(5, 3));
        System.out.println("subtract(5, 3) = " + subtract.operate(5, 3));
        System.out.println("multiply(5, 3) = " + multiply.operate(5, 3));
        System.out.println("divide(9, 3)   = " + divide.operate(9, 3));

        // Lambda with single parameter (no parentheses needed)
        StringTransform upper = s -> s.toUpperCase();
        StringTransform rev   = s -> new StringBuilder(s).reverse().toString();

        System.out.println("upper(\"hello\") = " + upper.transform("hello"));
        System.out.println("rev(\"hello\")   = " + rev.transform("hello"));

        System.out.println();
    }

    // ─── 2. Method References ─────────────────────────────────────────────────

    static void printItem(String item) {
        System.out.println("  → " + item);
    }

    static int compareByLength(String a, String b) {
        return Integer.compare(a.length(), b.length());
    }

    static void methodReferences() {
        System.out.println("=== 2. Method References ===");

        List<String> names = new ArrayList<>(List.of("Charlie", "Alice", "Bob", "Diana"));

        // Static method reference: ClassName::methodName
        names.forEach(Era05_Java8_Lambda::printItem);

        // Instance method reference on an object: object::methodName
        String prefix = "Hello, ";
        Function<String, String> greet = prefix::concat;
        System.out.println(greet.apply("Alice"));

        // Instance method reference on a type: ClassName::methodName
        names.sort(String::compareTo);         // same as (a, b) -> a.compareTo(b)
        System.out.println("sorted:     " + names);

        names.sort(Era05_Java8_Lambda::compareByLength);  // static method ref
        System.out.println("by length:  " + names);

        // Constructor reference: ClassName::new
        Supplier<ArrayList<String>> listFactory = ArrayList::new;
        ArrayList<String> newList = listFactory.get();
        newList.add("created via constructor reference");
        System.out.println(newList);

        System.out.println();
    }

    // ─── 3. Stream API ────────────────────────────────────────────────────────

    static void streamBasics() {
        System.out.println("=== 3. Stream — Basic Operations ===");

        List<Integer> numbers = List.of(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

        // filter — keep elements matching a predicate
        List<Integer> evens = numbers.stream()
                .filter(n -> n % 2 == 0)
                .collect(Collectors.toList());
        System.out.println("evens:         " + evens);

        // map — transform each element
        List<Integer> squares = numbers.stream()
                .map(n -> n * n)
                .collect(Collectors.toList());
        System.out.println("squares:       " + squares);

        // filter + map chained
        List<Integer> evenSquares = numbers.stream()
                .filter(n -> n % 2 == 0)
                .map(n -> n * n)
                .collect(Collectors.toList());
        System.out.println("even squares:  " + evenSquares);

        // reduce — combine all elements into one value
        int sum = numbers.stream()
                .reduce(0, Integer::sum);  // 0 is identity
        System.out.println("sum:           " + sum);

        // count
        long countEvens = numbers.stream()
                .filter(n -> n % 2 == 0)
                .count();
        System.out.println("count evens:   " + countEvens);

        // min / max
        Optional<Integer> max = numbers.stream().max(Integer::compareTo);
        Optional<Integer> min = numbers.stream().min(Integer::compareTo);
        System.out.println("max:           " + max.orElse(-1));
        System.out.println("min:           " + min.orElse(-1));

        // anyMatch / allMatch / noneMatch
        System.out.println("any > 5:       " + numbers.stream().anyMatch(n -> n > 5));
        System.out.println("all > 0:       " + numbers.stream().allMatch(n -> n > 0));
        System.out.println("none > 100:    " + numbers.stream().noneMatch(n -> n > 100));

        System.out.println();
    }

    static void streamCollectors() {
        System.out.println("=== 3b. Stream — Collectors ===");

        List<String> words = List.of("apple", "banana", "cherry", "avocado", "blueberry", "apricot");

        // joining
        String joined = words.stream().collect(Collectors.joining(", "));
        System.out.println("joined:       " + joined);

        // groupingBy
        Map<Character, List<String>> byFirstLetter = words.stream()
                .collect(Collectors.groupingBy(w -> w.charAt(0)));
        System.out.println("by letter:    " + byFirstLetter);

        // counting per group
        Map<Character, Long> countByLetter = words.stream()
                .collect(Collectors.groupingBy(w -> w.charAt(0), Collectors.counting()));
        System.out.println("count/letter: " + countByLetter);

        // partitioningBy
        Map<Boolean, List<String>> partition = words.stream()
                .collect(Collectors.partitioningBy(w -> w.startsWith("a")));
        System.out.println("starts w/ a:  " + partition.get(true));
        System.out.println("others:       " + partition.get(false));

        // toMap
        Map<String, Integer> wordLengths = words.stream()
                .collect(Collectors.toMap(w -> w, String::length));
        System.out.println("word lengths: " + wordLengths);

        // statistics
        IntSummaryStatistics stats = words.stream()
                .mapToInt(String::length)
                .summaryStatistics();
        System.out.printf("lengths — min:%d, max:%d, avg:%.1f%n",
                stats.getMin(), stats.getMax(), stats.getAverage());

        System.out.println();
    }

    // ─── 4. Optional ─────────────────────────────────────────────────────────

    static class User {
        String name;
        Address address;
        User(String name, Address address) { this.name = name; this.address = address; }
    }

    static class Address {
        City city;
        Address(City city) { this.city = city; }
    }

    static class City {
        String name;
        City(String name) { this.name = name; }
    }

    static void optionalDemo() {
        System.out.println("=== 4. Optional ===");

        // Creating Optionals
        Optional<String> present = Optional.of("hello");
        Optional<String> empty   = Optional.empty();
        Optional<String> nullable= Optional.ofNullable(null);

        System.out.println("present.isPresent():  " + present.isPresent());
        System.out.println("empty.isPresent():    " + empty.isPresent());
        System.out.println("empty.isEmpty():      " + empty.isEmpty());

        // Safely get
        System.out.println("present.get():        " + present.get());
        System.out.println("empty.orElse(\"def\"): " + empty.orElse("default"));
        System.out.println("empty.orElseGet:      " + empty.orElseGet(() -> "computed default"));

        // Optional chaining — the real power
        User userWithCity = new User("Alice", new Address(new City("San Francisco")));
        User userNoAddress = new User("Bob", null);
        User noUser = null;

        // Without Optional — null checks everywhere
        String city1 = (userWithCity != null && userWithCity.address != null
                && userWithCity.address.city != null)
                ? userWithCity.address.city.name : "Unknown";
        System.out.println("city (old way): " + city1);

        // With Optional — clean chain
        String city2 = Optional.ofNullable(userWithCity)
                .map(u -> u.address)
                .map(a -> a.city)
                .map(c -> c.name)
                .orElse("Unknown");

        String city3 = Optional.ofNullable(userNoAddress)
                .map(u -> u.address)
                .map(a -> a.city)
                .map(c -> c.name)
                .orElse("Unknown");

        System.out.println("city (Optional, has city): " + city2);
        System.out.println("city (Optional, no addr):  " + city3);

        // ifPresent / ifPresentOrElse
        present.ifPresent(s -> System.out.println("Value present: " + s));
        empty.ifPresentOrElse(
            s -> System.out.println("has: " + s),
            () -> System.out.println("empty — use fallback")
        );

        // filter on Optional
        Optional<String> longWord = present.filter(s -> s.length() > 3);
        Optional<String> shortWord = present.filter(s -> s.length() < 3);
        System.out.println("longWord: " + longWord);    // Optional[hello]
        System.out.println("shortWord: " + shortWord);   // Optional.empty

        System.out.println();
    }

    // ─── 5. Functional Interfaces ─────────────────────────────────────────────

    static void functionalInterfaces() {
        System.out.println("=== 5. Built-in Functional Interfaces ===");

        // Function<T,R>: takes T, returns R
        Function<String, Integer> length = String::length;
        Function<Integer, String> intToStr = i -> "number:" + i;
        Function<String, String> lengthStr = length.andThen(intToStr); // compose

        System.out.println("length(\"hello\"):    " + length.apply("hello"));
        System.out.println("composed:            " + lengthStr.apply("hello"));

        // Predicate<T>: takes T, returns boolean
        Predicate<String> notEmpty    = s -> !s.isEmpty();
        Predicate<String> longEnough  = s -> s.length() >= 5;
        Predicate<String> valid       = notEmpty.and(longEnough);
        Predicate<String> either      = notEmpty.or(s -> s.equals("x"));
        Predicate<String> invalid     = valid.negate();

        System.out.println("valid(\"\"):           " + valid.test(""));
        System.out.println("valid(\"hello\"):      " + valid.test("hello"));
        System.out.println("valid(\"hi\"):         " + valid.test("hi"));

        // Consumer<T>: takes T, returns void
        Consumer<String> print    = System.out::println;
        Consumer<String> printUpper = s -> System.out.println(s.toUpperCase());
        Consumer<String> both     = print.andThen(printUpper);

        System.out.print("both consumer: ");
        both.accept("hello");

        // Supplier<T>: takes nothing, returns T
        Supplier<Double> random = Math::random;
        Supplier<List<String>> emptyList = ArrayList::new;

        System.out.println("random:    " + String.format("%.4f", random.get()));
        System.out.println("emptyList: " + emptyList.get());

        // BiFunction<T,U,R>
        BiFunction<String, String, String> concat = (a, b) -> a + " " + b;
        BiFunction<Integer, Integer, Integer> multiply = (a, b) -> a * b;

        System.out.println("concat:    " + concat.apply("Hello", "World"));
        System.out.println("multiply:  " + multiply.apply(6, 7));

        // UnaryOperator<T> — specialization of Function<T,T>
        UnaryOperator<String> trim = String::trim;
        UnaryOperator<Integer> increment = n -> n + 1;

        // BinaryOperator<T> — specialization of BiFunction<T,T,T>
        BinaryOperator<Integer> add = Integer::sum;
        BinaryOperator<String> longest = (a, b) -> a.length() >= b.length() ? a : b;

        System.out.println("trim:      '" + trim.apply("  hello  ") + "'");
        System.out.println("add:       " + add.apply(3, 4));
        System.out.println("longest:   " + longest.apply("apple", "banana"));

        System.out.println();
    }

    // ─── 6. Default methods ───────────────────────────────────────────────────

    interface Drawable {
        void draw();  // abstract — must implement

        default void drawTwice() {   // default — inherited for free
            draw();
            draw();
        }

        default String describe() {
            return "A drawable shape";
        }

        static Drawable of(String type) {  // static factory
            return switch (type) {
                case "circle"    -> () -> System.out.println("○ circle");
                case "square"    -> () -> System.out.println("□ square");
                case "triangle"  -> () -> System.out.println("△ triangle");
                default          -> () -> System.out.println("? unknown");
            };
        }
    }

    static void defaultMethodsDemo() {
        System.out.println("=== 6. Default Methods in Interfaces ===");

        Drawable circle = Drawable.of("circle");
        Drawable square = Drawable.of("square");

        circle.draw();
        circle.drawTwice();
        System.out.println(circle.describe());

        List<Drawable> shapes = List.of(
            Drawable.of("circle"),
            Drawable.of("square"),
            Drawable.of("triangle")
        );
        shapes.forEach(Drawable::draw);  // method reference to abstract method

        System.out.println();
    }

    // ─── main ─────────────────────────────────────────────────────────────────

    public static void main(String[] args) {
        System.out.println("╔══════════════════════════════════════════╗");
        System.out.println("║  Era 05 — Java 8 Lambda & Streams       ║");
        System.out.println("╚══════════════════════════════════════════╝\n");

        lambdaBasics();
        methodReferences();
        streamBasics();
        streamCollectors();
        optionalDemo();
        functionalInterfaces();
        defaultMethodsDemo();

        System.out.println("✓ All Era05 examples complete.");
    }
}
