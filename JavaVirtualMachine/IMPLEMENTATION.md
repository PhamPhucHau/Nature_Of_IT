# JVM Implementation Guide

## Getting Started

### Prerequisites
- C programming knowledge
- Understanding of data structures
- Basic knowledge of Java
- GCC or Clang compiler

### Project Structure
```
JavaVirtualMachine/
├── src/
│   ├── jvm/
│   │   ├── classfile/
│   │   │   ├── parser.c
│   │   │   ├── parser.h
│   │   │   ├── constant_pool.c
│   │   │   └── constant_pool.h
│   │   ├── runtime/
│   │   │   ├── memory.c
│   │   │   ├── memory.h
│   │   │   ├── stack.c
│   │   │   └── stack.h
│   │   ├── interpreter/
│   │   │   ├── executor.c
│   │   │   ├── executor.h
│   │   │   └── instructions/
│   │   │       ├── load_store.c
│   │   │       ├── arithmetic.c
│   │   │       ├── control.c
│   │   │       └── object.c
│   │   └── gc/
│   │       ├── mark_sweep.c
│   │       └── mark_sweep.h
│   └── main.c
├── examples/
├── test/
└── makefile
```

---

## Phase 1: Class File Parser

### Step 1: Define Data Structures

Create `src/jvm/classfile/parser.h`:

```c
#ifndef JVM_CLASSFILE_PARSER_H
#define JVM_CLASSFILE_PARSER_H

#include <stdint.h>

// Type definitions
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

// Constant pool tags
#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref         10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType       12

// Constant pool entry
typedef struct {
    u1 tag;
    union {
        struct {
            u4 length;
            u1* bytes;
        } utf8;
        struct {
            u4 bytes;
        } integer;
        struct {
            u4 bytes;
        } float_;
        struct {
            u4 high_bytes;
            u4 low_bytes;
        } long_;
        struct {
            u4 high_bytes;
            u4 low_bytes;
        } double_;
        struct {
            u2 name_index;
        } class_;
        struct {
            u2 string_index;
        } string;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } fieldref;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } methodref;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } interface_methodref;
        struct {
            u2 name_index;
            u2 descriptor_index;
        } name_and_type;
    } info;
} cp_info;

// Class file structure
typedef struct {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 constant_pool_count;
    cp_info* constant_pool;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    u2 interfaces_count;
    u2* interfaces;
    u2 fields_count;
    // field_info* fields;  // Simplified
    u2 methods_count;
    // method_info* methods;  // Simplified
    u2 attributes_count;
    // attribute_info* attributes;  // Simplified
} ClassFile;

// Function declarations
ClassFile* parse_class_file(const char* filename);
void free_class_file(ClassFile* cf);

#endif
```

### Step 2: Implement Parser

Create `src/jvm/classfile/parser.c`:

