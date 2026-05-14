# JVM Test Cases

## Overview

This document contains test cases for the Java Virtual Machine implementation.

---

## Test Categories

### 1. Class File Parsing Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-CP-001 | Valid class file | Hello.class | Parse successfully | ✅ |
| TC-CP-002 | Invalid magic number | Invalid.class | Return error | ✅ |
| TC-CP-003 | Empty file | Empty.class | Return error | ✅ |
| TC-CP-004 | Truncated file | Truncated.class | Return error | ✅ |
| TC-CP-005 | Constant pool parsing | Test.class | Parse all entries | ✅ |

### 2. Memory Management Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-MM-001 | Heap allocation | Allocate 100 bytes | Return valid pointer | ✅ |
| TC-MM-002 | Heap overflow | Allocate too much | Return NULL | ✅ |
| TC-MM-003 | Multiple allocations | Allocate 10 times | All succeed | ✅ |
| TC-MM-004 | Memory zeroing | Allocate and read | All zeros | ✅ |

### 3. Stack Operations Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-ST-001 | Push operation | Push 42 | Stack size = 1 | ✅ |
| TC-ST-002 | Pop operation | Push then pop | Value = 42 | ✅ |
| TC-ST-003 | Stack underflow | Pop empty stack | Return 0 | ✅ |
| TC-ST-004 | Stack overflow | Push 1025 times | Last push fails | ✅ |
| TC-ST-005 | Stack empty check | Check empty stack | Return true | ✅ |

### 4. Bytecode Instruction Tests

#### Load/Store Instructions

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-IS-001 | iload_0 | Load local var 0 | Value on stack | ✅ |
| TC-IS-002 | iload_1 | Load local var 1 | Value on stack | ✅ |
| TC-IS-003 | istore_0 | Store to local var 0 | Value in local vars | ✅ |
| TC-IS-004 | bipush | Push byte value | Value on stack | ✅ |
| TC-IS-005 | sipush | Push short value | Value on stack | ✅ |
| TC-IS-006 | iconst_0 | Push 0 | 0 on stack | ✅ |
| TC-IS-007 | iconst_1 | Push 1 | 1 on stack | ✅ |

#### Arithmetic Instructions

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-AR-001 | iadd | 5 + 3 | 8 on stack | ✅ |
| TC-AR-002 | isub | 10 - 3 | 7 on stack | ✅ |
| TC-AR-003 | imul | 5 * 3 | 15 on stack | ✅ |
| TC-AR-004 | idiv | 10 / 3 | 3 on stack | ✅ |
| TC-AR-005 | irem | 10 % 3 | 1 on stack | ✅ |
| TC-AR-006 | ineg | -5 | 5 on stack | ✅ |

#### Control Instructions

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-CT-001 | if_icmpeq | 5 == 5 | Branch taken | ✅ |
| TC-CT-002 | if_icmpne | 5 != 3 | Branch taken | ✅ |
| TC-CT-003 | if_icmplt | 3 < 5 | Branch taken | ✅ |
| TC-CT-004 | if_icmpge | 5 >= 5 | Branch taken | ✅ |
| TC-CT-005 | goto | Unconditional branch | Branch taken | ✅ |

### 5. Object Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-OB-001 | Object creation | new instruction | Object allocated | ✅ |
| TC-OB-002 | Field access | getfield | Field value | ✅ |
| TC-OB-003 | Field assignment | putfield | Field updated | ✅ |

### 6. Method Invocation Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-MV-001 | Static method | invokestatic | Method executed | ✅ |
| TC-MV-002 | Instance method | invokevirtual | Method executed | ✅ |
| TC-MV-003 | Method with parameters | Pass args | Parameters set | ✅ |
| TC-MV-004 | Method return | return instruction | Return value | ✅ |

### 7. Exception Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-EX-001 | Try-catch block | Exception thrown | Handler executed | ✅ |
| TC-EX-002 | Exception table | Match exception | Correct handler | ✅ |

### 8. Integration Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-IN-001 | Hello World | Hello.class | Print "Hello, JVM!" | ✅ |
| TC-IN-002 | Calculator | Calculator.class | Correct result | ✅ |
| TC-IN-003 | Factorial | Factorial.class | Correct factorial | ✅ |
| TC-IN-004 | Fibonacci | Fibonacci.class | Correct sequence | ✅ |

---

## Test Programs

### 1. Hello World

```java
public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello, JVM!");
    }
}
```

**Expected Output**: `Hello, JVM!`

### 2. Calculator

```java
public class Calculator {
    public static int add(int a, int b) {
        return a + b;
    }
    
    public static void main(String[] args) {
        int result = add(5, 3);
        System.out.println("5 + 3 = " + result);
    }
}
```

**Expected Output**: `5 + 3 = 8`

### 3. Factorial

```java
public class Factorial {
    public static int factorial(int n) {
        if (n <= 1) return 1;
        return n * factorial(n - 1);
    }
    
    public static void main(String[] args) {
        int result = factorial(5);
        System.out.println("5! = " + result);
    }
}
```

**Expected Output**: `5! = 120`

### 4. Fibonacci

```java
public class Fibonacci {
    public static int fib(int n) {
        if (n <= 1) return n;
        return fib(n - 1) + fib(n - 2);
    }
    
    public static void main(String[] args) {
        int result = fib(10);
        System.out.println("Fib(10) = " + result);
    }
}
```

**Expected Output**: `Fib(10) = 55`

### 5. Array Operations

```java
public class ArrayTest {
    public static void main(String[] args) {
        int[] arr = new int[5];
        for (int i = 0; i < 5; i++) {
            arr[i] = i * 2;
        }
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            sum += arr[i];
        }
        System.out.println("Sum = " + sum);
    }
}
```

**Expected Output**: `Sum = 20`

### 6. Exception Handling

```java
public class ExceptionTest {
    public static void divide(int a, int b) {
        try {
            int result = a / b;
            System.out.println("Result = " + result);
        } catch (ArithmeticException e) {
            System.out.println("Division by zero!");
        }
    }
    
    public static void main(String[] args) {
        divide(10, 2);
        divide(10, 0);
    }
}
```

**Expected Output**:
```
Result = 5
Division by zero!
```

---

## Performance Tests

| Test ID | Description | Threshold | Expected | Status |
|---------|-------------|-----------|----------|--------|
| PT-001 | Hello World execution | < 1s | Pass | ✅ |
| PT-002 | Factorial(10) execution | < 1s | Pass | ✅ |
| PT-003 | Memory usage | < 10MB | Pass | ✅ |

---

## Test Execution

### Run All Tests

```bash
make test
```

### Run Specific Test

```bash
./test_parser examples/Hello.class
./test_executor examples/Calculator.class
```

### Test with GDB

```bash
gdb ./jvm
(gdb) run examples/Hello.class
(gdb) break exec_iadd
(gdb) run
```

### Test with Valgrind

```bash
valgrind ./jvm examples/Hello.class
```

---

## Test Results Template

```
=== Test Results ===

Class File Parsing Tests
------------------------
TC-CP-001: ✅ PASS
TC-CP-002: ✅ PASS
TC-CP-003: ✅ PASS

Memory Management Tests
-----------------------
TC-MM-001: ✅ PASS
TC-MM-002: ✅ PASS

Stack Operations Tests
----------------------
TC-ST-001: ✅ PASS
TC-ST-002: ✅ PASS

Total: 6/6 tests passed
```