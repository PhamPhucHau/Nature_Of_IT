# JVM Architecture Documentation

## Overview

This document describes the architecture of our simplified Java Virtual Machine implementation.

---

## High-Level Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        JVM Runtime                          │
├─────────────────────────────────────────────────────────────┤
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Class Loader │  │ Runtime Data │  │ Execution    │      │
│  │ Subsystem    │  │ Areas        │  │ Engine       │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│         │                 │                  │               │
│         ▼                 ▼                  ▼               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │
│  │ Method Area  │  │ Heap         │  │ Interpreter  │      │
│  │              │  │              │  │              │      │
│  │ Class Meta   │  │ Objects      │  │ Bytecode     │      │
│  │ Data         │  │              │  │ Executor     │      │
│  └──────────────┘  └──────────────┘  └──────────────┘      │
│                                      │                  │   │
│                              ┌───────┴──────────────────┘   │
│                              │                              │
│                      ┌───────▼────────┐                    │
│                      │  Java Stacks   │                    │
│                      │                │                    │
│                      │ Stack Frames   │                    │
│                      │ - Local Vars   │                    │
│                      │ - Operand Stack│                    │
│                      │ - Dynamic Link │                    │
│                      └────────────────┘                    │
└─────────────────────────────────────────────────────────────┘
```

---

## Runtime Data Areas

### 1. Method Area

**Purpose**: Stores class metadata, method code, constants

**Structure**:
```c
typedef struct {
    char* class_name;
    char* super_class;
    int access_flags;
    ConstantPool* constant_pool;
    FieldInfo* fields;
    MethodInfo* methods;
} ClassInfo;
```

**Operations**:
- Load class from .class file
- Store class metadata
- Retrieve class by name

### 2. Heap

**Purpose**: Stores object instances

**Structure**:
```c
typedef struct {
    void* memory;
    size_t size;
    size_t used;
    GCInfo* gc_info;
} Heap;
```

**Operations**:
- Allocate memory for objects
- Track object references
- Garbage collection

### 3. Java Stacks

**Purpose**: Stores method call frames

**Structure**:
```c
typedef struct StackFrame {
    jint* local_vars;
    OperandStack* operand_stack;
    ClassInfo* current_class;
    MethodInfo* current_method;
    struct StackFrame* prev;
    struct StackFrame* next;
} StackFrame;

typedef struct {
    StackFrame* top;
    int max_depth;
} JavaStack;
```

**Operations**:
- Push frame for method call
- Pop frame on return
- Access local variables
- Manage operand stack

### 4. PC Registers

**Purpose**: Tracks current instruction address

**Structure**:
```c
typedef struct {
    u4 instruction_pointer;
    ClassInfo* current_class;
    MethodInfo* current_method;
} PCRegisters;
```

### 5. Native Method Stacks

**Purpose**: Supports native methods (JNI)

**Structure**:
```c
typedef struct {
    void* stack;
    size_t size;
} NativeMethodStack;
```

---

## Execution Engine

### 1. Interpreter

**Purpose**: Executes bytecode instructions

**Structure**:
```c
typedef struct {
    JavaStack* stack;
    Heap* heap;
    ClassLoader* class_loader;
    PCRegisters* pc;
} Interpreter;

typedef struct {
    u1 opcode;
    const char* name;
    void (*execute)(Interpreter* interp);
} Instruction;
```

**Operations**:
- Fetch instruction
- Decode opcode
- Execute instruction
- Update PC

### 2. Instruction Set

**Categories**:
- Load/Store: iload, istore, aload, astore
- Arithmetic: iadd, isub, imul, idiv
- Control: if_icmpeq, goto, tableswitch
- Object: new, getfield, putfield
- Method: invokevirtual, invokestatic

**Example**:
```c
void execute_iadd(Interpreter* interp) {
    OperandStack* stack = interp->stack->top->operand_stack;
    int b = pop_int(stack);
    int a = pop_int(stack);
    push_int(stack, a + b);
    interp->pc->instruction_pointer += 1;
}
```

---

## Class File Parser

### Structure
```c
typedef struct {
    u4 magic;                    // 0xCAFEBABE
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
    field_info* fields;
    u2 methods_count;
    method_info* methods;
    u2 attributes_count;
    attribute_info* attributes;
} ClassFile;
```

### Parsing Steps
1. Read magic number (validate)
2. Read version numbers
3. Parse constant pool
4. Parse class info
5. Parse fields
6. Parse methods
7. Parse attributes

---

## Memory Management

### Heap Allocation
```c
typedef struct {
    u1* memory;
    size_t size;
    size_t next_free;
    ObjectHeader* objects;
} Heap;

void* heap_allocate(Heap* heap, size_t size) {
    void* ptr = heap->memory + heap->next_free;
    heap->next_free += size;
    return ptr;
}
```

### Garbage Collection (Mark-Sweep)
```c
void gc_mark_sweep(Heap* heap) {
    // Mark phase
    mark_root_set(heap);
    mark_objects(heap);
    
    // Sweep phase
    sweep(heap);
}
```

---

## Thread Model

### Single Thread
- One Java stack
- One PC register
- Shared heap and method area

### Multi-Thread (Future)
- Each thread has own stack
- Shared heap and method area
- Thread-local storage

---

## Data Types

### Primitive Types
| Type | Size | Java Type |
|------|------|-----------|
| u1   | 1 byte | byte, boolean, char |
| u2   | 2 bytes | short, int (partial) |
| u4   | 4 bytes | int, float |
| u8   | 8 bytes | long, double |

### Reference Types
- Class references
- Array references
- Interface references

---

## Instruction Encoding

### Format
```
┌─────────────────────────────────────┐
│ Opcode (1 byte)                     │
├─────────────────────────────────────┤
│ Operand 1 (0-2 bytes)               │
├─────────────────────────────────────┤
│ Operand 2 (0-2 bytes)               │
└─────────────────────────────────────┘
```

### Examples
- `iload_0` - 0x15 (no operands)
- `iload` - 0x15 + 1 byte index
- `bipush` - 0x10 + 1 byte value
- `sipush` - 0x11 + 2 bytes value

---

## Error Handling

### JVM Errors
- OutOfMemoryError
- StackOverflowError
- NoSuchMethodError
- NoSuchFieldError
- ClassNotFoundException

### Exception Handling
```c
typedef struct {
    u2 start_pc;
    u2 end_pc;
    u2 handler_pc;
    u2 catch_type;
} ExceptionTableEntry;
```

---

## Performance Considerations

### Optimization Strategies
1. **Inline Caching**: Cache method lookup results
2. **Stack Allocation**: Allocate small objects on stack
3. **Escape Analysis**: Detect objects that don't escape
4. **JIT Compilation**: Compile hot methods to native code

### Current Limitations
- Single-threaded
- No JIT compilation
- Simple GC (no generational)
- Limited exception handling

---

## Future Enhancements

### Phase 2 Features
- Multi-threading support
- JIT compiler
- Generational GC
- Full exception handling

### Phase 3 Features
- JNI integration
- Security manager
- Profiling support
- Debugging support

---

## References

- [JVM Specification](https://docs.oracle.com/javase/specs/)
- [Java Bytecode Instructions](https://docs.oracle.com/javase/specs/jvms/se17/html/jvms-6.html)
- [OpenJDK Source Code](https://github.com/openjdk/jdk)