```c
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ClassFile* parse_class_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate class file structure
    ClassFile* cf = (ClassFile*)malloc(sizeof(ClassFile));
    if (!cf) {
        fclose(file);
        return NULL;
    }
    
    // Read magic number
    if (fread(&cf->magic, sizeof(u4), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read magic number\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    if (cf->magic != 0xCAFEBABE) {
        fprintf(stderr, "Error: Invalid class file (magic number mismatch)\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Read version numbers
    if (fread(&cf->minor_version, sizeof(u2), 1, file) != 1 ||
        fread(&cf->major_version, sizeof(u2), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read version numbers\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Read constant pool count
    if (fread(&cf->constant_pool_count, sizeof(u2), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read constant pool count\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Parse constant pool
    cf->constant_pool = (cp_info*)malloc((cf->constant_pool_count - 1) * sizeof(cp_info));
    if (!cf->constant_pool) {
        fclose(file);
        free(cf);
        return NULL;
    }
    
    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        if (fread(&cf->constant_pool[i].tag, sizeof(u1), 1, file) != 1) {
            fprintf(stderr, "Error: Cannot read constant pool tag at index %d\n", i);
            free(cf->constant_pool);
            free(cf);
            fclose(file);
            return NULL;
        }
        
        u1 tag = cf->constant_pool[i].tag;
        
        switch (tag) {
            case CONSTANT_Utf8: {
                u2 length;
                fread(&length, sizeof(u2), 1, file);
                cf->constant_pool[i].info.utf8.length = length;
                cf->constant_pool[i].info.utf8.bytes = (u1*)malloc(length + 1);
                fread(cf->constant_pool[i].info.utf8.bytes, 1, length, file);
                cf->constant_pool[i].info.utf8.bytes[length] = '\0';
                break;
            }
            case CONSTANT_Integer:
                fread(&cf->constant_pool[i].info.integer.bytes, sizeof(u4), 1, file);
                break;
            case CONSTANT_Float:
                fread(&cf->constant_pool[i].info.float_.bytes, sizeof(u4), 1, file);
                break;
            case CONSTANT_Long:
                fread(&cf->constant_pool[i].info.long_.high_bytes, sizeof(u4), 1, file);
                fread(&cf->constant_pool[i].info.long_.low_bytes, sizeof(u4), 1, file);
                i++;  // Long takes 2 entries
                break;
            case CONSTANT_Double:
                fread(&cf->constant_pool[i].info.double_.high_bytes, sizeof(u4), 1, file);
                fread(&cf->constant_pool[i].info.double_.low_bytes, sizeof(u4), 1, file);
                i++;  // Double takes 2 entries
                break;
            case CONSTANT_Class:
                fread(&cf->constant_pool[i].info.class_.name_index, sizeof(u2), 1, file);
                break;
            case CONSTANT_String:
                fread(&cf->constant_pool[i].info.string.string_index, sizeof(u2), 1, file);
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
            case CONSTANT_NameAndType:
                // Simplified - read first 4 bytes
                u4 temp;
                fread(&temp, sizeof(u4), 1, file);
                break;
            default:
                fprintf(stderr, "Warning: Unknown constant pool tag %d at index %d\n", tag, i);
                break;
        }
    }
    
    // Read class info (simplified)
    fread(&cf->access_flags, sizeof(u2), 1, file);
    fread(&cf->this_class, sizeof(u2), 1, file);
    fread(&cf->super_class, sizeof(u2), 1, file);
    
    // Read interfaces count
    fread(&cf->interfaces_count, sizeof(u2), 1, file);
    if (cf->interfaces_count > 0) {
        cf->interfaces = (u2*)malloc(cf->interfaces_count * sizeof(u2));
        fread(cf->interfaces, sizeof(u2), cf->interfaces_count, file);
    }
    
    // Read methods count
    fread(&cf->methods_count, sizeof(u2), 1, file);
    
    fclose(file);
    return cf;
}

void free_class_file(ClassFile* cf) {
    if (!cf) return;
    
    if (cf->constant_pool) {
        for (u2 i = 1; i < cf->constant_pool_count; i++) {
            if (cf->constant_pool[i].tag == CONSTANT_Utf8) {
                free(cf->constant_pool[i].info.utf8.bytes);
            }
        }
        free(cf->constant_pool);
    }
    
    if (cf->interfaces) {
        free(cf->interfaces);
    }
    
    free(cf);
}
```

### Step 3: Test Parser

Create `test/test_parser.c`:

```c
#include <stdio.h>
#include "jvm/classfile/parser.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <class_file>\n", argv[0]);
        return 1;
    }
    
    ClassFile* cf = parse_class_file(argv[1]);
    if (!cf) {
        fprintf(stderr, "Failed to parse class file\n");
        return 1;
    }
    
    printf("Class file parsed successfully!\n");
    printf("Magic: 0x%08X\n", cf->magic);
    printf("Version: %d.%d\n", cf->minor_version, cf->major_version);
    printf("Constant pool count: %d\n", cf->constant_pool_count);
    printf("Access flags: 0x%04X\n", cf->access_flags);
    printf("This class: %d\n", cf->this_class);
    printf("Super class: %d\n", cf->super_class);
    printf("Methods count: %d\n", cf->methods_count);
    
    free_class_file(cf);
    return 0;
}
```

---

## Phase 2: Runtime Data Areas

### Step 1: Memory Management

Create `src/jvm/runtime/memory.h`:

```c
#ifndef JVM_RUNTIME_MEMORY_H
#define JVM_RUNTIME_MEMORY_H

#include <stddef.h>

typedef struct {
    void* memory;
    size_t size;
    size_t next_free;
} Heap;

Heap* heap_create(size_t size);
void heap_destroy(Heap* heap);
void* heap_allocate(Heap* heap, size_t size);
void heap_free(Heap* heap, void* ptr);

#endif
```

