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