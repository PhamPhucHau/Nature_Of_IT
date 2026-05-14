# Go Compiler - Learning Path & Implementation

## Overview

This project implements a simplified Go compiler from scratch in C. The compiler can:
- Parse Go source code (lexer/parser)
- Build Abstract Syntax Tree (AST)
- Generate intermediate representation (IR)
- Compile to executable binary
- Run Go programs

---

## Learning Path

### Phase 1: Compiler Fundamentals (Week 1-2)

#### Topics
1. **Compiler Architecture**
   - Lexical Analysis (Lexer/Tokenizer)
   - Syntax Analysis (Parser)
   - Semantic Analysis
   - Code Generation
   - Optimization

2. **Go Language Basics**
   - Go syntax and grammar
   - Type system
   - Control structures
   - Functions and methods

3. **Lexical Analysis**
   - Token types
   - Regular expressions
   - State machines

#### Resources
- [Go Language Specification](https://go.dev/ref/spec)
- [Crafting Interpreters](https://craftinginterpreters.com/)
- [Let's Build a Simple Compiler](https://github.com/kanaka/mal)

### Phase 2: Lexer Implementation (Week 3)

#### Implementation Steps

1. **Token Definitions**
   - Keywords (if, else, for, func, etc.)
   - Identifiers
   - Literals (int, string, bool)
   - Operators (+, -, *, /, etc.)
   - Delimiters ({, }, (, ), etc.)

2. **Lexer Implementation**
   - Read source code
   - Generate tokens
   - Handle comments
   - Track line numbers

### Phase 3: Parser & AST (Week 4)

#### Topics
1. **Grammar Rules**
   - Go grammar (simplified)
   - Recursive descent parsing
   - Operator precedence

2. **AST Nodes**
   - Program
   - Function declarations
   - Variable declarations
   - Expressions
   - Statements

### Phase 4: Code Generation (Week 5)

#### Topics
1. **Intermediate Representation**
   - Three-address code
   - SSA form

2. **Assembly Generation**
   - x86-64 assembly
   - System V ABI
   - System calls

### Phase 5: Advanced Features (Week 6)

#### Topics
1. **Type Checking**
   - Type inference
   - Type compatibility

2. **Optimization**
   - Constant folding
   - Dead code elimination

---

## Project Structure

```
GoCompiler/
├── README.md                 # This file
├── LEARNING_PATH.md          # Detailed learning guide
├── ARCHITECTURE.md           # Compiler architecture documentation
├── IMPLEMENTATION.md         # Implementation guide
├── TEST_CASES.md             # Test cases and examples
├── makefile
├── src/
│   ├── lexer/
│   │   ├── lexer.c
│   │   ├── lexer.h
│   │   └── token.c
│   │   └── token.h
│   ├── parser/
│   │   ├── parser.c
│   │   ├── parser.h
│   │   └── ast.c
│   │   └── ast.h
│   ├── semantic/
│   │   ├── checker.c
│   │   └── checker.h
│   ├── codegen/
│   │   ├── ir.c
│   │   ├── ir.h
│   │   └── asm.c
│   │   └── asm.h
│   └── main.c
├── examples/
│   ├── hello.go
│   ├── factorial.go
│   └── fibonacci.go
└── test/
    ├── test_lexer.c
    ├── test_parser.c
    └── test_codegen.c
```

---

## Quick Start

### Prerequisites
- GCC or Clang
- Make
- Basic C programming knowledge
- Understanding of data structures

### Build
```bash
cd GoCompiler
make
```

### Run
```bash
# Compile a Go file
./goc examples/hello.go

# Run the compiled binary
./hello
```

---

## Example Go Programs

### Hello World
```go
package main

import "fmt"

func main() {
    fmt.Println("Hello, Go Compiler!")
}
```

### Factorial
```go
package main

import "fmt"

func factorial(n int) int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n-1)
}

func main() {
    fmt.Println("5! =", factorial(5))
}
```

---

## Learning Resources

### Books
1. **"The Go Programming Language"** - Alan Donovan
2. **"Crafting Interpreters"** - Robert Nystrom
3. **"Compilers: Principles, Techniques, and Tools"** - Aho et al.

### Online Resources
1. [Go Language Spec](https://go.dev/ref/spec)
2. [Compiler Explorer](https://godbolt.org/)
3. [x86-64 Assembly Guide](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

---

## Implementation Checklist

### Phase 1: Core Structure
- [ ] Token definitions
- [ ] Lexer implementation
- [ ] AST node definitions

### Phase 2: Parsing
- [ ] Parser implementation
- [ ] Expression parsing
- [ ] Statement parsing

### Phase 3: Code Generation
- [ ] IR generation
- [ ] Assembly generation
- [ ] Linking

### Phase 4: Testing
- [ ] Unit tests
- [ ] Integration tests
- [ ] Performance tests

---

## Next Steps

1. Start with Phase 1 learning materials
2. Implement lexer for Go tokens
3. Build AST for Go programs
4. Generate assembly code
5. Test with simple Go programs
6. Add advanced features