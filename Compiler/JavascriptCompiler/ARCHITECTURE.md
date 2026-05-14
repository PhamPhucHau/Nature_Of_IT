# JavaScript Compiler Architecture Documentation

## Overview

This document describes the architecture of our simplified JavaScript compiler inspired by V8 Engine.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   JavaScript Compiler                       │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Lexer      │  │   Parser     │  │ Bytecode     │      │
│  │   (Tokenizer│  │   (AST       │  │ Generator    │      │
│  │   )          │  │   Builder)   │  │              │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Token      │  │    AST       │  │  Bytecode    │      │
│  │   Stream     │  │   Tree       │  │   Stream     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │  Interpreter   │                    │
│                      │  (Ignition)    │                    │
│                      │                │                    │
│                      │  Register VM   │                    │
│                      └────────────────┘                    │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │  JIT Compiler  │                    │
│                      │  (TurboFan)    │                    │
│                      │                │                    │
│                      │  Machine Code  │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Compiler Phases

### 1. Lexer (Tokenizer)

**Purpose**: Convert source code into tokens

**Input**: JavaScript source code as string

**Output**: Stream of tokens

**Token Types**:
```
- KEYWORD: var, let, const, function, return, etc.
- IDENT: variable/function names
- LITERAL: numbers, strings, booleans
- OPERATOR: +, -, *, /, =, ==, ===, etc.
- DELIMITER: {, }, (, ), [, ], ;, comma
- COMMENT: // and /* */
- EOF: end of file
```

**Structure**:
```cpp
enum class TokenType {
    EOF_TOKEN,
    ILLEGAL,
    // Keywords
    VAR, LET, CONST, FUNCTION, RETURN, IF, ELSE, FOR, WHILE,
    // Identifiers and literals
    IDENTIFIER, NUMBER, STRING, BOOLEAN,
    // Operators
    ASSIGN, ADD, SUB, MUL, DIV, EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT, XOR, LSHIFT, RSHIFT,
    // Delimiters
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, SEMICOLON, DOT, COLON,
    // Comments
    COMMENT_LINE, COMMENT_BLOCK
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};
```

### 2. Parser (AST Builder)

**Purpose**: Build Abstract Syntax Tree from tokens

**Input**: Token stream

**Output**: AST root node

**AST Node Types**:
```cpp
enum class NodeType {
    PROGRAM,
    FUNCTION_DECLARATION,
    VARIABLE_DECLARATION,
    EXPRESSION,
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    IDENTIFIER,
    LITERAL,
    IF_STATEMENT,
    FOR_STATEMENT,
    WHILE_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION_STATEMENT,
    BLOCK_STATEMENT
};

struct ASTNode {
    NodeType type;
    ASTNode* parent;
    std::vector<ASTNode*> children;
    union {
        std::string identifier;
        int int_value;
        double float_value;
        std::string string_value;
        bool bool_value;
    } value;
};
```

### 3. Bytecode Generator

**Purpose**: Generate bytecode from AST

**Input**: AST root node

**Output**: Bytecode stream

**Bytecode Instructions**:
```cpp
enum class BytecodeOpcode {
    LOAD_GLOBAL,
    LOAD_LOCAL,
    STORE_LOCAL,
    LOAD_CONST,
    ADD,
    SUB,
    MUL,
    DIV,
    EQ,
    NEQ,
    LT,
    GT,
    LE,
    GE,
    JUMP,
    JUMP_IF_FALSE,
    CALL,
    RETURN,
    ENTER_CONTEXT,
    EXIT_CONTEXT
};

struct BytecodeInstruction {
    BytecodeOpcode opcode;
    std::vector<int> operands;
};
```

### 4. Interpreter

**Purpose**: Execute bytecode instructions

**Input**: Bytecode stream

**Output**: Execution results

**Interpreter State**:
```cpp
struct RegisterFile {
    std::vector<Value> registers;
    int sp;  // Stack pointer
};

struct CallFrame {
    Bytecode* bytecode;
    int pc;  // Program counter
    RegisterFile* registers;
    CallFrame* caller;
};
```

### 5. JIT Compiler (TurboFan-style)

**Purpose**: Compile hot functions to machine code

**Input**: Bytecode with profiling feedback

**Output**: Machine code

**IR Nodes**:
```cpp
enum class IRNodeType {
    PHI,
    CONSTANT,
    LOAD,
    STORE,
    ADD,
    SUB,
    MUL,
    DIV,
    CALL,
    RETURN,
    JUMP,
    BRANCH
};

struct IRNode {
    IRNodeType type;
    std::vector<IRNode*> inputs;
    Value value;
};
```

---

## Data Structures

### Token Structure
```cpp
enum class TokenType {
    EOF_TOKEN, ILLEGAL,
    VAR, LET, CONST, FUNCTION, RETURN, IF, ELSE, FOR, WHILE,
    IDENTIFIER, NUMBER, STRING, BOOLEAN,
    ASSIGN, ADD, SUB, MUL, DIV, EQ, NEQ, LT, GT, LE, GE,
    AND, OR, NOT, XOR, LSHIFT, RSHIFT,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    COMMA, SEMICOLON, DOT, COLON,
    COMMENT_LINE, COMMENT_BLOCK
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType t, const std::string& l, int ln, int col)
        : type(t), lexeme(l), line(ln), column(col) {}
};
```

