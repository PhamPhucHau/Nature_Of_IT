#include "load_store.h"

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