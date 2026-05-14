# Go Compiler Architecture Documentation

## Overview

This document describes the architecture of our simplified Go compiler implementation.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Go Compiler                            │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Lexer      │  │   Parser     │  │ Semantic     │      │
│  │   (Tokenizer│  │   (AST       │  │ Analysis     │      │
│  │   )          │  │   Builder)   │  │              │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Token      │  │    AST       │  │  Type        │      │
│  │   Stream     │  │   Tree       │  │  Checking    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │  Code          │                    │
│                      │  Generator     │                    │
│                      │                │                    │
│                      │  IR / Assembly │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Compiler Phases

### 1. Lexer (Tokenizer)

**Purpose**: Convert source code into tokens

**Input**: Go source code as string

**Output**: Stream of tokens

**Token Types**:
```
- KEYWORD: if, else, for, func, var, const, etc.
- IDENT: variable/function names
- LITERAL: integers, floats, strings, booleans
- OPERATOR: +, -, *, /, =, ==, !=, etc.
- DELIMITER: {, }, (, ), [, ], ;, comma
- COMMENT: // and /* */
- EOF: end of file
```

**Structure**:
```c
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

typedef struct {
    Token* tokens;
    int count;
    int capacity;
} TokenStream;
```

### 2. Parser (AST Builder)

**Purpose**: Build Abstract Syntax Tree from tokens

**Input**: Token stream

**Output**: AST root node

**AST Node Types**:
```c
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VARIABLE_DECL,
    NODE_EXPRESSION,
    NODE_STATEMENT,
    NODE_IF_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_BINARY_EXPR,
    NODE_CALL_EXPR,
    NODE_LITERAL,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode* parent;
    struct ASTNode** children;
    int child_count;
    union {
        char* identifier;
        int int_value;
        double float_value;
        char* string_value;
        bool bool_value;
    } value;
} ASTNode;
```

### 3. Semantic Analyzer

**Purpose**: Check semantic correctness of AST

**Input**: AST root node

**Output**: Type-checked AST with symbol table

**Symbol Table**:
```c
typedef struct {
    char* name;
    Type* type;
    int scope_level;
    void* value;  // runtime value
} Symbol;

typedef struct {
    Symbol* symbols;
    int count;
    int capacity;
    int scope_level;
} SymbolTable;
```

### 4. Code Generator

**Purpose**: Generate assembly code from AST

**Input**: Type-checked AST

**Output**: Assembly code string

**IR Instructions**:
```c
typedef enum {
    IR_MOV,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_CALL,
    IR_RET,
    IR_JMP,
    IR_JZ,
    IR_LABEL,
} IRInstructionType;

typedef struct {
    IRInstructionType type;
    char* dest;
    char* src1;
    char* src2;
} IRInstruction;
```

---

## Data Structures

### Token Structure
```c
typedef enum {
    TOKEN_EOF,
    TOKEN_KEYWORD,
    TOKEN_IDENT,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_OPERATOR,
    TOKEN_DELIMITER,
    TOKEN_COMMENT,
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;
```

### AST Node Structure
```c
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VARIABLE_DECL,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_IDENT,
    NODE_INT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_IF_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_BLOCK_STMT,
} NodeType;

typedef struct ASTNode {
    NodeType type;
    struct ASTNode* parent;
    struct ASTNode** children;
    int child_count;
    union {
        char* identifier;
        int int_value;
        double float_value;
        char* string_value;
        bool bool_value;
    } value;
    Type* type;  // semantic type
} ASTNode;
```

### Type Structure
```c
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_ARRAY,
    TYPE_STRUCT,
    TYPE_FUNCTION,
    TYPE_VOID,
} TypeKind;

typedef struct Type {
    TypeKind kind;
    union {
        int array_size;
        struct {
            struct Type** params;
            int param_count;
            struct Type* return_type;
        } function;
    } details;
} Type;
```

---

## Lexer Implementation

### State Machine
```
Start
  │
  ├─► Letter/Underscore ──► Identifier/Keyword
  │
  ├─► Digit ──► Integer/Float
  │
  ├─► '"' ──► String
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
```c
Token* tokenize(const char* source) {
    TokenStream* stream = create_token_stream();
    
    while (*source != '\0') {
        // Skip whitespace
        if (is_whitespace(*source)) {
            source++;
            continue;
        }
        
        // Handle comments
        if (*source == '/' && *(source+1) == '/') {
            skip_line_comment(&source);
            continue;
        }
        
        // Handle identifiers/keywords
        if (is_alpha(*source) || *source == '_') {
            Token* token = parse_identifier(&source);
            add_token(stream, token);
            continue;
        }
        
        // Handle numbers
        if (is_digit(*source)) {
            Token* token = parse_number(&source);
            add_token(stream, token);
            continue;
        }
        
        // Handle strings
        if (*source == '"') {
            Token* token = parse_string(&source);
            add_token(stream, token);
            continue;
        }
        
        // Handle operators and delimiters
        Token* token = parse_operator_or_delimiter(&source);
        add_token(stream, token);
    }
    
    add_token(stream, create_token(TOKEN_EOF, "", 0, 0));
    return stream->tokens;
}
```

---

## Parser Implementation

### Recursive Descent Parsing
```c
ASTNode* parse_program(TokenStream* tokens) {
    ASTNode* program = create_node(NODE_PROGRAM);
    
    // Parse package clause
    expect_keyword(tokens, "package");
    expect_token(tokens, TOKEN_IDENT);
    
    // Parse imports (optional)
    while (peek_token(tokens)->type == TOKEN_KEYWORD && 
           strcmp(peek_token(tokens)->lexeme, "import") == 0) {
        parse_import(tokens);
    }
    
    // Parse declarations
    while (peek_token(tokens)->type != TOKEN_EOF) {
        ASTNode* decl = parse_declaration(tokens);
        add_child(program, decl);
    }
    
    return program;
}

