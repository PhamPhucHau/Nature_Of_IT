#ifndef JVM_CLASSFILE_CONSTANT_POOL_H
#define JVM_CLASSFILE_CONSTANT_POOL_H

#include "parser.h"

// Constant pool operations
cp_info* constant_pool_get(ClassFile* cf, u2 index);
char* constant_pool_get_utf8(ClassFile* cf, u2 index);
int constant_pool_get_integer(ClassFile* cf, u2 index);

#endif