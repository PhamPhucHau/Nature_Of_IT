# Go to Docker Compiler Architecture Documentation

## Overview

This document describes the architecture of our Go to Docker compiler implementation.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│              Go to Docker Compiler (g2d)                    │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Go Parser  │  │ Dependency   │  │ Dockerfile   │      │
│  │   (AST)      │  │ Analyzer     │  │ Generator    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Go AST     │  │ Dependencies │  │ Dockerfile   │      │
│  │   Structure  │  │ Analysis     │  │ Content      │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │ Docker Compose │                    │
│                      │ Generator      │                    │
│                      │                │                    │
│                      │ docker-compose │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Compiler Phases

### 1. Go Parser

**Purpose**: Parse Go source code into AST

**Input**: Go source code as string

**Output**: Go AST structure

**AST Node Types**:
```
- Program
- PackageClause
- ImportDecl
- FunctionDecl
- VariableDecl
- Expression
- Statement
```

### 2. Dependency Analyzer

**Purpose**: Analyze Go code dependencies

**Input**: Go AST

**Output**: Dependency information

**Analysis**:
- Standard library imports
- External package imports
- System dependencies
- Port requirements
- Volume requirements

### 3. Dockerfile Generator

**Purpose**: Generate Dockerfile from analysis

**Input**: Dependency analysis

**Output**: Dockerfile content

**Features**:
- Base image selection
- Multi-stage builds
- Build optimization
- Runtime configuration

### 4. Docker Compose Generator

**Purpose**: Generate docker-compose.yml

**Input**: Multiple service definitions

**Output**: Compose file content

**Features**:
- Service definitions
- Network configuration
- Volume management
- Environment setup

---

## Data Structures

### Go AST Node
```c
typedef enum {
    NODE_PROGRAM,
    NODE_PACKAGE_CLAUSE,
    NODE_IMPORT_DECL,
    NODE_FUNCTION_DECL,
    NODE_VARIABLE_DECL,
    NODE_EXPRESSION,
    NODE_BINARY_EXPR,
    NODE_CALL_EXPR,
    NODE_IDENT,
    NODE_LITERAL,
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
    } value;
} ASTNode;
```

### Dependency Information
```c
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
```

### Dockerfile Configuration
```c
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
    char* commands[50];
    int cmd_count;
    bool multi_stage;
    char* build_stage;
    char* runtime_stage;
} DockerfileConfig;
```

### Compose Service
```c
typedef struct {
    char* name;
    char* image;
    char* dockerfile;
    int ports[10];
    int port_count;
    char* volumes[10];
    int volume_count;
    char* environment[20];
    int env_count;
    char* depends_on[10];
    int depends_count;
    char* network[10];
    int network_count;
} ComposeService;
```

---

## Go Parser Implementation

### Parsing Algorithm
```c
ASTNode* parse_go_program(const char* source) {
    TokenStream* tokens = tokenize(source);
    Parser* parser = create_parser(tokens);
    
    ASTNode* program = create_node(NODE_PROGRAM);
    
    // Parse package clause
    parse_package_clause(parser, program);
    
    // Parse imports
    while (peek_token(parser)->type == TOKEN_IMPORT) {
        parse_import_decl(parser, program);
    }
    
    // Parse declarations
    while (peek_token(parser)->type != TOKEN_EOF) {
        if (peek_token(parser)->type == TOKEN_FUNC) {
            parse_function_decl(parser, program);
        } else if (peek_token(parser)->type == TOKEN_VAR) {
            parse_variable_decl(parser, program);
        } else {
            parse_statement(parser, program);
        }
    }
    
    destroy_parser(parser);
    return program;
}
```

