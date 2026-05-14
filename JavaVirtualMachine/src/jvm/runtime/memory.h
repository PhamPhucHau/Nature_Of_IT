#ifndef JVM_RUNTIME_MEMORY_H
#define JVM_RUNTIME_MEMORY_H

#include <stddef.h>

typedef struct {
    void* memory;
    size_t size;
    size_t next_free;
} Heap;

Heap* heap_create(size_t size);
void heap_destroy(Heap* heap);
void* heap_allocate(Heap* heap, size_t size);
void heap_free(Heap* heap, void* ptr);

#endif