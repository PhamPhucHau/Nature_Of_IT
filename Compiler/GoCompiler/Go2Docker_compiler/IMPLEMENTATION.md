# Go to Docker Compiler Implementation Guide

## Getting Started

### Prerequisites
- C programming knowledge
- Understanding of data structures
- Basic knowledge of Go and Docker
- GCC or Clang compiler

### Project Structure
```
Go2Docker_compiler/
├── src/
│   ├── parser/
│   │   ├── go_parser.c
│   │   ├── go_parser.h
│   │   └── ast.c
│   │   └── ast.h
│   ├── analyzer/
│   │   ├── dependency.c
│   │   ├── dependency.h
│   │   └── requirements.c
│   │   └── requirements.h
│   ├── generator/
│   │   ├── dockerfile.c
│   │   ├── dockerfile.h
│   │   ├── compose.c
│   │   └── compose.h
│   └── main.c
├── examples/
├── templates/
└── makefile
```

---

## Phase 1: Token Definitions

Create `src/parser/token.h`:

```c
#ifndef GO2DOCKER_TOKEN_H
#define GO2DOCKER_TOKEN_H

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

Create `src/parser/token.c`:

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

## Phase 2: Go Parser

Create `src/parser/go_parser.h`:

```c
#ifndef GO2DOCKER_GO_PARSER_H
#define GO2DOCKER_GO_PARSER_H

#include "token.h"
#include "ast.h"

