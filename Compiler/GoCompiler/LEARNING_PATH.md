# Go Compiler Learning Path - Detailed Guide

## Week 1: Compiler Fundamentals & Go Syntax

### Day 1-2: Compiler Architecture

#### Topics
1. **Compiler Phases**
   - Lexical Analysis (Lexer)
   - Syntax Analysis (Parser)
   - Semantic Analysis
   - Intermediate Code Generation
   - Code Optimization
   - Code Generation
   - Linking

2. **Compiler Frontend vs Backend**
   - Frontend: Lexer, Parser, Semantic Analysis
   - Backend: Code Generation, Optimization

#### Exercises
- Draw compiler architecture diagram
- Explain each phase's role
- Compare with JVM and C compiler

### Day 3-5: Go Language Syntax

#### Topics
1. **Go Syntax Overview**
   - Package declarations
   - Import statements
   - Function declarations
   - Variable declarations
   - Control structures (if, for, switch)
   - Arrays and slices
   - Structs

2. **Go Grammar (Simplified)**
   ```
   Program        → PackageClause {ImportDecl} {TopLevelDecl}
   PackageClause  → "package" Identifier
   ImportDecl     → "import" (StringLit | "(" {StringLit} ")")
   TopLevelDecl   → FunctionDecl | VariableDecl
   FunctionDecl   → "func" Identifier Parameters [Result] BlockStmt
   VariableDecl   → "var" Identifier [Type] "=" Expression
   ```

#### Exercises
- Write simple Go programs
- Identify syntax elements
- Draw parse trees for statements

### Day 6-7: Token Types

#### Topics
1. **Go Token Categories**
   - Keywords: if, else, for, func, var, const, etc.
   - Identifiers: variable names, function names
   - Literals: integers, floats, strings, booleans
   - Operators: +, -, *, /, =, ==, !=, etc.
   - Delimiters: {, }, (, ), [, ], ;, comma
   - Comments: // and /* */

#### Exercises
- List all Go keywords
- Identify tokens in sample code
- Create token examples

---

## Week 2: Lexical Analysis Deep Dive

### Day 1-2: Regular Expressions

#### Topics
1. **Regex for Tokenization**
   - Token patterns
   - Lexer generators (flex)
   - Manual lexer implementation

2. **Go Token Patterns**
   ```
   Identifier: [a-zA-Z_][a-zA-Z0-9_]*
   Integer:    [0-9]+
   Float:      [0-9]+\.[0-9]+
   String:     "([^"\n]|\\")*"
   Operator:   \+|\-|\*|\/|==|!=|<|>|<=|>=
   Delimiter:  \{|\}|\(|\)|\[|\]|;|,|\.|:
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
   - String state
   - Comment state

#### Exercises
- Implement basic lexer
- Test with sample code
- Handle comments

### Day 5-7: Advanced Lexer Features

#### Topics
1. **Keyword Recognition**
   - Reserved words
   - Identifier vs keyword

2. **Multi-character Tokens**
   - == vs =
   - != vs !
   - <=, >=, !=

3. **Error Handling**
   - Invalid characters
   - Unclosed strings
   - Unclosed comments

#### Exercises
- Implement keyword table
- Handle multi-char operators
- Add error reporting

---

## Week 3: Parser & AST

### Day 1-2: Parsing Concepts

#### Topics
1. **Parsing Techniques**
   - Recursive descent
   - LL(1) parsing
   - Operator precedence

2. **Grammar Design**
   - Context-free grammar
   - Left recursion elimination
   - Left factoring

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
   │   ├── PackageName
   │   ├── Imports
   │   └── Declarations
   ├── FunctionDecl
   │   ├── Name
   │   ├── Parameters
   │   ├── Result
   │   └── Body
   ├── VariableDecl
   │   ├── Name
   │   ├── Type
   │   └── Value
   ├── Expression
   │   ├── BinaryExpr
   │   ├── UnaryExpr
   │   ├── CallExpr
   │   └── Literal
   └── Statement
       ├── IfStmt
       ├── ForStmt
       └── ReturnStmt
   ```

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

## Week 4: Semantic Analysis

### Day 1-2: Type System

#### Topics
1. **Go Type System**
   - Basic types (int, string, bool)
   - Composite types (struct, array, slice)
   - Function types
   - Interface types

2. **Type Checking**
   - Type compatibility
   - Type inference
   - Type conversion

#### Exercises
- Design type structures
- Implement type checking
- Test with type errors

### Day 3-4: Symbol Tables

#### Topics
1. **Symbol Table Design**
   - Variable declarations
   - Function declarations
   - Scope management
   - Nested scopes

2. **Scope Chains**
   - Global scope
   - Function scope
   - Block scope

#### Exercises
- Implement symbol table
- Handle scope nesting
- Test with variable resolution

### Day 5-7: Semantic Checks

#### Topics
1. **Semantic Analysis**
   - Variable declarations
   - Function calls
   - Type checking
   - Reachability analysis

2. **Error Reporting**
   - Undefined variables
   - Type mismatches
   - Unused variables

#### Exercises
- Implement semantic checker
- Add error reporting
- Test with invalid code

---

## Week 5: Code Generation

### Day 1-2: Intermediate Representation

#### Topics
1. **IR Design**
   - Three-address code
   - SSA form
   - IR instructions

2. **IR Generation**
   - Expression translation
   - Statement translation
   - Function translation

#### Exercises
- Design IR structures
- Generate IR from AST
- Test IR generation

### Day 3-4: Assembly Generation

#### Topics
1. **x86-64 Assembly**
   - Registers (rax, rbx, rcx, etc.)
   - Instructions (mov, add, sub, call)
   - Calling convention (System V ABI)

2. **Assembly Generation**
   - Register allocation
   - Stack frame management
   - Function prologue/epilogue

#### Exercises
- Write assembly for simple expressions
- Generate function calls
- Test assembly output

### Day 5-7: Code Generation

#### Topics
1. **Complete Code Generation**
   - Main function
   - Package initialization
   - Runtime support

2. **Output Generation**
   - Assembly file
   - Object file
   - Executable

#### Exercises
- Generate complete programs
- Assemble and link
- Test executables

---

## Week 6: Testing & Optimization

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
   - Arrays

#### Exercises
- Write test suite
- Run tests
- Fix bugs

### Day 3-4: Optimization

#### Topics
1. **Optimization Techniques**
   - Constant folding
   - Dead code elimination
   - Common subexpression elimination

2. **IR Optimization**
   - Simplify expressions
   - Remove unused code

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
- [Go Source Code](https://github.com/golang/go)
- [Simple Compiler](https://github.com/kanaka/mal)
- [Compiler Explorer](https://godbolt.org/)

### Tools
- `go fmt` - Format Go code
- `go build` - Build Go programs
- `objdump` - Disassemble binaries
- `gdb` - Debug assembly

### Practice Projects
1. Implement a calculator
2. Build a simple interpreter
3. Create a domain-specific language

---

## Assessment

### Phase 1 (Week 1-2)
- [ ] Understand compiler architecture
- [ ] Know Go syntax
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
- [ ] Generate IR
- [ ] Generate assembly
- [ ] Create executables

### Phase 5 (Week 6)
- [ ] Pass all tests
- [ ] Document code
- [ ] Optimize performance