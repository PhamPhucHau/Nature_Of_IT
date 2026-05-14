# Java Spring Boot Compiler Architecture Documentation

## Overview

This document describes the architecture of our Java Spring Boot compiler implementation.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│           Java Spring Boot Compiler (jsb)                   │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │  Java Parser │  │ Spring       │  │ Spring Boot  │      │
│  │   (AST)      │  │ Analyzer     │  │ Generator    │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │   Java AST   │  │ Spring       │  │ Spring Boot  │      │
│  │   Structure  │  │ Analysis     │  │ Project      │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │ Configuration  │                    │
│                      │ Generator      │                    │
│                      │                │                    │
│                      │ pom.xml,       │                    │
│                      │ application.*  │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Compiler Phases

### 1. Java Parser

**Purpose**: Parse Java source code into AST

**Input**: Java source code as string

**Output**: Java AST structure

**AST Node Types**:
```
- Program
- PackageDeclaration
- ImportDeclaration
- ClassDeclaration
- MethodDeclaration
- FieldDeclaration
- Statement
- Expression
```

### 2. Spring Analyzer

**Purpose**: Analyze Java code for Spring Boot requirements

**Input**: Java AST

**Output**: Spring Boot requirements

**Analysis**:
- REST controllers
- Service classes
- Repository interfaces
- Data models
- Dependencies

### 3. Spring Boot Generator

**Purpose**: Generate Spring Boot project structure

**Input**: Spring Boot requirements

**Output**: Project files

**Features**:
- Main application class
- REST controllers
- Service layer
- Repository layer
- DTOs

### 4. Configuration Generator

**Purpose**: Generate configuration files

**Input**: Project requirements

**Output**: Configuration files

**Features**:
- pom.xml / build.gradle
- application.properties
- application.yml
- Docker configuration

---

## Data Structures

### Java AST Node
```c
typedef enum {
    NODE_PROGRAM,
    NODE_PACKAGE_DECLARATION,
    NODE_IMPORT_DECLARATION,
    NODE_CLASS_DECLARATION,
    NODE_METHOD_DECLARATION,
    NODE_FIELD_DECLARATION,
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

### Spring Requirements
```c
typedef enum {
    REQ_CONTROLLER,
    REQ_SERVICE,
    REQ_REPOSITORY,
    REQ_MODEL,
    REQ_DATABASE,
    REQ_SECURITY,
} RequirementType;

typedef struct {
    RequirementType type;
    char* name;
    char* package;
    char* fields[10];
    int field_count;
    char* methods[10];
    int method_count;
} SpringRequirement;

typedef struct {
    SpringRequirement* requirements;
    int count;
    int capacity;
} SpringRequirements;
```

### Spring Boot Project
```c
typedef struct {
    char* group_id;
    char* artifact_id;
    char* version;
    char* main_class;
    char* package_name;
    char* application_name;
    SpringRequirements* requirements;
    char* dependencies[20];
    int dependency_count;
    char* properties[20];
    int property_count;
} SpringBootProject;
```

---

## Java Parser Implementation

### Parsing Algorithm
```c
ASTNode* parse_java_program(const char* source) {
    TokenStream* tokens = tokenize(source);
    Parser* parser = create_parser(tokens);
    
    ASTNode* program = create_node(NODE_PROGRAM);
    
    // Parse package declaration
    parse_package_declaration(parser, program);
    
    // Parse imports
    while (peek_token(parser)->type == TOKEN_IMPORT) {
        parse_import_declaration(parser, program);
    }
    
    // Parse type declarations
    while (peek_token(parser)->type != TOKEN_EOF) {
        if (peek_token(parser)->type == TOKEN_CLASS) {
            parse_class_declaration(parser, program);
        } else {
            next_token(parser);
        }
    }
    
    destroy_parser(parser);
    return program;
}
```

### Spring Analysis
```c
SpringRequirements* analyze_spring(ASTNode* ast) {
    SpringRequirements* reqs = create_spring_requirements();
    
    // Find REST controllers
    find_controllers(ast, reqs);
    
    // Find services
    find_services(ast, reqs);
    
    // Find repositories
    find_repositories(ast, reqs);
    
    // Find models
    find_models(ast, reqs);
    
    return reqs;
}

