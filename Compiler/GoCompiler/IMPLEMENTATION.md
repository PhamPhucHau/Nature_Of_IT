# Go Compiler Implementation Guide

## Getting Started

### Prerequisites
- C programming knowledge
- Understanding of data structures
- Basic knowledge of Go
- GCC or Clang compiler

### Project Structure
```
GoCompiler/
├── src/
│   ├── lexer/
│   │   ├── lexer.c
│   │   ├── lexer.h
│   │   ├── token.c
│   │   └── token.h
│   ├── parser/
│   │   ├── parser.c
│   │   ├── parser.h
│   │   ├── ast.c
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
├── test/
└── makefile
```

---

## Phase 1: Token Definitions

Create `src/lexer/token.h`:

```c
#ifndef GO_COMPILER_TOKEN_H
#define GO_COMPILER_TOKEN_H

#include <stdint.h>

// Token types
typedef enum {
    TOKEN_EOF,
    TOKEN_ILLEGAL,
    
    // Keywords
    TOKEN_PACKAGE,
    TOKEN_IMPORT,
    TOKEN_FUNC,
    TOKEN_VAR,
    TOKEN_CONST,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_STRING,
    TOKEN_BOOL,
    TOKEN_FLOAT,
    TOKEN_STRUCT,
    TOKEN_INTERFACE,
    TOKEN_MAKE,
    TOKEN_NEW,
    TOKEN_TYPE,
    TOKEN_CONST,
    
    // Identifiers and literals
    TOKEN_IDENT,
    TOKEN_INT_LIT,
    TOKEN_FLOAT_LIT,
    TOKEN_STRING_LIT,
    TOKEN_BOOL_LIT,
    
    // Operators
    TOKEN_ASSIGN,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_XOR,
    TOKEN_NOT,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_EQ,
    TOKEN_NEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LE,
    TOKEN_GE,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    
    // Delimiters
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_ARROW,
    TOKEN_BLANK,
    
    // Comments
    TOKEN_COMMENT_LINE,
    TOKEN_COMMENT_BLOCK,
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

// Function declarations
Token* create_token(TokenType type, const char* lexeme, int line, int column);
void free_token(Token* token);
const char* token_type_name(TokenType type);

#endif
```

Create `src/lexer/token.c`:

```c
#include "token.h"
#include <stdlib.h>
#include <string.h>

Token* create_token(TokenType type, const char* lexeme, int line, int column) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->type = type;
    token->lexeme = lexeme ? strdup(lexeme) : NULL;
    token->line = line;
    token->column = column;
    
    return token;
}

void free_token(Token* token) {
    if (!token) return;
    if (token->lexeme) free(token->lexeme);
    free(token);
}

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "EOF";
        case TOKEN_ILLEGAL: return "ILLEGAL";
        case TOKEN_PACKAGE: return "PACKAGE";
        case TOKEN_IMPORT: return "IMPORT";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_VAR: return "VAR";
        case TOKEN_CONST: return "CONST";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_INT: return "INT";
        case TOKEN_STRING: return "STRING";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_IDENT: return "IDENT";
        case TOKEN_INT_LIT: return "INT_LIT";
        case TOKEN_FLOAT_LIT: return "FLOAT_LIT";
        case TOKEN_STRING_LIT: return "STRING_LIT";
        case TOKEN_BOOL_LIT: return "BOOL_LIT";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_ADD: return "ADD";
        case TOKEN_SUB: return "SUB";
        case TOKEN_MUL: return "MUL";
        case TOKEN_DIV: return "DIV";
        case TOKEN_MOD: return "MOD";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NEQ: return "NEQ";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LE: return "LE";
        case TOKEN_GE: return "GE";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_DOT: return "DOT";
        default: return "UNKNOWN";
    }
}
```

---

## Phase 2: Lexer Implementation

Create `src/lexer/lexer.h`:

```c
#ifndef GO_COMPILER_LEXER_H
#define GO_COMPILER_LEXER_H

#include "token.h"

typedef struct {
    const char* input;
    int position;
    int read_position;
    int line;
    int column;
    char ch;
} Lexer;

// Function declarations
Lexer* lexer_create(const char* input);
void lexer_destroy(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
Token* lexer_peek_token(Lexer* lexer);

#endif
```

