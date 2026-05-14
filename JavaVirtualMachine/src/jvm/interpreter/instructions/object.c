#include "object.h"

void exec_new(Interpreter* interp, int class_index) {
    // Allocate object memory
    // For now, just allocate a small block
    void* obj = heap_allocate(interp->heap, 64);
    if (obj) {
        stack_push(interp->operand_stack, (long)obj);
    } else {
        fprintf(stderr, "Error: Out of memory\n");
    }
    interp->pc += 3;  // Opcode + 2 bytes for class index
}

void exec_getfield(Interpreter* interp, int field_index) {
    // Get field value from object
    long obj_ref = stack_pop(interp->operand_stack);
    // For now, just push a dummy value
    stack_push(interp->operand_stack, 42);
    interp->pc += 3;  // Opcode + 2 bytes for field index
}

void exec_putfield(Interpreter* interp, int field_index) {
    // Set field value in object
    long value = stack_pop(interp->operand_stack);
    long obj_ref = stack_pop(interp->operand_stack);
    // For now, just ignore
    interp->pc += 3;  // Opcode + 2 bytes for field index
}

void exec_getstatic(Interpreter* interp, int field_index) {
    // Get static field value
    stack_push(interp->operand_stack, 42);
    interp->pc += 3;  // Opcode + 2 bytes for field index
}

void exec_putstatic(Interpreter* interp, int field_index) {
    // Set static field value
    long value = stack_pop(interp->operand_stack);
    interp->pc += 3;  // Opcode + 2 bytes for field index
}