ASTNode* parse_function_decl(TokenStream* tokens) {
    expect_keyword(tokens, "func");
    
    ASTNode* func = create_node(NODE_FUNCTION_DECL);
    
    // Parse function name
    expect_token(tokens, TOKEN_IDENT);
    func->value.identifier = tokens->current->lexeme;
    
    // Parse parameters
    parse_parameters(tokens, func);
    
    // Parse return type (optional)
    parse_return_type(tokens, func);
    
    // Parse body
    parse_block_stmt(tokens, func);
    
    return func;
}
```

---

## Semantic Analysis

### Symbol Table Operations
```c
void symbol_table_enter_scope(SymbolTable* table) {
    table->scope_level++;
}

void symbol_table_exit_scope(SymbolTable* table) {
    table->scope_level--;
    // Remove symbols from current scope
}

void symbol_table_add(SymbolTable* table, char* name, Type* type) {
    Symbol* symbol = malloc(sizeof(Symbol));
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->scope_level = table->scope_level;
    add_symbol(table, symbol);
}

Symbol* symbol_table_lookup(SymbolTable* table, char* name) {
    // Search from current scope to global
    for (int i = table->count - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i].name, name) == 0 &&
            table->symbols[i].scope_level <= table->scope_level) {
            return &table->symbols[i];
        }
    }
    return NULL;
}
```

### Type Checking
```c
Type* check_expression(ASTNode* node, SymbolTable* table) {
    switch (node->type) {
        case NODE_BINARY_EXPR:
            Type* left = check_expression(node->left, table);
            Type* right = check_expression(node->right, table);
            if (!types_compatible(left, right)) {
                error("Type mismatch in binary expression");
                return &type_error;
            }
            return left;
        
        case NODE_IDENT:
            Symbol* symbol = symbol_table_lookup(table, node->value.identifier);
            if (!symbol) {
                error("Undefined variable: %s", node->value.identifier);
                return &type_error;
            }
            return symbol->type;
        
        case NODE_INT_LITERAL:
            return &type_int;
        
        // ... other cases
    }
}
```

---

## Code Generation

### IR Generation
```c
void generate_ir(ASTNode* node, IRBuilder* builder) {
    switch (node->type) {
        case NODE_BINARY_EXPR:
            generate_ir(node->left, builder);
            generate_ir(node->right, builder);
            
            char* result = generate_temp(builder);
            char* left = pop_temp(builder);
            char* right = pop_temp(builder);
            
            add_ir_instruction(builder, IR_ADD, result, left, right);
            push_temp(builder, result);
            break;
        
        case NODE_IDENT:
            // Load variable from memory
            char* result = generate_temp(builder);
            add_ir_instruction(builder, IR_MOV, result, node->value.identifier, NULL);
            push_temp(builder, result);
            break;
        
        // ... other cases
    }
}
```

### Assembly Generation
```c
void generate_asm(IRInstruction* ir, FILE* out) {
    switch (ir->type) {
        case IR_MOV:
            fprintf(out, "  movq %s, %s\n", ir->src1, ir->dest);
            break;
        
        case IR_ADD:
            fprintf(out, "  addq %s, %s\n", ir->src1, ir->dest);
            break;
        
        case IR_CALL:
            fprintf(out, "  call %s\n", ir->dest);
            break;
        
        case IR_RET:
            fprintf(out, "  ret\n");
            break;
    }
}
```

---

## Memory Management

### AST Memory
```c
ASTNode* create_node(NodeType type) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->parent = NULL;
    node->children = NULL;
    node->child_count = 0;
    memset(&node->value, 0, sizeof(node->value));
    node->type = NULL;
    return node;
}

void add_child(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children, 
                               (parent->child_count + 1) * sizeof(ASTNode*));
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    free(node->children);
    free(node);
}
```

---

## Error Handling

### Error Types
```c
typedef enum {
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_SEMANTIC,
    ERROR_CODEGEN,
} ErrorType;

typedef struct {
    ErrorType type;
    int line;
    int column;
    char* message;
} CompilerError;
```

### Error Reporting
```c
void report_error(ErrorType type, int line, int column, const char* message) {
    fprintf(stderr, "Error [%s:%d:%d]: %s\n", 
            error_type_names[type], line, column, message);
    error_count++;
}
```

---

## Performance Considerations

### Optimization Strategies
1. **Token Reuse**: Reuse token strings
2. **AST Pooling**: Use memory pools for AST nodes
3. **Lazy Evaluation**: Delay type checking
4. **IR CSE**: Common subexpression elimination

### Current Limitations
- Single-threaded
- No JIT compilation
- Limited optimization
- Simple type system

---

## Future Enhancements

### Phase 2 Features
- Multi-threading support
- JIT compilation
- Generational GC
- Full type inference

### Phase 3 Features
- Standard library support
- Package management
- Profiling support
- Debugging support

---

## References

- [Go Language Spec](https://go.dev/ref/spec)
- [Crafting Interpreters](https://craftinginterpreters.com/)
- [Let's Build a Compiler](https://github.com/kanaka/mal)
- [x86-64 Assembly](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)