Create `src/lexer/lexer.c`:

```c
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Keyword table
typedef struct {
    const char* keyword;
    TokenType token_type;
} Keyword;

static Keyword keywords[] = {
    {"package", TOKEN_PACKAGE},
    {"import", TOKEN_IMPORT},
    {"func", TOKEN_FUNC},
    {"var", TOKEN_VAR},
    {"const", TOKEN_CONST},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"int", TOKEN_INT},
    {"string", TOKEN_STRING},
    {"bool", TOKEN_BOOL},
    {"float", TOKEN_FLOAT},
    {"struct", TOKEN_STRUCT},
    {"interface", TOKEN_INTERFACE},
    {"make", TOKEN_MAKE},
    {"new", TOKEN_NEW},
    {"type", TOKEN_TYPE},
    {NULL, TOKEN_IDENT}
};

static bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static void read_char(Lexer* lexer) {
    if (lexer->read_position >= (int)strlen(lexer->input)) {
        lexer->ch = '\0';
    } else {
        lexer->ch = lexer->input[lexer->read_position];
    }
    lexer->position = lexer->read_position;
    if (lexer->ch == '\n') {
        lexer->line++;
        lexer->column = 0;
    } else {
        lexer->column++;
    }
    lexer->read_position++;
}

static char peek_char(Lexer* lexer) {
    if (lexer->read_position >= (int)strlen(lexer->input)) {
        return '\0';
    }
    return lexer->input[lexer->read_position];
}

static void skip_whitespace(Lexer* lexer) {
    while (lexer->ch == ' ' || lexer->ch == '\t' || 
           lexer->ch == '\n' || lexer->ch == '\r') {
        read_char(lexer);
    }
}

static Token* read_identifier(Lexer* lexer) {
    int start = lexer->position;
    while (is_letter(lexer->ch) || is_digit(lexer->ch)) {
        read_char(lexer);
    }
    int length = lexer->position - start;
    char* identifier = strndup(lexer->input + start, length);
    
    // Check if it's a keyword
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(keywords[i].keyword, identifier) == 0) {
            free(identifier);
            return create_token(keywords[i].token_type, keywords[i].keyword, 
                              lexer->line, lexer->column - length);
        }
    }
    
    Token* token = create_token(TOKEN_IDENT, identifier, lexer->line, lexer->column - length);
    free(identifier);
    return token;
}

static Token* read_number(Lexer* lexer) {
    int start = lexer->position;
    while (is_digit(lexer->ch)) {
        read_char(lexer);
    }
    
    bool is_float = false;
    if (lexer->ch == '.') {
        is_float = true;
        read_char(lexer);
        while (is_digit(lexer->ch)) {
            read_char(lexer);
        }
    }
    
    int length = lexer->position - start;
    char* literal = strndup(lexer->input + start, length);
    
    TokenType type = is_float ? TOKEN_FLOAT_LIT : TOKEN_INT_LIT;
    Token* token = create_token(type, literal, lexer->line, lexer->column - length);
    free(literal);
    return token;
}

static Token* read_string(Lexer* lexer) {
    int start = lexer->position + 1;  // Skip opening quote
    while (lexer->ch != '"' && lexer->ch != '\0') {
        if (lexer->ch == '\\' && lexer->read_position < (int)strlen(lexer->input)) {
            read_char(lexer);  // Skip escaped character
        }
        read_char(lexer);
    }
    
    int length = lexer->position - start;
    char* literal = strndup(lexer->input + start, length);
    
    Token* token = create_token(TOKEN_STRING_LIT, literal, lexer->line, lexer->column - length - 1);
    free(literal);
    
    if (lexer->ch == '"') {
        read_char(lexer);  // Skip closing quote
    }
    
    return token;
}

static Token* read_operator(Lexer* lexer) {
    char first = lexer->ch;
    read_char(lexer);
    
    // Check for two-character operators
    if (lexer->ch == '=') {
        read_char(lexer);
        switch (first) {
            case '=': return create_token(TOKEN_EQ, "==", lexer->line, lexer->column - 2);
            case '!': return create_token(TOKEN_NEQ, "!=", lexer->line, lexer->column - 2);
            case '<': return create_token(TOKEN_LE, "<=", lexer->line, lexer->column - 2);
            case '>': return create_token(TOKEN_GE, ">=", lexer->line, lexer->column - 2);
            case '&': return create_token(TOKEN_AND, "&=", lexer->line, lexer->column - 2);
            case '|': return create_token(TOKEN_OR, "|=", lexer->line, lexer->column - 2);
        }
    }
    
    if (lexer->ch == '&' && first == '&') {
        read_char(lexer);
        return create_token(TOKEN_AND_AND, "&&", lexer->line, lexer->column - 2);
    }
    
    if (lexer->ch == '|' && first == '|') {
        read_char(lexer);
        return create_token(TOKEN_OR_OR, "||", lexer->line, lexer->column - 2);
    }
    
    if (lexer->ch == '<' && first == '<') {
        read_char(lexer);
        return create_token(TOKEN_LSHIFT, "<<", lexer->line, lexer->column - 2);
    }
    
    if (lexer->ch == '>' && first == '>') {
        read_char(lexer);
        return create_token(TOKEN_RSHIFT, ">>", lexer->line, lexer->column - 2);
    }
    
    switch (first) {
        case '+': return create_token(TOKEN_ADD, "+", lexer->line, lexer->column - 1);
        case '-': return create_token(TOKEN_SUB, "-", lexer->line, lexer->column - 1);
        case '*': return create_token(TOKEN_MUL, "*", lexer->line, lexer->column - 1);
        case '/': return create_token(TOKEN_DIV, "/", lexer->line, lexer->column - 1);
        case '%': return create_token(TOKEN_MOD, "%", lexer->line, lexer->column - 1);
        case '=': return create_token(TOKEN_ASSIGN, "=", lexer->line, lexer->column - 1);
        case '!': return create_token(TOKEN_NOT, "!", lexer->line, lexer->column - 1);
        case '&': return create_token(TOKEN_AND, "&", lexer->line, lexer->column - 1);
        case '|': return create_token(TOKEN_OR, "|", lexer->line, lexer->column - 1);
        case '^': return create_token(TOKEN_XOR, "^", lexer->line, lexer->column - 1);
        case '<': return create_token(TOKEN_LT, "<", lexer->line, lexer->column - 1);
        case '>': return create_token(TOKEN_GT, ">", lexer->line, lexer->column - 1);
        case '.': return create_token(TOKEN_DOT, ".", lexer->line, lexer->column - 1);
        case ':': return create_token(TOKEN_COLON, ":", lexer->line, lexer->column - 1);
        case ';': return create_token(TOKEN_SEMICOLON, ";", lexer->line, lexer->column - 1);
        case '(': return create_token(TOKEN_LPAREN, "(", lexer->line, lexer->column - 1);
        case ')': return create_token(TOKEN_RPAREN, ")", lexer->line, lexer->column - 1);
        case '{': return create_token(TOKEN_LBRACE, "{", lexer->line, lexer->column - 1);
        case '}': return create_token(TOKEN_RBRACE, "}", lexer->line, lexer->column - 1);
        case '[': return create_token(TOKEN_LBRACKET, "[", lexer->line, lexer->column - 1);
        case ']': return create_token(TOKEN_RBRACKET, "]", lexer->line, lexer->column - 1);
        case ',': return create_token(TOKEN_COMMA, ",", lexer->line, lexer->column - 1);
        case '~': return create_token(TOKEN_BLANK, "~", lexer->line, lexer->column - 1);
    }
    
    return create_token(TOKEN_ILLEGAL, &first, lexer->line, lexer->column - 1);
}

Lexer* lexer_create(const char* input) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->input = input;
    lexer->position = 0;
    lexer->read_position = 0;
    lexer->line = 1;
    lexer->column = 0;
    lexer->ch = '\0';
    
    read_char(lexer);
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    if (!lexer) return;
    free(lexer);
}

Token* lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    Token* token;
    
    if (is_letter(lexer->ch)) {
        token = read_identifier(lexer);
    } else if (is_digit(lexer->ch)) {
        token = read_number(lexer);
    } else if (lexer->ch == '"') {
        token = read_string(lexer);
    } else if (lexer->ch == '/' && peek_char(lexer) == '/') {
        // Skip line comment
        while (lexer->ch != '\n' && lexer->ch != '\0') {
            read_char(lexer);
        }
        return lexer_next_token(lexer);
    } else if (lexer->ch == '/' && peek_char(lexer) == '*') {
        // Skip block comment
        read_char(lexer);  // Skip *
        read_char(lexer);  // Skip /
        while (!(lexer->ch == '*' && peek_char(lexer) == '/')) {
            if (lexer->ch == '\0') break;
            read_char(lexer);
        }
        if (lexer->ch == '*') {
            read_char(lexer);  // Skip *
            read_char(lexer);  // Skip /
        }
        return lexer_next_token(lexer);
    } else {
        token = read_operator(lexer);
    }
    
    return token;
}

Token* lexer_peek_token(Lexer* lexer) {
    Token* current = lexer_next_token(lexer);
    return current;
}
```

