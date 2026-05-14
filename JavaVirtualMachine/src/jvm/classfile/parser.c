#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ClassFile* parse_class_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate class file structure
    ClassFile* cf = (ClassFile*)malloc(sizeof(ClassFile));
    if (!cf) {
        fclose(file);
        return NULL;
    }
    
    // Read magic number
    if (fread(&cf->magic, sizeof(u4), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read magic number\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    if (cf->magic != 0xCAFEBABE) {
        fprintf(stderr, "Error: Invalid class file (magic number mismatch)\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Read version numbers
    if (fread(&cf->minor_version, sizeof(u2), 1, file) != 1 ||
        fread(&cf->major_version, sizeof(u2), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read version numbers\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Read constant pool count
    if (fread(&cf->constant_pool_count, sizeof(u2), 1, file) != 1) {
        fprintf(stderr, "Error: Cannot read constant pool count\n");
        free(cf);
        fclose(file);
        return NULL;
    }
    
    // Parse constant pool
    cf->constant_pool = (cp_info*)malloc((cf->constant_pool_count - 1) * sizeof(cp_info));
    if (!cf->constant_pool) {
        fclose(file);
        free(cf);
        return NULL;
    }
    
    for (u2 i = 1; i < cf->constant_pool_count; i++) {
        if (fread(&cf->constant_pool[i].tag, sizeof(u1), 1, file) != 1) {
            fprintf(stderr, "Error: Cannot read constant pool tag at index %d\n", i);
            free(cf->constant_pool);
            free(cf);
            fclose(file);
            return NULL;
        }
        
        u1 tag = cf->constant_pool[i].tag;
        
        switch (tag) {
            case CONSTANT_Utf8: {
                u2 length;
                fread(&length, sizeof(u2), 1, file);
                cf->constant_pool[i].info.utf8.length = length;
                cf->constant_pool[i].info.utf8.bytes = (u1*)malloc(length + 1);
                fread(cf->constant_pool[i].info.utf8.bytes, 1, length, file);
                cf->constant_pool[i].info.utf8.bytes[length] = '\0';
                break;
            }
            case CONSTANT_Integer:
                fread(&cf->constant_pool[i].info.integer.bytes, sizeof(u4), 1, file);
                break;
            case CONSTANT_Float:
                fread(&cf->constant_pool[i].info.float_.bytes, sizeof(u4), 1, file);
                break;
            case CONSTANT_Long:
                fread(&cf->constant_pool[i].info.long_.high_bytes, sizeof(u4), 1, file);
                fread(&cf->constant_pool[i].info.long_.low_bytes, sizeof(u4), 1, file);
                i++;  // Long takes 2 entries
                break;
            case CONSTANT_Double:
                fread(&cf->constant_pool[i].info.double_.high_bytes, sizeof(u4), 1, file);
                fread(&cf->constant_pool[i].info.double_.low_bytes, sizeof(u4), 1, file);
                i++;  // Double takes 2 entries
                break;
            case CONSTANT_Class:
                fread(&cf->constant_pool[i].info.class_.name_index, sizeof(u2), 1, file);
                break;
            case CONSTANT_String:
                fread(&cf->constant_pool[i].info.string.string_index, sizeof(u2), 1, file);
                break;
            case CONSTANT_Fieldref:
            case CONSTANT_Methodref:
            case CONSTANT_InterfaceMethodref:
            case CONSTANT_NameAndType:
                // Simplified - read first 4 bytes
                u4 temp;
                fread(&temp, sizeof(u4), 1, file);
                break;
            default:
                fprintf(stderr, "Warning: Unknown constant pool tag %d at index %d\n", tag, i);
                break;
        }
    }
    
    // Read class info (simplified)
    fread(&cf->access_flags, sizeof(u2), 1, file);
    fread(&cf->this_class, sizeof(u2), 1, file);
    fread(&cf->super_class, sizeof(u2), 1, file);
    
    // Read interfaces count
    fread(&cf->interfaces_count, sizeof(u2), 1, file);
    if (cf->interfaces_count > 0) {
        cf->interfaces = (u2*)malloc(cf->interfaces_count * sizeof(u2));
        fread(cf->interfaces, sizeof(u2), cf->interfaces_count, file);
    }
    
    // Read methods count
    fread(&cf->methods_count, sizeof(u2), 1, file);
    
    fclose(file);
    return cf;
}

void free_class_file(ClassFile* cf) {
    if (!cf) return;
    
    if (cf->constant_pool) {
        for (u2 i = 1; i < cf->constant_pool_count; i++) {
            if (cf->constant_pool[i].tag == CONSTANT_Utf8) {
                free(cf->constant_pool[i].info.utf8.bytes);
            }
        }
        free(cf->constant_pool);
    }
    
    if (cf->interfaces) {
        free(cf->interfaces);
    }
    
    free(cf);
}