// Parser structure
typedef struct {
    Token** tokens;
    int count;
    int position;
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

Create `src/parser/go_parser.c`:

```c
#include "go_parser.h"
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
    
    return parser;
}

void parser_destroy(Parser* parser) {
    if (!parser) return;
    free(parser);
}

ASTNode* parse_program(Parser* parser) {
    ASTNode* program = create_node(NODE_PROGRAM);
    
    // Parse package clause
    if (expect_keyword(parser, "package")) {
        ASTNode* pkg = create_node(NODE_PACKAGE_CLAUSE);
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

## Phase 3: AST Node Definitions

Create `src/parser/ast.h`:

```c
#ifndef GO2DOCKER_AST_H
#define GO2DOCKER_AST_H

#include <stdbool.h>

// AST node types
typedef enum {
    NODE_PROGRAM,
    NODE_PACKAGE_CLAUSE,
    NODE_IMPORT_DECL,
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
        case NODE_PACKAGE_CLAUSE:
            printf("PackageClause\n");
            break;
        case NODE_IMPORT_DECL:
            printf("ImportDecl: %s\n", node->value.identifier);
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

## Phase 4: Dependency Analyzer

Create `src/analyzer/dependency.h`:

```c
#ifndef GO2DOCKER_DEPENDENCY_H
#define GO2DOCKER_DEPENDENCY_H

#include "../parser/ast.h"

typedef enum {
    DEP_STANDARD,
    DEP_EXTERNAL,
    DEP_SYSTEM,
    DEP_NETWORK,
    DEP_VOLUME,
} DependencyType;

typedef struct {
    DependencyType type;
    char* name;
    char* version;
    char* path;
} Dependency;

typedef struct {
    Dependency* dependencies;
    int count;
    int capacity;
} DependencyList;

// Function declarations
DependencyList* analyze_dependencies(ASTNode* ast);
void free_dependencies(DependencyList* deps);
void extract_imports(ASTNode* node, DependencyList* deps);
void detect_ports(ASTNode* node, DependencyList* deps);
void detect_volumes(ASTNode* node, DependencyList* deps);

#endif
```

Create `src/analyzer/dependency.c`:

```c
#include "dependency.h"
#include <stdlib.h>
#include <string.h>

DependencyList* create_dependency_list() {
    DependencyList* list = (DependencyList*)malloc(sizeof(DependencyList));
    if (!list) return NULL;
    
    list->dependencies = NULL;
    list->count = 0;
    list->capacity = 0;
    
    return list;
}

void free_dependencies(DependencyList* deps) {
    if (!deps) return;
    
    for (int i = 0; i < deps->count; i++) {
        free(deps->dependencies[i].name);
        free(deps->dependencies[i].version);
        free(deps->dependencies[i].path);
    }
    
    free(deps->dependencies);
    free(deps);
}

void add_dependency(DependencyList* deps, Dependency* dep) {
    if (deps->count >= deps->capacity) {
        deps->capacity = deps->capacity == 0 ? 16 : deps->capacity * 2;
        deps->dependencies = (Dependency*)realloc(deps->dependencies, 
                                                  deps->capacity * sizeof(Dependency));
    }
    
    deps->dependencies[deps->count] = *dep;
    deps->count++;
}

void extract_imports(ASTNode* node, DependencyList* deps) {
    if (!node) return;
    
    if (node->type == NODE_IMPORT_DECL) {
        // Check if it's a standard library import
        if (node->value.identifier) {
            Dependency dep;
            dep.type = DEP_STANDARD;
            dep.name = strdup(node->value.identifier);
            dep.version = NULL;
            dep.path = NULL;
            add_dependency(deps, &dep);
        }
    }
    
    for (int i = 0; i < node->child_count; i++) {
        extract_imports(node->children[i], deps);
    }
}

void detect_ports(ASTNode* node, DependencyList* deps) {
    if (!node) return;
    
    // Check for common port patterns
    if (node->type == NODE_STRING_LITERAL && node->value.string_value) {
        // Check if it's a port number
        if (strstr(node->value.string_value, ":8080") ||
            strstr(node->value.string_value, ":80") ||
            strstr(node->value.string_value, ":3000")) {
            Dependency dep;
            dep.type = DEP_NETWORK;
            dep.name = strdup("port");
            dep.version = strdup(node->value.string_value);
            dep.path = NULL;
            add_dependency(deps, &dep);
        }
    }
    
    for (int i = 0; i < node->child_count; i++) {
        detect_ports(node->children[i], deps);
    }
}

void detect_volumes(ASTNode* node, DependencyList* deps) {
    if (!node) return;
    
    // Check for file operations
    if (node->type == NODE_IDENT && node->value.identifier) {
        if (strstr(node->value.identifier, "file") ||
            strstr(node->value.identifier, "db") ||
            strstr(node->value.identifier, "data")) {
            Dependency dep;
            dep.type = DEP_VOLUME;
            dep.name = strdup("volume");
            dep.version = NULL;
            dep.path = NULL;
            add_dependency(deps, &dep);
        }
    }
    
    for (int i = 0; i < node->child_count; i++) {
        detect_volumes(node->children[i], deps);
    }
}

DependencyList* analyze_dependencies(ASTNode* ast) {
    DependencyList* deps = create_dependency_list();
    
    if (!deps) return NULL;
    
    // Extract imports
    extract_imports(ast, deps);
    
    // Detect ports
    detect_ports(ast, deps);
    
    // Detect volumes
    detect_volumes(ast, deps);
    
    return deps;
}
```

---

## Phase 5: Dockerfile Generator

Create `src/generator/dockerfile.h`:

```c
#ifndef GO2DOCKER_DOCKERFILE_H
#define GO2DOCKER_DOCKERFILE_H

#include "../parser/ast.h"
#include "../analyzer/dependency.h"

typedef struct {
    char* base_image;
    char* go_version;
    char* working_dir;
    int ports[10];
    int port_count;
    char* volumes[10];
    int volume_count;
    char* environment[20];
    int env_count;
    bool multi_stage;
    char* build_stage;
    char* runtime_stage;
} DockerfileConfig;

// Function declarations
DockerfileConfig* create_config(void);
void free_config(DockerfileConfig* config);
char* generate_dockerfile(ASTNode* ast, DependencyList* deps);
char* format_dockerfile(DockerfileConfig* config);

#endif
```

Create `src/generator/dockerfile.c`:

```c
#include "dockerfile.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

DockerfileConfig* create_config(void) {
    DockerfileConfig* config = (DockerfileConfig*)malloc(sizeof(DockerfileConfig));
    if (!config) return NULL;
    
    config->base_image = strdup("golang:1.21-alpine");
    config->go_version = strdup("1.21");
    config->working_dir = strdup("/app");
    config->port_count = 0;
    config->volume_count = 0;
    config->env_count = 0;
    config->multi_stage = false;
    config->build_stage = strdup("builder");
    config->runtime_stage = strdup("runtime");
    
    return config;
}

void free_config(DockerfileConfig* config) {
    if (!config) return;
    
    free(config->base_image);
    free(config->go_version);
    free(config->working_dir);
    free(config->build_stage);
    free(config->runtime_stage);
    
    free(config);
}

char* generate_dockerfile(ASTNode* ast, DependencyList* deps) {
    DockerfileConfig* config = create_config();
    
    if (!config) return NULL;
    
    // Analyze dependencies to configure
    for (int i = 0; i < deps->count; i++) {
        Dependency* dep = &deps->dependencies[i];
        
        if (dep->type == DEP_NETWORK && dep->version) {
            if (config->port_count < 10) {
                config->ports[config->port_count++] = atoi(dep->version);
            }
        }
        
        if (dep->type == DEP_VOLUME) {
            if (config->volume_count < 10) {
                config->volumes[config->volume_count++] = strdup("/data");
            }
        }
    }
    
    // Generate Dockerfile content
    char* dockerfile = format_dockerfile(config);
    
    free_config(config);
    
    return dockerfile;
}

char* format_dockerfile(DockerfileConfig* config) {
    // Calculate total length
    size_t total_len = 0;
    
    total_len += strlen("# Generated Dockerfile\n");
    total_len += strlen("FROM ") + strlen(config->base_image) + 2;
    
    if (config->multi_stage) {
        total_len += strlen("FROM ") + strlen(config->base_image) + strlen(" AS ") + 
                     strlen(config->build_stage) + 2;
    }
    
    total_len += strlen("WORKDIR ") + strlen(config->working_dir) + 2;
    
    for (int i = 0; i < config->port_count; i++) {
        total_len += strlen("EXPOSE ") + 10 + 2;
    }
    
    for (int i = 0; i < config->volume_count; i++) {
        total_len += strlen("VOLUME [\"") + strlen(config->volumes[i]) + strlen("\"]") + 2;
    }
    
    total_len += strlen("COPY . .\n");
    total_len += strlen("RUN go build -o main .\n");
    total_len += strlen("CMD [\"./main\"]\n");
    
    // Allocate buffer
    char* buffer = (char*)malloc(total_len + 1);
    if (!buffer) return NULL;
    
    // Build Dockerfile
    size_t pos = 0;
    
    pos += sprintf(buffer + pos, "# Generated Dockerfile\n");
    pos += sprintf(buffer + pos, "FROM %s\n", config->base_image);
    
    if (config->multi_stage) {
        pos += sprintf(buffer + pos, "FROM %s AS %s\n", config->base_image, config->build_stage);
    }
    
    pos += sprintf(buffer + pos, "WORKDIR %s\n", config->working_dir);
    
    for (int i = 0; i < config->port_count; i++) {
        pos += sprintf(buffer + pos, "EXPOSE %d\n", config->ports[i]);
    }
    
    for (int i = 0; i < config->volume_count; i++) {
        pos += sprintf(buffer + pos, "VOLUME [\"%s\"]\n", config->volumes[i]);
    }
    
    pos += sprintf(buffer + pos, "COPY . .\n");
    pos += sprintf(buffer + pos, "RUN go build -o main .\n");
    pos += sprintf(buffer + pos, "CMD [\"./main\"]\n");
    
    buffer[pos] = '\0';
    
    return buffer;
}
```

---

## Phase 6: Main Program

Create `src/main.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser/token.h"
#include "parser/go_parser.h"
#include "parser/ast.h"
#include "analyzer/dependency.h"
#include "generator/dockerfile.h"

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
    
    printf("=== Go to Docker Compiler ===\n");
    printf("Processing: %s\n\n", argv[1]);
    
    // Read source file
    char* source = read_file(argv[1]);
    if (!source) {
        return 1;
    }
    
    // Lexical analysis
    printf("Phase 1: Lexical Analysis\n");
    Token** tokens = NULL;
    int token_count = 0;
    int token_capacity = 0;
    
    // Simple tokenization (in practice, use full lexer)
    // For now, just create a minimal token stream
    tokens = (Token**)malloc(10 * sizeof(Token*));
    tokens[token_count++] = create_token(TOKEN_PACKAGE, "package", 1, 1);
    tokens[token_count++] = create_token(TOKEN_IDENT, "main", 1, 9);
    tokens[token_count++] = create_token(TOKEN_EOF, "", 0, 0);
    
    printf("  Found %d tokens\n\n", token_count);
    
    // Parse
    printf("Phase 2: Syntax Analysis\n");
    Parser* parser = parser_create(tokens, token_count);
    ASTNode* ast = parse_program(parser);
    
    printf("AST:\n");
    print_ast(ast, 0);
    printf("\n");
    
    // Analyze dependencies
    printf("Phase 3: Dependency Analysis\n");
    DependencyList* deps = analyze_dependencies(ast);
    printf("  Found %d dependencies\n\n", deps->count);
    
    // Generate Dockerfile
    printf("Phase 4: Dockerfile Generation\n");
    char* dockerfile = generate_dockerfile(ast, deps);
    printf("Generated Dockerfile:\n%s\n", dockerfile);
    
    // Cleanup
    parser_destroy(parser);
    free_ast(ast);
    
    for (int i = 0; i < token_count; i++) {
        free_token(tokens[i]);
    }
    free(tokens);
    
    free(source);
    free(dockerfile);
    
    printf("Compilation completed!\n");
    return 0;
}
```

---

## Phase 7: Makefile

Create `makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = g2d
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/parser/token.c \
          $(SRC_DIR)/parser/ast.c \
          $(SRC_DIR)/parser/go_parser.c \
          $(SRC_DIR)/analyzer/dependency.c \
          $(SRC_DIR)/generator/dockerfile.c

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
	@./$(TARGET) examples/webserver.go

setup:
	@echo "Creating example Go files..."
	@mkdir -p examples
	@echo 'package main' > examples/hello.go
	@echo '' >> examples/hello.go
	@echo 'import "fmt"' >> examples/hello.go
	@echo '' >> examples/hello.go
	@echo 'func main() {' >> examples/hello.go
	@echo '    fmt.Println("Hello, Docker!")' >> examples/hello.go
	@echo '}' >> examples/hello.go
	@echo 'package main' > examples/webserver.go
	@echo '' >> examples/webserver.go
	@echo 'import (' >> examples/webserver.go
	@echo '    "fmt"' >> examples/webserver.go
	@echo '    "net/http"' >> examples/webserver.go
	@echo ')' >> examples/webserver.go
	@echo '' >> examples/webserver.go
	@echo 'func handler(w http.ResponseWriter, r *http.Request) {' >> examples/webserver.go
	@echo '    fmt.Fprintf(w, "Hello, Docker!")' >> examples/webserver.go
	@echo '}' >> examples/webserver.go
	@echo '' >> examples/webserver.go
	@echo 'func main() {' >> examples/webserver.go
	@echo '    http.HandleFunc("/", handler)' >> examples/webserver.go
	@echo '    http.ListenAndServe(":8080", nil)' >> examples/webserver.go
	@echo '}' >> examples/webserver.go

rebuild: clean all
```

---

## Build and Run

```bash
cd Go2Docker_compiler
make setup      # Create example files
make            # Build
./g2d examples/hello.go
```

---

## Next Steps

1. Implement full lexer
2. Add more expression types
3. Add semantic analysis
4. Implement Docker Compose generation
5. Add image building support