---

## Phase 3: AST Node Definitions

Create `src/parser/ast.h`:

```c
#ifndef GO_COMPILER_AST_H
#define GO_COMPILER_AST_H

#include <stdbool.h>

// AST node types
typedef enum {
    NODE_PROGRAM,
    NODE_FUNCTION_DECL,
    NODE_VARIABLE_DECL,
    NODE_EXPRESSION,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_CALL_EXPR,
    NODE_IDENT,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_STRING_LITERAL,
    NODE_BOOL_LITERAL,
    NODE_IF_STMT,
    NODE_FOR_STMT,
    NODE_RETURN_STMT,
    NODE_BLOCK_STMT,
    NODE_IMPORT_DECL,
    NODE_PACKAGE_DECL,
} NodeType;

// AST node structure
typedef struct ASTNode {
    NodeType type;
    struct ASTNode* parent;
    struct ASTNode** children;
    int child_count;
    int capacity;
    union {
        char* identifier;
        int int_value;
        double float_value;
        char* string_value;
        bool bool_value;
    } value;
} ASTNode;

// Function declarations
ASTNode* create_node(NodeType type);
void add_child(ASTNode* parent, ASTNode* child);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);

#endif
```

Create `src/parser/ast.c`:

```c
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ASTNode* create_node(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->parent = NULL;
    node->children = NULL;
    node->child_count = 0;
    node->capacity = 0;
    memset(&node->value, 0, sizeof(node->value));
    
    return node;
}

void add_child(ASTNode* parent, ASTNode* child) {
    if (!parent || !child) return;
    
    if (parent->child_count >= parent->capacity) {
        parent->capacity = parent->capacity == 0 ? 4 : parent->capacity * 2;
        parent->children = (ASTNode**)realloc(parent->children, 
                                              parent->capacity * sizeof(ASTNode*));
    }
    
    parent->children[parent->child_count++] = child;
    child->parent = parent;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    
    for (int i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    
    if (node->children) {
        free(node->children);
    }
    
    if (node->type == NODE_IDENT || node->type == NODE_STRING_LITERAL) {
        free(node->value.identifier);
    }
    
    free(node);
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case NODE_PROGRAM:
            printf("Program\n");
            break;
        case NODE_FUNCTION_DECL:
            printf("FunctionDecl: %s\n", node->value.identifier);
            break;
        case NODE_VARIABLE_DECL:
            printf("VariableDecl: %s\n", node->value.identifier);
            break;
        case NODE_IDENT:
            printf("Ident: %s\n", node->value.identifier);
            break;
        case NODE_INT_LITERAL:
            printf("IntLiteral: %d\n", node->value.int_value);
            break;
        case NODE_FLOAT_LITERAL:
            printf("FloatLiteral: %g\n", node->value.float_value);
            break;
        case NODE_STRING_LITERAL:
            printf("StringLiteral: %s\n", node->value.string_value);
            break;
        case NODE_BOOL_LITERAL:
            printf("BoolLiteral: %s\n", node->value.bool_value ? "true" : "false");
            break;
        case NODE_BINARY_EXPR:
            printf("BinaryExpr\n");
            break;
        case NODE_UNARY_EXPR:
            printf("UnaryExpr\n");
            break;
        case NODE_CALL_EXPR:
            printf("CallExpr: %s\n", node->value.identifier);
            break;
        case NODE_IF_STMT:
            printf("IfStmt\n");
            break;
        case NODE_FOR_STMT:
            printf("ForStmt\n");
            break;
        case NODE_RETURN_STMT:
            printf("ReturnStmt\n");
            break;
        case NODE_BLOCK_STMT:
            printf("BlockStmt\n");
            break;
        case NODE_IMPORT_DECL:
            printf("ImportDecl\n");
            break;
        case NODE_PACKAGE_DECL:
            printf("PackageDecl\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    
    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], indent + 1);
    }
}
```

