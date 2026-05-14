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