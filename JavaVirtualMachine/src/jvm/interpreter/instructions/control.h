#ifndef JVM_INSTRUCTIONS_CONTROL_H
#define JVM_INSTRUCTIONS_CONTROL_H

#include "../executor.h"

// Control instructions
void exec_if_icmpeq(Interpreter* interp, int target);
void exec_if_icmpne(Interpreter* interp, int target);
void exec_if_icmplt(Interpreter* interp, int target);
void exec_if_icmpge(Interpreter* interp, int target);
void exec_if_icmpgt(Interpreter* interp, int target);
void exec_if_icmple(Interpreter* interp, int target);
void exec_goto(Interpreter* interp, int target);
void exec_return(Interpreter* interp);

#endif