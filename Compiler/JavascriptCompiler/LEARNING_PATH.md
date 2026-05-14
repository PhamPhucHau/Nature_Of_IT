# JavaScript Compiler Learning Path - Detailed Guide

## Week 1: JavaScript Fundamentals & V8 Overview

### Day 1-2: JavaScript Language Basics

#### Topics
1. **ECMAScript 6+ Syntax**
   - Variable declarations (var, let, const)
   - Functions (arrow functions, default params)
   - Classes and objects
   - Arrays and destructuring
   - Modules (import/export)
   - Async/await

2. **JavaScript Type System**
   - Dynamic typing
   - Type coercion
   - Primitive types (number, string, boolean, null, undefined, symbol)
   - Object types

#### Exercises
- Write JavaScript programs
- Identify syntax elements
- Understand type coercion

### Day 3-5: V8 Architecture

#### Topics
1. **V8 Components**
   - **Parser**: Converts source to AST
   - **Bytecode Generator**: Converts AST to bytecode
   - **Ignition**: Bytecode interpreter
   - **TurboFan**: JIT compiler
   - **Heap**: Memory management
   - **Garbage Collector**: Mark-sweep, compaction

2. **Execution Pipeline**
   ```
   Source Code
       ↓
   Parser (AST)
       ↓
   Bytecode Generator (Bytecode)
       ↓
   Ignition (Interpreter)
       ↓
   Profiler (Hot functions)
       ↓
   TurboFan (JIT Compiler)
       ↓
   Machine Code
   ```

#### Exercises
- Draw V8 architecture diagram
- Explain each component's role
- Compare with other JS engines (SpiderMonkey, JavaScriptCore)

### Day 6-7: Token Types

#### Topics
1. **JavaScript Token Categories**
   - Keywords: var, let, const, function, return, etc.
   - Identifiers: variable/function names
   - Literals: numbers, strings, booleans, regex
   - Operators: +, -, *, /, =, ==, ===, etc.
   - Delimiters: {, }, (, ), [, ], ;, comma
   - Comments: // and /* */

#### Exercises
- List all JavaScript keywords
- Identify tokens in sample code
- Create token examples

---

## Week 2: Advanced JavaScript Concepts

### Day 1-2: Functions & Closures

#### Topics
1. **Function Concepts**
   - Function declarations vs expressions
   - Closures
   - Scope chains
   - this binding
   - Arrow functions

2. **Higher-Order Functions**
   - Callbacks
   - Function composition
   - Currying

#### Exercises
- Write closure examples
- Understand scope chains
- Test this binding

### Day 3-4: Objects & Prototypes

#### Topics
1. **Object System**
   - Object literals
   - Prototypes
   - Inheritance
   - Classes (syntax sugar)

2. **Property Access**
   - Dot notation
   - Bracket notation
   - Getters/setters

#### Exercises
- Create object hierarchies
- Understand prototype chain
- Implement class syntax

### Day 5-7: Async Programming

#### Topics
1. **Asynchronous JavaScript**
   - Callbacks
   - Promises
   - Async/await
   - Event loop

2. **Execution Context**
   - Call stack
   - Callback queue
   - Microtask queue

#### Exercises
- Write async functions
- Understand event loop
- Debug async code

---

## Week 3: Lexer Implementation

### Day 1-2: Regular Expressions

#### Topics
1. **Regex for Tokenization**
   - Token patterns
   - Lexer generators (flex)
   - Manual lexer implementation

