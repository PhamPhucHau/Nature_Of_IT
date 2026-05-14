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