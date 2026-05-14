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