---

## Phase 4: Parser Implementation

Create `src/parser/parser.h`:

```c
#ifndef GO_COMPILER_PARSER_H
#define GO_COMPILER_PARSER_H

#include "../lexer/token.h"
#include "../parser/ast.h"

typedef struct {
    Token** tokens;
    int count;
    int position;
    ASTNode* current;
} Parser;

// Function declarations
Parser* parser_create(Token** tokens, int count);
void parser_destroy(Parser* parser);
ASTNode* parse_program(Parser* parser);
ASTNode* parse_function_decl(Parser* parser);
ASTNode* parse_variable_decl(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_statement(Parser* parser);

#endif
```

Create `src/parser/parser.c`:

```c
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static Token* current_token(Parser* parser) {
    if (parser->position >= parser->count) {
        return NULL;
    }
    return parser->tokens[parser->position];
}

static Token* peek_token(Parser* parser) {
    if (parser->position + 1 >= parser->count) {
        return NULL;
    }
    return parser->tokens[parser->position + 1];
}

static void next_token(Parser* parser) {
    parser->position++;
}

static bool match_token(Parser* parser, TokenType type) {
    Token* token = current_token(parser);
    return token && token->type == type;
}

static bool expect_token(Parser* parser, TokenType type) {
    if (match_token(parser, type)) {
        next_token(parser);
        return true;
    }
    fprintf(stderr, "Expected token type %s, got %s\n", 
            token_type_name(type), 
            token_type_name(current_token(parser)->type));
    return false;
}

static bool expect_keyword(Parser* parser, const char* keyword) {
    Token* token = current_token(parser);
    if (token && token->type == TOKEN_IDENT && strcmp(token->lexeme, keyword) == 0) {
        next_token(parser);
        return true;
    }
    fprintf(stderr, "Expected keyword '%s', got '%s'\n", keyword, token->lexeme);
    return false;
}

Parser* parser_create(Token** tokens, int count) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->tokens = tokens;
    parser->count = count;
    parser->position = 0;
    parser->current = NULL;
    
    return parser;
}

void parser_destroy(Parser* parser) {
    if (!parser) return;
    free(parser);
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* program = create_node(NODE_PROGRAM);
    
    // Parse package declaration
    if (expect_keyword(parser, "package")) {
        ASTNode* pkg = create_node(NODE_PACKAGE_DECL);
        if (expect_token(parser, TOKEN_IDENT)) {
            pkg->value.identifier = strdup(current_token(parser)->lexeme);
        }
        add_child(program, pkg);
    }
    
    // Parse imports
    while (match_token(parser, TOKEN_IDENT) && 
           strcmp(current_token(parser)->lexeme, "import") == 0) {
        next_token(parser);  // Skip 'import'
        if (expect_token(parser, TOKEN_STRING_LIT)) {
            ASTNode* imp = create_node(NODE_IMPORT_DECL);
            imp->value.identifier = strdup(current_token(parser)->lexeme);
            add_child(program, imp);
        }
    }
    
    // Parse declarations
    while (current_token(parser) && 
           current_token(parser)->type != TOKEN_EOF) {
        if (match_token(parser, TOKEN_IDENT) && 
            strcmp(current_token(parser)->lexeme, "func") == 0) {
            ASTNode* func = parse_function_decl(parser);
            add_child(program, func);
        } else if (match_token(parser, TOKEN_IDENT) && 
                   strcmp(current_token(parser)->lexeme, "var") == 0) {
            ASTNode* var = parse_variable_decl(parser);
            add_child(program, var);
        } else {
            next_token(parser);
        }
    }
    
    return program;
}

ASTNode* parse_function_decl(Parser* parser) {
    expect_keyword(parser, "func");
    
    ASTNode* func = create_node(NODE_FUNCTION_DECL);
    
    // Parse function name
    if (expect_token(parser, TOKEN_IDENT)) {
        func->value.identifier = strdup(current_token(parser)->lexeme);
    }
    
    // Parse parameters
    if (expect_token(parser, TOKEN_LPAREN)) {
        while (!match_token(parser, TOKEN_RPAREN) && 
               current_token(parser)->type != TOKEN_EOF) {
            if (expect_token(parser, TOKEN_IDENT)) {
                ASTNode* param = create_node(NODE_IDENT);
                param->value.identifier = strdup(current_token(parser)->lexeme);
                add_child(func, param);
            }
            if (match_token(parser, TOKEN_COMMA)) {
                next_token(parser);
            }
        }
        expect_token(parser, TOKEN_RPAREN);
    }
    
    // Parse return type (optional)
    if (match_token(parser, TOKEN_IDENT)) {
        ASTNode* ret = create_node(NODE_IDENT);
        ret->value.identifier = strdup(current_token(parser)->lexeme);
        add_child(func, ret);
    }
    
    // Parse function body
    if (expect_token(parser, TOKEN_LBRACE)) {
        ASTNode* body = create_node(NODE_BLOCK_STMT);
        while (!match_token(parser, TOKEN_RBRACE) && 
               current_token(parser)->type != TOKEN_EOF) {
            ASTNode* stmt = parse_statement(parser);
            if (stmt) {
                add_child(body, stmt);
            }
        }
        expect_token(parser, TOKEN_RBRACE);
        add_child(func, body);
    }
    
    return func;
}

ASTNode* parse_variable_decl(Parser* parser) {
    expect_keyword(parser, "var");
    
    ASTNode* var = create_node(NODE_VARIABLE_DECL);
    
    // Parse variable name
    if (expect_token(parser, TOKEN_IDENT)) {
        var->value.identifier = strdup(current_token(parser)->lexeme);
    }
    
    // Parse type (optional)
    if (match_token(parser, TOKEN_IDENT)) {
        ASTNode* type = create_node(NODE_IDENT);
        type->value.identifier = strdup(current_token(parser)->lexeme);
        add_child(var, type);
    }
    
    // Parse value
    if (match_token(parser, TOKEN_ASSIGN)) {
        next_token(parser);
        ASTNode* value = parse_expression(parser);
        add_child(var, value);
    }
    
    return var;
}

ASTNode* parse_expression(Parser* parser) {
    // Simple expression parser for now
    ASTNode* left = NULL;
    
    if (match_token(parser, TOKEN_IDENT)) {
        left = create_node(NODE_IDENT);
        left->value.identifier = strdup(current_token(parser)->lexeme);
        next_token(parser);
    } else if (match_token(parser, TOKEN_INT_LIT)) {
        left = create_node(NODE_INT_LITERAL);
        left->value.int_value = atoi(current_token(parser)->lexeme);
        next_token(parser);
    } else if (match_token(parser, TOKEN_STRING_LIT)) {
        left = create_node(NODE_STRING_LITERAL);
        left->value.string_value = strdup(current_token(parser)->lexeme);
        next_token(parser);
    }
    
    // Handle binary expressions
    if (match_token(parser, TOKEN_ADD) || match_token(parser, TOKEN_SUB) ||
        match_token(parser, TOKEN_MUL) || match_token(parser, TOKEN_DIV)) {
        TokenType op = current_token(parser)->type;
        next_token(parser);
        ASTNode* right = parse_expression(parser);
        
        ASTNode* expr = create_node(NODE_BINARY_EXPR);
        expr->value.int_value = op;
        add_child(expr, left);
        add_child(expr, right);
        return expr;
    }
    
    return left;
}

ASTNode* parse_statement(Parser* parser) {
    if (match_token(parser, TOKEN_IDENT) && 
        strcmp(current_token(parser)->lexeme, "return") == 0) {
        next_token(parser);
        ASTNode* ret = create_node(NODE_RETURN_STMT);
        ASTNode* value = parse_expression(parser);
        add_child(ret, value);
        return ret;
    }
    
    return parse_expression(parser);
}
```

