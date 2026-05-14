#ifndef JVM_CLASSFILE_PARSER_H
#define JVM_CLASSFILE_PARSER_H

#include <stdint.h>

// Type definitions
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

// Constant pool tags
#define CONSTANT_Utf8               1
#define CONSTANT_Integer            3
#define CONSTANT_Float              4
#define CONSTANT_Long               5
#define CONSTANT_Double             6
#define CONSTANT_Class              7
#define CONSTANT_String             8
#define CONSTANT_Fieldref           9
#define CONSTANT_Methodref         10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_NameAndType       12

// Constant pool entry
typedef struct {
    u1 tag;
    union {
        struct {
            u4 length;
            u1* bytes;
        } utf8;
        struct {
            u4 bytes;
        } integer;
        struct {
            u4 bytes;
        } float_;
        struct {
            u4 high_bytes;
            u4 low_bytes;
        } long_;
        struct {
            u4 high_bytes;
            u4 low_bytes;
        } double_;
        struct {
            u2 name_index;
        } class_;
        struct {
            u2 string_index;
        } string;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } fieldref;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } methodref;
        struct {
            u2 class_index;
            u2 name_and_type_index;
        } interface_methodref;
        struct {
            u2 name_index;
            u2 descriptor_index;
        } name_and_type;
    } info;
} cp_info;

// Class file structure
typedef struct {
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 constant_pool_count;
    cp_info* constant_pool;
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    u2 interfaces_count;
    u2* interfaces;
    u2 fields_count;
    u2 methods_count;
    u2 attributes_count;
} ClassFile;

// Function declarations
ClassFile* parse_class_file(const char* filename);
void free_class_file(ClassFile* cf);

#endif