void find_controllers(ASTNode* node, SpringRequirements* reqs) {
    if (!node) return;
    
    if (node->type == NODE_CLASS_DECLARATION) {
        // Check for @RestController annotation
        if (has_annotation(node, "RestController")) {
            SpringRequirement req;
            req.type = REQ_CONTROLLER;
            req.name = node->value.identifier;
            req.package = get_package(node);
            extract_fields(node, req.fields, &req.field_count);
            extract_methods(node, req.methods, &req.method_count);
            add_requirement(reqs, &req);
        }
    }
    
    for (int i = 0; i < node->child_count; i++) {
        find_controllers(node->children[i], reqs);
    }
}
```

---

## Spring Boot Generator Implementation

### Project Generation
```c
SpringBootProject* generate_spring_boot_project(ASTNode* ast, SpringRequirements* reqs) {
    SpringBootProject* project = create_spring_boot_project();
    
    // Set project info
    project->group_id = strdup("com.example");
    project->artifact_id = strdup("demo");
    project->version = strdup("0.0.1-SNAPSHOT");
    project->main_class = strdup("DemoApplication");
    project->package_name = strdup("com.example.demo");
    
    // Set requirements
    project->requirements = reqs;
    
    // Add dependencies
    add_dependency(project, "spring-boot-starter-web");
    add_dependency(project, "spring-boot-starter-data-jpa");
    add_dependency(project, "spring-boot-starter-validation");
    add_dependency(project, "mysql-connector-java");
    
    return project;
}
```

### File Generation
```c
char* generate_main_class(SpringBootProject* project) {
    char* template = 
        "package %s;\n"
        "\n"
        "import org.springframework.boot.SpringApplication;\n"
        "import org.springframework.boot.autoconfigure.SpringBootApplication;\n"
        "\n"
        "@SpringBootApplication\n"
        "public class %s {\n"
        "    public static void main(String[] args) {\n"
        "        SpringApplication.run(%s.class, args);\n"
        "    }\n"
        "}\n";
    
    size_t len = strlen(template) + 
                 strlen(project->package_name) * 2 +
                 strlen(project->main_class) * 2 + 100;
    
    char* result = malloc(len);
    sprintf(result, template,
            project->package_name,
            project->main_class,
            project->main_class);
    
    return result;
}
```

---

## Configuration Generator Implementation

### Maven POM Generation
```c
char* generate_pom(SpringBootProject* project) {
    char* template = 
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<project xmlns=\"http://maven.apache.org/POM/4.0.0\"\n"
        "         xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
        "         xsi:schemaLocation=\"http://maven.apache.org/POM/4.0.0 \n"
        "         http://maven.apache.org/xsd/maven-4.0.0.xsd\">\n"
        "    <modelVersion>4.0.0</modelVersion>\n"
        "\n"
        "    <groupId>%s</groupId>\n"
        "    <artifactId>%s</artifactId>\n"
        "    <version>%s</version>\n"
        "    <packaging>jar</packaging>\n"
        "\n"
        "    <name>%s</name>\n"
        "    <description>Demo project for Spring Boot</description>\n"
        "\n"
        "    <parent>\n"
        "        <groupId>org.springframework.boot</groupId>\n"
        "        <artifactId>spring-boot-starter-parent</artifactId>\n"
        "        <version>3.2.0</version>\n"
        "        <relativePath/> \n"
        "    </parent>\n"
        "\n"
        "    <properties>\n"
        "        <java.version>17</java.version>\n"
        "    </properties>\n"
        "\n"
        "    <dependencies>\n"
        "%s"
        "    </dependencies>\n"
        "\n"
        "    <build>\n"
        "        <plugins>\n"
        "            <plugin>\n"
        "                <groupId>org.springframework.boot</groupId>\n"
        "                <artifactId>spring-boot-maven-plugin</artifactId>\n"
        "            </plugin>\n"
        "        </plugins>\n"
        "    </build>\n"
        "</project>\n";
    
    // Build dependencies string
    char* deps = "";
    for (int i = 0; i < project->dependency_count; i++) {
        char* dep_xml = 
            "        <dependency>\n"
            "            <groupId>%s</groupId>\n"
            "            <artifactId>%s</artifactId>\n"
            "        </dependency>\n";
        // ... build dependency XML
    }
    
    // Format final POM
    // ...
    
    return pom;
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

### Spring Requirements Memory
```c
SpringRequirements* create_spring_requirements() {
    SpringRequirements* reqs = malloc(sizeof(SpringRequirements));
    reqs->requirements = NULL;
    reqs->count = 0;
    reqs->capacity = 0;
    return reqs;
}

void free_spring_requirements(SpringRequirements* reqs) {
    if (!reqs) return;
    for (int i = 0; i < reqs->count; i++) {
        free(reqs->requirements[i].name);
        free(reqs->requirements[i].package);
    }
    free(reqs->requirements);
    free(reqs);
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
    ERROR_BUILD,
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
4. **Incremental Generation**: Only regenerate changed parts

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

- [Spring Boot Docs](https://spring.io/projects/spring-boot)
- [Java Language Spec](https://docs.oracle.com/javase/specs/)
- [Maven POM](https://maven.apache.org/pom/)
- [Baeldung Spring](https://www.baeldung.com/spring-boot)