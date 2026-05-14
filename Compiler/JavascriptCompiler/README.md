# JavaScript Compiler (V8-like) - Learning Path & Implementation

## Overview

This project implements a simplified JavaScript compiler inspired by V8 Engine in C++. The compiler can:
- Parse JavaScript source code (lexer/parser)
- Build Abstract Syntax Tree (AST)
- Generate bytecode (Ignition-style)
- Execute bytecode with interpreter
- Compile hot functions to machine code (TurboFan-style - simplified)
- Run JavaScript programs

---

## Learning Path

### Phase 1: JavaScript Fundamentals (Week 1-2)

#### Topics
1. **JavaScript Language**
   - ES6+ syntax
   - Type system (dynamic)
   - Functions and closures
   - Objects and prototypes
   - Async/await

2. **V8 Architecture Overview**
   - Parser (Hydrogen/Graph)
   - Bytecode Generator (Ignition)
   - Interpreter
   - JIT Compiler (TurboFan)
   - Garbage Collector (Mark-Sweep, Compacting)

#### Resources
- [V8 Documentation](https://v8.dev/docs)
- [JavaScript Specification](https://tc39.es/ecma262/)
- [How V8 Works](https://v8.dev/blog/preparser)

### Phase 2: Lexer & Parser (Week 3-4)

#### Implementation Steps

1. **Token Definitions**
   - Keywords (var, let, const, function, etc.)
   - Identifiers
   - Literals (number, string, boolean)
   - Operators (+, -, *, /, etc.)
   - Delimiters ({, }, (, ), etc.)

2. **Parser Implementation**
   - Recursive descent parsing
   - AST construction
   - Error handling

### Phase 3: Bytecode Generation (Week 5)

#### Topics
1. **Bytecode Format**
   - Instruction set
   - Operand stack
   - Register-based VM

2. **Bytecode Generator**
   - Visit AST nodes
   - Generate bytecode instructions
   - Handle control flow

### Phase 4: Interpreter (Week 6)

#### Topics
1. **Bytecode Interpreter**
   - Instruction dispatch
   - Operand stack management
   - Register file
   - Call stack

2. **Runtime Environment**
   - Global object
   - Function objects
   - Scope chains

### Phase 5: JIT Compilation (Week 7-8)

#### Topics
1. **TurboFan-style Compilation**
   - Hot function detection
   - IR generation
   - Code generation
   - Machine code output

---

## Project Structure

```
JavaScriptCompiler/
├── README.md                 # This file
├── LEARNING_PATH.md          # Detailed learning guide
├── ARCHITECTURE.md           # Compiler architecture documentation
├── IMPLEMENTATION.md         # Implementation guide
├── TEST_CASES.md             # Test cases and examples
├── makefile
├── src/
│   ├── lexer/
│   │   ├── lexer.cc
│   │   ├── lexer.h
│   │   ├── token.cc
│   │   └── token.h
│   ├── parser/
│   │   ├── parser.cc
│   │   ├── parser.h
│   │   ├── ast.cc
│   │   └── ast.h
│   ├── bytecode/
│   │   ├── generator.cc
│   │   ├── generator.h
│   │   ├── interpreter.cc
│   │   └── interpreter.h
│   ├── jit/
│   │   ├── compiler.cc
│   │   └── compiler.h
│   └── main.cc
├── examples/
│   ├── hello.js
│   ├── factorial.js
│   └── fibonacci.js
└── test/
    ├── test_lexer.cc
    ├── test_parser.cc
    └── test_bytecode.cc
```

---

## Quick Start

### Prerequisites
- C++17 or later
- GCC or Clang
- Make
- Basic C++ programming knowledge

### Build
```bash
cd JavaScriptCompiler
make
```

### Run
```bash
# Compile and run a JavaScript file
./jsc examples/hello.js
```

---

## Example JavaScript Programs

### Hello World
```javascript
console.log("Hello, JavaScript Compiler!");
```

### Factorial
```javascript
function factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

console.log("5! =", factorial(5));
```

---

## Learning Resources

### Books
1. **"How V8 Works"** - Irakli Gozalishvili
2. **"JavaScript: The Good Parts"** - Douglas Crockford
3. **"Compilers: Principles, Techniques, and Tools"** - Aho et al.

### Online Resources
1. [V8 Documentation](https://v8.dev/docs)
2. [TC39 Specifications](https://tc39.es/)
3. [JavaScript Engine Resources](https://github.com/leobalter/engine-resources)

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

### Phase 3: Bytecode
- [ ] Bytecode generator
- [ ] Interpreter
- [ ] Runtime environment

### Phase 4: Testing
- [ ] Unit tests
- [ ] Integration tests
- [ ] Performance tests

---

## Next Steps

1. Start with Phase 1 learning materials
2. Implement lexer for JavaScript tokens
3. Build AST for JavaScript programs
4. Generate bytecode
5. Implement interpreter
6. Add JIT compilation