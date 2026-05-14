# JavaScript Compiler Test Cases

## Overview

This document contains test cases for the JavaScript compiler implementation.

---

## Test Categories

### 1. Lexer Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TL-001 | Parse identifier | `hello` | TOKEN_IDENTIFIER | ✅ |
| TL-002 | Parse keyword | `function` | TOKEN_FUNCTION | ✅ |
| TL-003 | Parse integer | `42` | TOKEN_NUMBER | ✅ |
| TL-004 | Parse float | `3.14` | TOKEN_NUMBER | ✅ |
| TL-005 | Parse string | `"hello"` | TOKEN_STRING | ✅ |
| TL-006 | Parse operator | `+` | TOKEN_ADD | ✅ |
| TL-007 | Parse two-char operator | `==` | TOKEN_EQ | ✅ |
| TL-008 | Parse strict equality | `===` | TOKEN_STRICT_EQ | ✅ |
| TL-009 | Parse comment | `// comment` | Skip comment | ✅ |
| TL-010 | Parse block comment | `/* comment */` | Skip comment | ✅ |

### 2. Parser Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TP-001 | Parse function decl | `function foo() {}` | NODE_FUNCTION_DECL | ✅ |
| TP-002 | Parse var decl | `var x = 5` | NODE_VARIABLE_DECL | ✅ |
| TP-003 | Parse expression | `1 + 2` | NODE_BINARY_EXPR | ✅ |
| TP-004 | Parse return stmt | `return 42` | NODE_RETURN_STMT | ✅ |
| TP-005 | Parse if stmt | `if (x > 0) {}` | NODE_IF_STATEMENT | ✅ |
| TP-006 | Parse for stmt | `for (let i = 0; i < 10; i++) {}` | NODE_FOR_STATEMENT | ✅ |
| TP-007 | Parse call expr | `foo(1, 2)` | NODE_CALL_EXPRESSION | ✅ |

### 3. Semantic Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TS-001 | Type check number | `var x = 5` | Type: number | ✅ |
| TS-002 | Type check string | `var s = "hello"` | Type: string | ✅ |
| TS-003 | Type mismatch error | `var x = "hello"; x = 5` | Error | ✅ |
| TS-004 | Undefined variable | `console.log(x)` | Error: undefined | ✅ |
| TS-005 | Function call | `foo()` | Type: any | ✅ |

### 4. Bytecode Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TB-001 | Generate number | `42` | LOAD_CONST | ✅ |
| TB-002 | Generate add | `1 + 2` | ADD | ✅ |
| TB-003 | Generate call | `foo()` | CALL | ✅ |
| TB-004 | Generate return | `return 0` | RETURN | ✅ |

### 5. Integration Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TI-001 | Hello World | hello.js | Prints "Hello, JavaScript Compiler!" | ✅ |
| TI-002 | Factorial | factorial.js | Prints "5! = 120" | ✅ |
| TI-003 | Fibonacci | fibonacci.js | Prints "Fib(10) = 55" | ✅ |
| TI-004 | If statement | if.js | Correct branch executed | ✅ |
| TI-005 | For loop | for.js | Loop executes correctly | ✅ |
| TI-006 | Function call | func.js | Function executes correctly | ✅ |

---

## Test Programs

### 1. Hello World
```javascript
console.log("Hello, JavaScript Compiler!");
```

**Expected Output**: `Hello, JavaScript Compiler!`

### 2. Variable Declaration
```javascript
var x = 42;
console.log(x);
```

**Expected Output**: `42`

### 3. Arithmetic
```javascript
var a = 10;
var b = 5;
console.log(a + b);
console.log(a - b);
console.log(a * b);
console.log(a / b);
```

**Expected Output**:
```
15
5
50
2
```

### 4. If Statement
```javascript
var x = 10;
if (x > 5) {
    console.log("x is greater than 5");
} else {
    console.log("x is less than or equal to 5");
}
```

**Expected Output**: `x is greater than 5`

### 5. For Loop
```javascript
for (var i = 0; i < 5; i++) {
    console.log(i);
}
```

**Expected Output**:
```
0
1
2
3
4
```

### 6. Function
```javascript
function add(a, b) {
    return a + b;
}

var result = add(5, 3);
console.log(result);
```

**Expected Output**: `8`

### 7. Factorial
```javascript
function factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

console.log("5! =", factorial(5));
```

**Expected Output**: `5! = 120`

### 8. Fibonacci
```javascript
function fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

console.log("Fib(10) =", fibonacci(10));
```

**Expected Output**: `Fib(10) = 55`

### 9. Array
```javascript
var arr = [0, 2, 4, 6, 8];
var sum = 0;
for (var i = 0; i < arr.length; i++) {
    sum = sum + arr[i];
}
console.log("Sum =", sum);
```

**Expected Output**: `Sum = 20`

### 10. String Concatenation
```javascript
var name = "JavaScript";
console.log("Hello, " + name + "!");
```

**Expected Output**: `Hello, JavaScript!`

---

## Test Execution

### Run All Tests
```bash
make test
```

### Run Specific Test
```bash
./jsc examples/hello.js
./jsc examples/factorial.js
```

### Test with GDB
```bash
gdb ./jsc
(gdb) run examples/hello.js
(gdb) break parse_function_declaration
(gdb) run
```

### Test with Valgrind
```bash
valgrind ./jsc examples/hello.js
```

---

## Test Results Template

```
=== Test Results ===

Lexer Tests
-----------
TL-001: ✅ PASS
TL-002: ✅ PASS
TL-003: ✅ PASS

Parser Tests
------------
TP-001: ✅ PASS
TP-002: ✅ PASS

Semantic Tests
--------------
TS-001: ✅ PASS
TS-002: ✅ PASS

Bytecode Tests
--------------
TB-001: ✅ PASS
TB-002: ✅ PASS

Integration Tests
-----------------
TI-001: ✅ PASS
TI-002: ✅ PASS

Total: 15/15 tests passed
```