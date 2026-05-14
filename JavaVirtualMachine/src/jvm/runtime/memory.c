#include "memory.h"
#include <stdlib.h>
#include <string.h>

Heap* heap_create(size_t size) {
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    if (!heap) return NULL;
    
    heap->memory = malloc(size);
    if (!heap->memory) {
        free(heap);
        return NULL;
    }
    
    heap->size = size;
    heap->next_free = 0;
    memset(heap->memory, 0, size);
    
    return heap;
}

void heap_destroy(Heap* heap) {
    if (!heap) return;
    free(heap->memory);
    free(heap);
}

void* heap_allocate(Heap* heap, size_t size) {
    if (heap->next_free + size > heap->size) {
        return NULL;  // Out of memory
    }
    
    void* ptr = heap->memory + heap->next_free;
    heap->next_free += size;
    return ptr;
}

void heap_free(Heap* heap, void* ptr) {
    // Simplified - doesn't actually free memory
    // In a real implementation, this would track freed blocks
}