### Dependency Extraction
```c
DependencyList* analyze_dependencies(ASTNode* ast) {
    DependencyList* deps = create_dependency_list();
    
    // Extract imports
    extract_imports(ast, deps);
    
    // Analyze function calls
    analyze_calls(ast, deps);
    
    // Detect ports
    detect_ports(ast, deps);
    
    // Detect volumes
    detect_volumes(ast, deps);
    
    return deps;
}

void extract_imports(ASTNode* node, DependencyList* deps) {
    if (node->type == NODE_IMPORT_DECL) {
        for (int i = 0; i < node->child_count; i++) {
            ASTNode* import = node->children[i];
            if (import->type == NODE_IDENT) {
                Dependency dep;
                dep.type = DEP_STANDARD;
                dep.name = strdup(import->value.identifier);
                add_dependency(deps, &dep);
            }
        }
    }
    
    for (int i = 0; i < node->child_count; i++) {
        extract_imports(node->children[i], deps);
    }
}
```

---

## Dockerfile Generator Implementation

### Template System
```c
typedef struct {
    char* name;
    char* content;
} Template;

Template templates[] = {
    {"alpine", "FROM golang:1.21-alpine\n..."},
    {"debian", "FROM golang:1.21\n..."},
    {"multi-stage", "FROM golang:1.21 AS builder\n..."},
    {NULL, NULL}
};

char* generate_dockerfile(DockerfileConfig* config) {
    Template* template = find_template(config->multi_stage ? "multi-stage" : "alpine");
    
    // Replace placeholders
    char* result = replace_placeholders(template->content, config);
    
    return result;
}
```

### Build Optimization
```c
void optimize_dockerfile(DockerfileConfig* config) {
    // Order instructions for cache efficiency
    // 1. FROM
    // 2. WORKDIR
    // 3. COPY go.mod go.sum
    // 4. RUN go mod download
    // 5. COPY . .
    // 6. RUN go build
    
    // Minimize layers
    // Combine RUN commands when possible
}
```

---

## Docker Compose Generator Implementation

### Service Generation
```c
ComposeService* generate_service(ASTNode* ast, const char* name) {
    ComposeService* service = create_service(name);
    
    // Set image
    service->image = strdup(name);
    
    // Extract ports
    extract_ports(ast, service->ports, &service->port_count);
    
    // Extract volumes
    extract_volumes(ast, service->volumes, &service->volume_count);
    
    // Extract environment
    extract_environment(ast, service->environment, &service->env_count);
    
    return service;
}

char* generate_compose(ComposeService** services, int count) {
    char* compose = malloc(1024);
    compose[0] = '\0';
    
    strcat(compose, "version: '3.8'\n");
    strcat(compose, "services:\n");
    
    for (int i = 0; i < count; i++) {
        ComposeService* service = services[i];
        strcat(compose, format_service(service));
    }
    
    return compose;
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
    return node;
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

### Dependency Memory
```c
DependencyList* create_dependency_list() {
    DependencyList* list = malloc(sizeof(DependencyList));
    list->dependencies = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

void free_dependencies(DependencyList* list) {
    if (!list) return;
    for (int i = 0; i < list->count; i++) {
        free(list->dependencies[i].name);
    }
    free(list->dependencies);
    free(list);
}
```

---

## Error Handling

### Error Types
```c
typedef enum {
    ERROR_PARSER,
    ERROR_ANALYZER,
    ERROR_GENERATOR,
    ERROR_FILE,
    ERROR_DOCKER,
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
1. **AST Caching**: Cache parsed AST
2. **Dependency Caching**: Cache dependency analysis
3. **Template Caching**: Cache compiled templates
4. **Incremental Builds**: Only rebuild changed parts

### Current Limitations
- Single-threaded
- Limited optimization
- Simple type system
- No async support

---

## Future Enhancements

### Phase 2 Features
- Multi-threading support
- Full async support
- Generational GC
- Type inference

### Phase 3 Features
- Standard library support
- Package management
- Profiling support
- Debugging support

---

## References

- [Docker Documentation](https://docs.docker.com/)
- [Go Language Spec](https://go.dev/ref/spec)
- [Dockerfile Best Practices](https://docs.docker.com/develop/develop-images/dockerfile_best-practices/)
- [Go Docker Guide](https://docs.docker.com/language/golang/)