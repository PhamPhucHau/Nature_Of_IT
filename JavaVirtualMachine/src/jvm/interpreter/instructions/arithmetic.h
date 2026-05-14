#ifndef JVM_INSTRUCTIONS_ARITHMETIC_H
#define JVM_INSTRUCTIONS_ARITHMETIC_H

#include "../executor.h"

// Arithmetic instructions
void exec_iadd(Interpreter* interp);
void exec_isub(Interpreter* interp);
void exec_imul(Interpreter* interp);
void exec_idiv(Interpreter* interp);
void exec_irem(Interpreter* interp);
void exec_ineg(Interpreter* interp);

#endif