---

## Phase 5: Main Program

Create `src/main.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    return content;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <go_file>\n", argv[0]);
        return 1;
    }
    
    printf("=== Go Compiler ===\n");
    printf("Compiling: %s\n\n", argv[1]);
    
    // Read source file
    char* source = read_file(argv[1]);
    if (!source) {
        return 1;
    }
    
    // Lexical analysis
    printf("Phase 1: Lexical Analysis\n");
    Lexer* lexer = lexer_create(source);
    Token** tokens = NULL;
    int token_count = 0;
    int token_capacity = 0;
    
    Token* token;
    while ((token = lexer_next_token(lexer)) && token->type != TOKEN_EOF) {
        if (token_count >= token_capacity) {
            token_capacity = token_capacity == 0 ? 64 : token_capacity * 2;
            tokens = (Token**)realloc(tokens, token_capacity * sizeof(Token*));
        }
        tokens[token_count++] = token;
    }
    tokens[token_count++] = token;  // Add EOF token
    
    printf("  Found %d tokens\n\n", token_count);
    
    // Parse
    printf("Phase 2: Syntax Analysis\n");
    Parser* parser = parser_create(tokens, token_count);
    ASTNode* ast = parse_program(parser);
    
    printf("AST:\n");
    print_ast(ast, 0);
    printf("\n");
    
    // Cleanup
    lexer_destroy(lexer);
    parser_destroy(parser);
    free_ast(ast);
    
    for (int i = 0; i < token_count; i++) {
        free_token(tokens[i]);
    }
    free(tokens);
    free(source);
    
    printf("Compilation completed!\n");
    return 0;
}
```