Create `src/jvm/runtime/memory.c`:

```c
#include "memory.h"
#include <stdlib.h>
#include <string.h>

Heap* heap_create(size_t size) {
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    if (!heap) return NULL;
    
    heap->memory = malloc(size);
    if (!heap->memory) {
        free(heap);
        return NULL;
    }
    
    heap->size = size;
    heap->next_free = 0;
    memset(heap->memory, 0, size);
    
    return heap;
}

void heap_destroy(Heap* heap) {
    if (!heap) return;
    free(heap->memory);
    free(heap);
}

void* heap_allocate(Heap* heap, size_t size) {
    if (heap->next_free + size > heap->size) {
        return NULL;  // Out of memory
    }
    
    void* ptr = heap->memory + heap->next_free;
    heap->next_free += size;
    return ptr;
}

void heap_free(Heap* heap, void* ptr) {
    // Simplified - doesn't actually free memory
    // In a real implementation, this would track freed blocks
}
```

### Step 2: Stack Implementation

Create `src/jvm/runtime/stack.h`:

```c
#ifndef JVM_RUNTIME_STACK_H
#define JVM_RUNTIME_STACK_H

#include <stddef.h>

#define MAX_STACK_SIZE 1024
#define MAX_LOCAL_VARS 256

typedef struct {
    long values[MAX_STACK_SIZE];
    int top;
} OperandStack;

OperandStack* stack_create(void);
void stack_destroy(OperandStack* stack);
void stack_push(OperandStack* stack, long value);
long stack_pop(OperandStack* stack);
int stack_is_empty(OperandStack* stack);
int stack_size(OperandStack* stack);

#endif
```

Create `src/jvm/runtime/stack.c`:

```c
#include "stack.h"
#include <stdlib.h>

OperandStack* stack_create(void) {
    OperandStack* stack = (OperandStack*)malloc(sizeof(OperandStack));
    if (!stack) return NULL;
    
    stack->top = -1;
    return stack;
}

void stack_destroy(OperandStack* stack) {
    free(stack);
}

void stack_push(OperandStack* stack, long value) {
    if (stack->top < MAX_STACK_SIZE - 1) {
        stack->values[++stack->top] = value;
    }
}

long stack_pop(OperandStack* stack) {
    if (stack->top >= 0) {
        return stack->values[stack->top--];
    }
    return 0;
}

int stack_is_empty(OperandStack* stack) {
    return stack->top == -1;
}

int stack_size(OperandStack* stack) {
    return stack->top + 1;
}
```

---

## Phase 3: Bytecode Interpreter

### Step 1: Executor

Create `src/jvm/interpreter/executor.h`:

```c
#ifndef JVM_INTERPRETER_EXECUTOR_H
#define JVM_INTERPRETER_EXECUTOR_H

#include "../classfile/parser.h"
#include "../runtime/memory.h"
#include "../runtime/stack.h"

typedef struct {
    ClassFile* class_file;
    Heap* heap;
    OperandStack* operand_stack;
    long* local_vars;
    int pc;
} Interpreter;

Interpreter* interpreter_create(ClassFile* cf);
void interpreter_destroy(Interpreter* interp);
int interpreter_run(Interpreter* interp, const char* method_name);

#endif
```

Create `src/jvm/interpreter/executor.c`:

```c
#include "executor.h"
#include <stdio.h>
#include <string.h>

Interpreter* interpreter_create(ClassFile* cf) {
    Interpreter* interp = (Interpreter*)malloc(sizeof(Interpreter));
    if (!interp) return NULL;
    
    interp->class_file = cf;
    interp->heap = heap_create(64 * 1024);  // 64KB heap
    interp->operand_stack = stack_create();
    interp->local_vars = (long*)calloc(256, sizeof(long));
    interp->pc = 0;
    
    return interp;
}

void interpreter_destroy(Interpreter* interp) {
    if (!interp) return;
    
    heap_destroy(interp->heap);
    stack_destroy(interp->operand_stack);
    free(interp->local_vars);
    free(interp);
}

int interpreter_run(Interpreter* interp, const char* method_name) {
    printf("Running method: %s\n", method_name);
    
    // Find main method
    // For now, just execute a simple test
    interp->pc = 0;
    
    // Example: Execute some bytecode
    // This would be replaced with actual bytecode execution
    while (interp->pc < 10) {  // Simplified
        // Execute instruction at current PC
        // ...
        interp->pc++;
    }
    
    return 0;
}
```

