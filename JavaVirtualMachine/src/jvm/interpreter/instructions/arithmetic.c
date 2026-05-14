#include "arithmetic.h"

void exec_iadd(Interpreter* interp) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    stack_push(interp->operand_stack, a + b);
    interp->pc += 1;
}

void exec_isub(Interpreter* interp) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    stack_push(interp->operand_stack, a - b);
    interp->pc += 1;
}

void exec_imul(Interpreter* interp) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    stack_push(interp->operand_stack, a * b);
    interp->pc += 1;
}

void exec_idiv(Interpreter* interp) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (b != 0) {
        stack_push(interp->operand_stack, a / b);
    } else {
        fprintf(stderr, "Error: Division by zero\n");
    }
    interp->pc += 1;
}

void exec_irem(Interpreter* interp) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (b != 0) {
        stack_push(interp->operand_stack, a % b);
    } else {
        fprintf(stderr, "Error: Division by zero\n");
    }
    interp->pc += 1;
}

void exec_ineg(Interpreter* interp) {
    long a = stack_pop(interp->operand_stack);
    stack_push(interp->operand_stack, -a);
    interp->pc += 1;
}