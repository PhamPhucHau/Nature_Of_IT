#include "control.h"

void exec_if_icmpeq(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a == b) {
        interp->pc = target;
    } else {
        interp->pc += 3;  // Opcode + 2 bytes for target
    }
}

void exec_if_icmpne(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a != b) {
        interp->pc = target;
    } else {
        interp->pc += 3;
    }
}

void exec_if_icmplt(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a < b) {
        interp->pc = target;
    } else {
        interp->pc += 3;
    }
}

void exec_if_icmpge(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a >= b) {
        interp->pc = target;
    } else {
        interp->pc += 3;
    }
}

void exec_if_icmpgt(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a > b) {
        interp->pc = target;
    } else {
        interp->pc += 3;
    }
}

void exec_if_icmple(Interpreter* interp, int target) {
    long b = stack_pop(interp->operand_stack);
    long a = stack_pop(interp->operand_stack);
    if (a <= b) {
        interp->pc = target;
    } else {
        interp->pc += 3;
    }
}

void exec_goto(Interpreter* interp, int target) {
    interp->pc = target;
}

void exec_return(Interpreter* interp) {
    // Return from method
    // For now, just set PC to end
    interp->pc = -1;
}