### Step 2: Instructions

Create `src/jvm/interpreter/instructions/load_store.c`:

```c
#include "../executor.h"

// iload instruction
void exec_iload(Interpreter* interp, int index) {
    long value = interp->local_vars[index];
    stack_push(interp->operand_stack, value);
    interp->pc += 2;  // Opcode + index
}

// istore instruction
void exec_istore(Interpreter* interp, int index) {
    long value = stack_pop(interp->operand_stack);
    interp->local_vars[index] = value;
    interp->pc += 2;  // Opcode + index
}

// bipush instruction
void exec_bipush(Interpreter* interp) {
    // Read byte from bytecode
    // For simplicity, push a constant
    stack_push(interp->operand_stack, 42);
    interp->pc += 2;  // Opcode + value
}

// iconst_0 through iconst_5
void exec_iconst(Interpreter* interp, int value) {
    stack_push(interp->operand_stack, value);
    interp->pc += 1;  // Just opcode
}
```

---

## Phase 4: Main Program

Create `src/main.c`:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jvm/classfile/parser.h"
#include "jvm/interpreter/executor.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <class_file>\n", argv[0]);
        return 1;
    }
    
    printf("=== JVM - Java Virtual Machine ===\n");
    printf("Loading: %s\n\n", argv[1]);
    
    // Parse class file
    ClassFile* cf = parse_class_file(argv[1]);
    if (!cf) {
        fprintf(stderr, "Failed to parse class file\n");
        return 1;
    }
    
    printf("Class file loaded successfully!\n");
    printf("  Magic: 0x%08X\n", cf->magic);
    printf("  Version: %d.%d\n", cf->minor_version, cf->major_version);
    printf("  Constant pool: %d entries\n", cf->constant_pool_count);
    printf("  Methods: %d\n\n", cf->methods_count);
    
    // Create interpreter
    Interpreter* interp = interpreter_create(cf);
    if (!interp) {
        fprintf(stderr, "Failed to create interpreter\n");
        free_class_file(cf);
        return 1;
    }
    
    // Run the program
    printf("Starting execution...\n");
    interpreter_run(interp, "main");
    
    // Cleanup
    interpreter_destroy(interp);
    free_class_file(cf);
    
    printf("\nExecution completed!\n");
    return 0;
}
```

---

## Phase 5: Makefile

Create `makefile`:

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = jvm
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/jvm/classfile/parser.c \
          $(SRC_DIR)/jvm/classfile/constant_pool.c \
          $(SRC_DIR)/jvm/runtime/memory.c \
          $(SRC_DIR)/jvm/runtime/stack.c \
          $(SRC_DIR)/jvm/interpreter/executor.c \
          $(SRC_DIR)/jvm/interpreter/instructions/load_store.c \
          $(SRC_DIR)/jvm/interpreter/instructions/arithmetic.c \
          $(SRC_DIR)/jvm/interpreter/instructions/control.c \
          $(SRC_DIR)/jvm/interpreter/instructions/object.c \
          $(SRC_DIR)/jvm/gc/mark_sweep.c

OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run

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
	./$(TARGET) examples/Hello.class
```

---

## Testing

### Create Test Java Files

Create `examples/Hello.java`:

```java
public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello, JVM!");
        int a = 5;
        int b = 3;
        int sum = a + b;
        System.out.println("5 + 3 = " + sum);
    }
}
```

Compile:
```bash
javac examples/Hello.java
```

### Build and Run

```bash
make
./jvm examples/Hello.class
```

---

## Next Steps

1. Implement more bytecode instructions
2. Add method invocation support
3. Implement object creation
4. Add exception handling
5. Implement garbage collection
6. Add native method support

---

## Debugging Tips

1. Use `printf` debugging to trace execution
2. Use `gdb` for step-by-step debugging
3. Use `valgrind` to check memory issues
4. Compare with `javap -c` output for bytecode verification

```bash
# Disassemble Java class
javap -c examples/Hello.class

# Run with gdb
gdb ./jvm
(gdb) run examples/Hello.class

# Check memory with valgrind
valgrind ./jvm examples/Hello.class
```