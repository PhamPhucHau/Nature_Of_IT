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