2. **JavaScript Token Patterns**
   ```
   Identifier: [a-zA-Z_$][a-zA-Z0-9_$]*
   Number:     [0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?
   String:     "([^"\n]|\\")*"|'([^'\n]|\\')*'|`([^`\n]|\\`)*`
   Operator:   \+|\-|\*|\/|==|===|!=|!==|<|>|<=|>=
   ```

#### Exercises
- Write regex for each token type
- Test regex patterns
- Handle edge cases

### Day 3-4: Lexer Implementation

#### Topics
1. **Lexer Design**
   - Input buffer
   - Current position
   - Token structure
   - Error handling

2. **State Machine**
   - Initial state
   - Identifier state
   - Number state
   - String state (template literals)
   - Comment state
   - Regex state

#### Exercises
- Implement basic lexer
- Test with sample code
- Handle comments

### Day 5-7: Advanced Lexer Features

#### Topics
1. **Keyword Recognition**
   - Reserved words
   - Strict mode keywords
   - Identifier vs keyword

2. **Multi-character Tokens**
   - === vs ==
   - !== vs !=
   - <<=, >>=, >>>=

3. **Error Handling**
   - Invalid characters
   - Unclosed strings
   - Unclosed comments
   - Template literal nesting

#### Exercises
- Implement keyword table
- Handle multi-char operators
- Add error reporting

---

## Week 4: Parser & AST

### Day 1-2: Parsing Concepts

#### Topics
1. **Parsing Techniques**
   - Recursive descent
   - LL(1) parsing
   - Operator precedence

2. **JavaScript Grammar**
   - Expression grammar
   - Statement grammar
   - Function grammar

#### Exercises
- Design grammar for expressions
- Eliminate left recursion
- Test with sample code

### Day 3-4: AST Node Design

#### Topics
1. **AST Node Types**
   ```
   Node
   ├── Program
   │   ├── Directives
   │   └── Body
   ├── FunctionDeclaration
   │   ├── Id
   │   ├── Params
   │   ├── Body
   │   └── Generator/Async flags
   ├── VariableDeclaration
   │   ├── Kind (var/let/const)
   │   └── Declarations
   ├── Expression
   │   ├── BinaryExpression
   │   ├── UnaryExpression
   │   ├── CallExpression
   │   ├── MemberExpression
   │   ├── Identifier
   │   └── Literal
   └── Statement
       ├── IfStatement
       ├── ForStatement
       ├── WhileStatement
       ├── ReturnStatement
       └── ExpressionStatement
   ```

2. **Visitor Pattern**
   - AST visitor
   - Node traversal
   - Type checking

#### Exercises
- Design AST structures
- Create node constructors
- Test AST building

### Day 5-7: Parser Implementation

#### Topics
1. **Recursive Descent Parser**
   - Parse functions for each grammar rule
   - Token consumption
   - Error recovery

2. **Expression Parsing**
   - Operator precedence
   - Associativity
   - Parentheses handling

#### Exercises
- Implement parser for expressions
- Build AST from tokens
- Test with sample programs

---

## Week 5: Bytecode Generation

### Day 1-2: Bytecode Format

#### Topics
1. **Bytecode Design**
   - Instruction set
   - Operand stack vs registers
   - Bytecode size optimization

2. **Ignition-style Bytecode**
   - Register-based VM
   - Bytecode operators
   - Jump targets

#### Exercises
- Design bytecode format
- Create instruction set
- Test bytecode encoding

### Day 3-4: Bytecode Generator

#### Topics
1. **AST to Bytecode**
   - Visitor pattern
   - Register allocation
   - Label management

2. **Control Flow**
   - Conditional jumps
   - Loop handling
   - Function calls

#### Exercises
- Implement bytecode generator
- Generate bytecode for expressions
- Test with simple programs

### Day 5-7: Advanced Bytecode

#### Topics
1. **Optimizations**
   - Constant folding
   - Dead code elimination
   - Register coalescing

2. **Error Handling**
   - Try-catch bytecode
   - Exception tables

#### Exercises
- Implement optimizations
- Add error handling
- Test with complex programs

---

## Week 6: Interpreter

### Day 1-2: Interpreter Design

#### Topics
1. **Bytecode Interpreter**
   - Instruction dispatch
   - Operand stack
   - Register file
   - Call stack

2. **Runtime Environment**
   - Global object
   - Function objects
   - Scope chains

#### Exercises
- Design interpreter structures
- Implement instruction dispatch
- Test with bytecode

### Day 3-4: Execution Engine

#### Topics
1. **Execution Loop**
   - Fetch-decode-execute
   - PC register
   - Stack management

2. **Function Calls**
   - Call setup
   - Argument passing
   - Return handling

#### Exercises
- Implement execution loop
- Test function calls
- Debug interpreter

### Day 5-7: Runtime Support

#### Topics
1. **Runtime Functions**
   - Type checking
   - Property access
   - Operator overloading

2. **Error Handling**
   - Runtime errors
   - Exception throwing

#### Exercises
- Implement runtime functions
- Add error handling
- Test edge cases

---

## Week 7: JIT Compilation

### Day 1-2: JIT Concepts

#### Topics
1. **TurboFan Architecture**
   - Hydrogen IR
   - Lithium IR
   - Code generation

2. **Hot Function Detection**
   - Feedback vectors
   - Tier-up triggers
   - Profiling

#### Exercises
- Design JIT structures
- Implement profiling
- Test hot detection

### Day 3-4: IR Generation

#### Topics
1. **Intermediate Representation**
   - Graph-based IR
   - SSA form
   - Optimization passes

2. **Optimization**
   - Constant propagation
   - Dead code elimination
   - Strength reduction

#### Exercises
- Generate IR from bytecode
- Implement optimizations
- Test IR generation

### Day 5-7: Code Generation

#### Topics
1. **Assembly Generation**
   - x86-64 assembly
   - Register allocation
   - Calling convention

2. **Machine Code**
   - Code object creation
   - Relocation
   - Cache flushing

#### Exercises
- Generate assembly
- Assemble to machine code
- Test executables

---

## Week 8: Testing & Optimization

### Day 1-2: Testing

#### Topics
1. **Test Strategy**
   - Unit tests
   - Integration tests
   - Regression tests

2. **Test Cases**
   - Hello World
   - Arithmetic
   - Control flow
   - Functions
   - Objects

#### Exercises
- Write test suite
- Run tests
- Fix bugs

### Day 3-4: Optimization

#### Topics
1. **Optimization Techniques**
   - Inlining
   - Loop unrolling
   - Common subexpression elimination

2. **Memory Optimization**
   - Object layout
   - Hidden classes
   - Fast properties

#### Exercises
- Implement optimizations
- Measure improvements
- Test optimized code

### Day 5-6: Final Project

#### Topics
1. **Complete Compiler**
   - All phases working
   - Error handling
   - Performance

2. **Documentation**
   - Architecture docs
   - User guide
   - API docs

#### Exercises
- Final integration
- Documentation
- Demo

---

## Additional Resources

### Code Examples
- [V8 Source Code](https://github.com/v8/v8)
- [Simple JS Engine](https://github.com/rui314/8cc)
- [TinyJS](https://github.com/kripken/tinyscheme)

### Tools
- `node --v8-options` - V8 options
- `node --trace-ast` - Trace AST
- `objdump` - Disassemble binaries
- `gdb` - Debug C++ code

### Practice Projects
1. Implement a calculator
2. Build a simple interpreter
3. Create a domain-specific language

---

## Assessment

### Phase 1 (Week 1-2)
- [ ] Understand JavaScript syntax
- [ ] Know V8 architecture
- [ ] Design token types

### Phase 2 (Week 3)
- [ ] Implement lexer
- [ ] Generate tokens correctly
- [ ] Handle comments

### Phase 3 (Week 4)
- [ ] Implement parser
- [ ] Build AST
- [ ] Handle expressions

### Phase 4 (Week 5)
- [ ] Generate bytecode
- [ ] Implement interpreter
- [ ] Execute programs

### Phase 5 (Week 6-8)
- [ ] Pass all tests
- [ ] Document code
- [ ] Optimize performance