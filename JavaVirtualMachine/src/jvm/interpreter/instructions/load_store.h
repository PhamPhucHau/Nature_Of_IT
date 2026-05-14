#ifndef JVM_INSTRUCTIONS_LOAD_STORE_H
#define JVM_INSTRUCTIONS_LOAD_STORE_H

#include "../executor.h"

// iload instruction
void exec_iload(Interpreter* interp, int index);

// istore instruction
void exec_istore(Interpreter* interp, int index);

// bipush instruction
void exec_bipush(Interpreter* interp);

// iconst instructions
void exec_iconst(Interpreter* interp, int value);

#endif