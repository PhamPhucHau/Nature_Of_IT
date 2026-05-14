#include "constant_pool.h"
#include <stdlib.h>
#include <string.h>

cp_info* constant_pool_get(ClassFile* cf, u2 index) {
    if (index == 0 || index >= cf->constant_pool_count) {
        return NULL;
    }
    return &cf->constant_pool[index];
}

char* constant_pool_get_utf8(ClassFile* cf, u2 index) {
    cp_info* entry = constant_pool_get(cf, index);
    if (!entry || entry->tag != CONSTANT_Utf8) {
        return NULL;
    }
    return (char*)entry->info.utf8.bytes;
}

int constant_pool_get_integer(ClassFile* cf, u2 index) {
    cp_info* entry = constant_pool_get(cf, index);
    if (!entry || entry->tag != CONSTANT_Integer) {
        return 0;
    }
    return (int)entry->info.integer.bytes;
}