### AST Node Structure
```cpp
enum class NodeType {
    PROGRAM, FUNCTION_DECLARATION, VARIABLE_DECLARATION,
    BINARY_EXPRESSION, UNARY_EXPRESSION, CALL_EXPRESSION,
    MEMBER_EXPRESSION, IDENTIFIER, LITERAL,
    IF_STATEMENT, FOR_STATEMENT, WHILE_STATEMENT,
    RETURN_STATEMENT, EXPRESSION_STATEMENT, BLOCK_STATEMENT
};

struct ASTNode {
    NodeType type;
    ASTNode* parent;
    std::vector<ASTNode*> children;
    
    union {
        std::string identifier;
        int int_value;
        double float_value;
        std::string string_value;
        bool bool_value;
    } value;
    
    ASTNode(NodeType t) : type(t), parent(nullptr) {}
    ~ASTNode() {
        for (auto* child : children) delete child;
    }
};
```

### Value Type
```cpp
enum class ValueType {
    UNDEFINED, NULL_TYPE, BOOLEAN, NUMBER, STRING, OBJECT, FUNCTION
};

struct Value {
    ValueType type;
    union {
        bool bool_value;
        double number_value;
        std::string* string_value;
        void* object_value;
    };
    
    Value() : type(ValueType::UNDEFINED) {}
    Value(bool b) : type(ValueType::BOOLEAN), bool_value(b) {}
    Value(double n) : type(ValueType::NUMBER), number_value(n) {}
    Value(const std::string& s) : type(ValueType::STRING) {
        string_value = new std::string(s);
    }
};
```

---

## Lexer Implementation

### State Machine
```
Start
  │
  ├─► Letter/Underscore ──► Identifier/Keyword
  │
  ├─► Digit ──► Number
  │
  ├─► '"' / "'" / '`' ──► String
  │
  ├─► '/' ──► Comment or Operator
  │
  ├─► Operator characters ──► Operator
  │
  ├─► Delimiter characters ──► Delimiter
  │
  └─► Whitespace ──► Skip
```

### Tokenization Algorithm
```cpp
class Lexer {
private:
    const std::string& input;
    int position;
    int read_position;
    int line;
    int column;
    char ch;
    
    void read_char() {
        if (read_position >= input.length()) {
            ch = '\0';
        } else {
            ch = input[read_position];
        }
        position = read_position++;
        if (ch == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
    }
    
    char peek_char() {
        if (read_position >= input.length()) return '\0';
        return input[read_position];
    }
    
public:
    Token next_token() {
        skip_whitespace();
        
        if (is_letter(ch)) {
            return read_identifier();
        } else if (is_digit(ch)) {
            return read_number();
        } else if (ch == '"' || ch == '\'') {
            return read_string();
        } else if (ch == '/') {
            if (peek_char() == '/') {
                skip_line_comment();
                return next_token();
            } else if (peek_char() == '*') {
                skip_block_comment();
                return next_token();
            }
            return read_operator();
        } else if (is_operator_char(ch)) {
            return read_operator();
        } else if (is_delimiter(ch)) {
            return read_delimiter();
        }
        
        return Token(TokenType::ILLEGAL, std::string(1, ch), line, column);
    }
};
```

---

## Parser Implementation

### Recursive Descent Parsing
```cpp
class Parser {
private:
    std::vector<Token>& tokens;
    int position;
    
    Token* current_token() {
        if (position >= tokens.size()) return nullptr;
        return &tokens[position];
    }
    
    Token* peek_token() {
        if (position + 1 >= tokens.size()) return nullptr;
        return &tokens[position + 1];
    }
    
    void next_token() { position++; }
    
    bool match(TokenType type) {
        return current_token() && current_token()->type == type;
    }
    
    bool expect(TokenType type) {
        if (match(type)) {
            next_token();
            return true;
        }
        return false;
    }

public:
    ASTNode* parse_program() {
        auto* program = new ASTNode(NodeType::PROGRAM);
        
        while (current_token() && current_token()->type != TokenType::EOF_TOKEN) {
            if (match(TokenType::VAR) || match(TokenType::LET) || match(TokenType::CONST)) {
                program->children.push_back(parse_variable_declaration());
            } else if (match(TokenType::FUNCTION)) {
                program->children.push_back(parse_function_declaration());
            } else {
                program->children.push_back(parse_statement());
            }
        }
        
        return program;
    }
    
