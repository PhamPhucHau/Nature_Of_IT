# Go Compiler Test Cases

## Overview

This document contains test cases for the Go compiler implementation.

---

## Test Categories

### 1. Lexer Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TL-001 | Parse identifier | `hello` | TOKEN_IDENT | ✅ |
| TL-002 | Parse keyword | `func` | TOKEN_FUNC | ✅ |
| TL-003 | Parse integer | `42` | TOKEN_INT_LIT | ✅ |
| TL-004 | Parse float | `3.14` | TOKEN_FLOAT_LIT | ✅ |
| TL-005 | Parse string | `"hello"` | TOKEN_STRING_LIT | ✅ |
| TL-006 | Parse operator | `+` | TOKEN_ADD | ✅ |
| TL-007 | Parse two-char operator | `==` | TOKEN_EQ | ✅ |
| TL-008 | Parse comment | `// comment` | Skip comment | ✅ |
| TL-009 | Parse block comment | `/* comment */` | Skip comment | ✅ |
| TL-010 | Parse delimiter | `{` | TOKEN_LBRACE | ✅ |

### 2. Parser Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TP-001 | Parse package decl | `package main` | NODE_PACKAGE_DECL | ✅ |
| TP-002 | Parse import decl | `import "fmt"` | NODE_IMPORT_DECL | ✅ |
| TP-003 | Parse func decl | `func main()` | NODE_FUNCTION_DECL | ✅ |
| TP-004 | Parse var decl | `var x int` | NODE_VARIABLE_DECL | ✅ |
| TP-005 | Parse expression | `1 + 2` | NODE_BINARY_EXPR | ✅ |
| TP-006 | Parse return stmt | `return 42` | NODE_RETURN_STMT | ✅ |
| TP-007 | Parse if stmt | `if x > 0 {}` | NODE_IF_STMT | ✅ |
| TP-008 | Parse for stmt | `for i := 0; i < 10; i++ {}` | NODE_FOR_STMT | ✅ |

### 3. Semantic Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TS-001 | Type check int | `var x int = 5` | Type: int | ✅ |
| TS-002 | Type check string | `var s string = "hello"` | Type: string | ✅ |
| TS-003 | Type mismatch error | `var x int = "hello"` | Error | ✅ |
| TS-004 | Undefined variable | `println(x)` | Error: undefined | ✅ |
| TS-005 | Function call | `main()` | Type: void | ✅ |

### 4. Code Generation Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TC-001 | Generate int literal | `42` | movq $42, %rax | ✅ |
| TC-002 | Generate add | `1 + 2` | addq %rbx, %rax | ✅ |
| TC-003 | Generate function call | `main()` | call main | ✅ |
| TC-004 | Generate return | `return 0` | ret | ✅ |

### 5. Integration Tests

| Test ID | Description | Input | Expected Output | Status |
|---------|-------------|-------|-----------------|--------|
| TI-001 | Hello World | hello.go | Prints "Hello, Go Compiler!" | ✅ |
| TI-002 | Factorial | factorial.go | Prints "5! = 120" | ✅ |
| TI-003 | Fibonacci | fibonacci.go | Prints "Fib(10) = 55" | ✅ |
| TI-004 | If statement | if.go | Correct branch executed | ✅ |
| TI-005 | For loop | for.go | Loop executes correctly | ✅ |

---

## Test Programs

### 1. Hello World
```go
package main

func main() {
    println("Hello, Go Compiler!")
}
```

**Expected Output**: `Hello, Go Compiler!`

### 2. Variable Declaration
```go
package main

func main() {
    var x int = 42
    println(x)
}
```

**Expected Output**: `42`

### 3. Arithmetic
```go
package main

func main() {
    var a int = 10
    var b int = 5
    println(a + b)
    println(a - b)
    println(a * b)
    println(a / b)
}
```

**Expected Output**:
```
15
5
50
2
```

### 4. If Statement
```go
package main

func main() {
    var x int = 10
    if x > 5 {
        println("x is greater than 5")
    } else {
        println("x is less than or equal to 5")
    }
}
```

**Expected Output**: `x is greater than 5`

### 5. For Loop
```go
package main

func main() {
    for i := 0; i < 5; i++ {
        println(i)
    }
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
```go
package main

func add(a int, b int) int {
    return a + b
}

func main() {
    var result int = add(5, 3)
    println(result)
}
```

**Expected Output**: `8`

### 7. Factorial
```go
package main

func factorial(n int) int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n-1)
}

func main() {
    println("5! =", factorial(5))
}
```

**Expected Output**: `5! = 120`

### 8. Fibonacci
```go
package main

func fibonacci(n int) int {
    if n <= 1 {
        return n
    }
    return fibonacci(n-1) + fibonacci(n-2)
}

func main() {
    println("Fib(10) =", fibonacci(10))
}
```

**Expected Output**: `Fib(10) = 55`

### 9. Array
```go
package main

func main() {
    var arr [5]int
    for i := 0; i < 5; i++ {
        arr[i] = i * 2
    }
    var sum int = 0
    for i := 0; i < 5; i++ {
        sum = sum + arr[i]
    }
    println("Sum =", sum)
}
```

**Expected Output**: `Sum = 20`

### 10. String
```go
package main

func main() {
    var name string = "Go"
    println("Hello, " + name + "!")
}
```

**Expected Output**: `Hello, Go!`

---

## Test Execution

### Run All Tests
```bash
make test
```

### Run Specific Test
```bash
./goc examples/hello.go
./goc examples/factorial.go
```

### Test with GDB
```bash
gdb ./goc
(gdb) run examples/hello.go
(gdb) break parse_function_decl
(gdb) run
```

### Test with Valgrind
```bash
valgrind ./goc examples/hello.go
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

Code Generation Tests
---------------------
TC-001: ✅ PASS
TC-002: ✅ PASS

Integration Tests
-----------------
TI-001: ✅ PASS
TI-002: ✅ PASS

Total: 15/15 tests passed
```