---

## Phase 6: Makefile

Create `makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = goc
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/lexer/token.c \
          $(SRC_DIR)/lexer/lexer.c \
          $(SRC_DIR)/parser/ast.c \
          $(SRC_DIR)/parser/parser.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) examples/hello.go

test: $(TARGET)
	@echo "=== Running Tests ==="
	@./$(TARGET) examples/hello.go
	@./$(TARGET) examples/factorial.go

setup:
	@echo "Creating example Go files..."
	@mkdir -p examples
	@echo 'package main' > examples/hello.go
	@echo '' >> examples/hello.go
	@echo 'func main() {' >> examples/hello.go
	@echo '    println("Hello, Go Compiler!")' >> examples/hello.go
	@echo '}' >> examples/hello.go
	@echo 'package main' > examples/factorial.go
	@echo '' >> examples/factorial.go
	@echo 'func factorial(n int) int {' >> examples/factorial.go
	@echo '    if n <= 1 {' >> examples/factorial.go
	@echo '        return 1' >> examples/factorial.go
	@echo '    }' >> examples/factorial.go
	@echo '    return n * factorial(n-1)' >> examples/factorial.go
	@echo '}' >> examples/factorial.go
	@echo '' >> examples/factorial.go
	@echo 'func main() {' >> examples/factorial.go
	@echo '    println("5! =", factorial(5))' >> examples/factorial.go
	@echo '}' >> examples/factorial.go
```

---

## Build and Run

```bash
cd GoCompiler
make setup      # Create example files
make            # Build
./goc examples/hello.go
```

---

## Next Steps

1. Implement more expression types
2. Add semantic analysis
3. Implement code generation
4. Add type checking
5. Generate assembly code