    ASTNode* parse_function_declaration() {
        expect(TokenType::FUNCTION);
        
        auto* func = new ASTNode(NodeType::FUNCTION_DECLARATION);
        
        // Parse function name
        if (match(TokenType::IDENTIFIER)) {
            func->value.identifier = current_token()->lexeme;
            next_token();
        }
        
        // Parse parameters
        expect(TokenType::LPAREN);
        while (!match(TokenType::RPAREN)) {
            if (match(TokenType::IDENTIFIER)) {
                auto* param = new ASTNode(NodeType::IDENTIFIER);
                param->value.identifier = current_token()->lexeme;
                func->children.push_back(param);
                next_token();
            }
            if (match(TokenType::COMMA)) next_token();
        }
        expect(TokenType::RPAREN);
        
        // Parse body
        expect(TokenType::LBRACE);
        while (!match(TokenType::RBRACE)) {
            func->children.push_back(parse_statement());
        }
        expect(TokenType::RBRACE);
        
        return func;
    }
};
```

---

## Bytecode Generation

### IR Generation
```cpp
class BytecodeGenerator {
private:
    ASTNode* ast;
    std::vector<BytecodeInstruction> bytecode;
    int next_register;
    int next_label;
    
    int new_register() { return next_register++; }
    int new_label() { return next_label++; }
    
    void generate(ASTNode* node) {
        switch (node->type) {
            case NodeType::BINARY_EXPRESSION:
                generate(node->children[0]);  // Left
                generate(node->children[1]);  // Right
                bytecode.push_back({get_opcode(node->value.int_value)});
                break;
                
            case NodeType::IDENTIFIER:
                bytecode.push_back({BytecodeOpcode::LOAD_LOCAL, {get_register(node->value.identifier)}});
                break;
                
            case NodeType::LITERAL:
                bytecode.push_back({BytecodeOpcode::LOAD_CONST, {get_constant_index(node->value)}});
                break;
                
            // ... other cases
        }
    }
    
    BytecodeOpcode get_opcode(TokenType op) {
        switch (op) {
            case TokenType::ADD: return BytecodeOpcode::ADD;
            case TokenType::SUB: return BytecodeOpcode::SUB;
            case TokenType::MUL: return BytecodeOpcode::MUL;
            case TokenType::DIV: return BytecodeOpcode::DIV;
            default: return BytecodeOpcode::NOP;
        }
    }
};
```

---

## Interpreter Implementation

### Execution Loop
```cpp
class Interpreter {
private:
    std::vector<Value> registers;
    int sp;
    std::vector<BytecodeInstruction> bytecode;
    int pc;
    
    Value execute() {
        while (pc < bytecode.size()) {
            auto& instr = bytecode[pc];
            
            switch (instr.opcode) {
                case BytecodeOpcode::LOAD_GLOBAL:
                    registers[instr.operands[0]] = get_global(instr.operands[1]);
                    pc++;
                    break;
                    
                case BytecodeOpcode::ADD:
                    registers[instr.operands[0]] = 
                        registers[instr.operands[1]] + registers[instr.operands[2]];
                    pc++;
                    break;
                    
                case BytecodeOpcode::CALL:
                    call_function(registers[instr.operands[0]], instr.operands[1]);
                    pc++;
                    break;
                    
                case BytecodeOpcode::RETURN:
                    return registers[instr.operands[0]];
                    
                // ... other cases
            }
        }
        return Value();
    }
};
```

---

## Memory Management

### Object Allocation
```cpp
class Heap {
private:
    std::vector<Object*> objects;
    
public:
    Object* allocate_object() {
        auto* obj = new Object();
        objects.push_back(obj);
        return obj;
    }
    
    void collect() {
        mark_root_set();
        mark_objects();
        sweep();
    }
    
private:
    void mark_root_set() {
        // Mark objects reachable from roots
    }
    
    void mark_objects() {
        // Mark all reachable objects
    }
    
    void sweep() {
        // Free unmarked objects
    }
};
```

---

## Error Handling

### Error Types
```cpp
enum class ErrorType {
    LEXER_ERROR,
    PARSER_ERROR,
    SEMANTIC_ERROR,
    RUNTIME_ERROR,
    COMPILER_ERROR
};

struct CompilerError {
    ErrorType type;
    int line;
    int column;
    std::string message;
    
    CompilerError(ErrorType t, int ln, int col, const std::string& msg)
        : type(t), line(ln), column(col), message(msg) {}
};
```

### Error Reporting
```cpp
void report_error(ErrorType type, int line, int column, const std::string& message) {
    std::cerr << "Error [" << error_type_names[type] << ":" << line << ":" 
              << column << "]: " << message << std::endl;
    error_count++;
}
```

---

## Performance Considerations

### Optimization Strategies
1. **Inline Caching**: Cache property access
2. **Hidden Classes**: Optimize object layout
3. **Feedback Vectors**: Track type feedback
4. **JIT Compilation**: Compile hot functions

### Current Limitations
- Single-threaded
- Limited optimization
- Simple type system
- No async support

---

## Future Enhancements

### Phase 2 Features
- Multi-threading support
- Full async/await
- Generational GC
- Full type inference

### Phase 3 Features
- Standard library support
- Module system
- Profiling support
- Debugging support

---

## References

- [V8 Documentation](https://v8.dev/docs)
- [TC39 Specifications](https://tc39.es/)
- [Crafting Interpreters](https://craftinginterpreters.com/)
- [How V8 Works](https://v8.dev/blog/preparser)