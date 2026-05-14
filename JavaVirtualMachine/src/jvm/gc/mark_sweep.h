#ifndef JVM_GC_MARK_SWEEP_H
#define JVM_GC_MARK_SWEEP_H

#include "../runtime/memory.h"

// Garbage collection
void gc_mark_sweep(Heap* heap);
void gc_mark_root_set(Heap* heap);
void gc_mark_objects(Heap* heap);
void gc_sweep(Heap* heap);

#endif