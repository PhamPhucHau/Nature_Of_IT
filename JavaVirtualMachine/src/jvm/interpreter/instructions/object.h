#ifndef JVM_INSTRUCTIONS_OBJECT_H
#define JVM_INSTRUCTIONS_OBJECT_H

#include "../executor.h"

// Object instructions
void exec_new(Interpreter* interp, int class_index);
void exec_getfield(Interpreter* interp, int field_index);
void exec_putfield(Interpreter* interp, int field_index);
void exec_getstatic(Interpreter* interp, int field_index);
void exec_putstatic(Interpreter* interp, int field_index);

#endif