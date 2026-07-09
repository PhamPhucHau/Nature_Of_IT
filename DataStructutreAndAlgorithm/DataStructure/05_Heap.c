#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_SIZE 100

typedef struct {
    int data[HEAP_SIZE];
    int size;
} MinHeap;

MinHeap* createHeap() {
    MinHeap *heap = (MinHeap *)malloc(sizeof(MinHeap));
    heap->size = 0;
    return heap;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleUp(MinHeap *heap, int idx) {
    while (idx > 0 && heap->data[(idx-1)/2] > heap->data[idx]) {
        swap(&heap->data[(idx-1)/2], &heap->data[idx]);
        idx = (idx-1)/2;
    }
}

void bubbleDown(MinHeap *heap, int idx) {
    while (2*idx + 1 < heap->size) {
        int smallest = idx;
        int left = 2*idx + 1;
        int right = 2*idx + 2;
        
        if (heap->data[left] < heap->data[smallest]) smallest = left;
        if (right < heap->size && heap->data[right] < heap->data[smallest]) smallest = right;
        
        if (smallest == idx) break;
        
        swap(&heap->data[idx], &heap->data[smallest]);
        idx = smallest;
    }
}

void insert(MinHeap *heap, int val) {
    if (heap->size < HEAP_SIZE) {
        heap->data[heap->size] = val;
        bubbleUp(heap, heap->size);
        heap->size++;
        printf("  + Insert %d\n", val);
    }
}

int extractMin(MinHeap *heap) {
    if (heap->size == 0) return -1;
    
    int min = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    bubbleDown(heap, 0);
    printf("  - Extract %d\n", min);
    return min;
}

void printHeap(MinHeap *heap) {
    printf("Heap [");
    for (int i = 0; i < heap->size; i++) {
        printf("%d", heap->data[i]);
        if (i < heap->size - 1) printf(", ");
    }
    printf("] (size: %d)\n", heap->size);
}

int main() {
    printf("===== MIN HEAP =====\n\n");
    
    MinHeap *heap = createHeap();
    
    printf("--- Inserting elements ---\n");
    insert(heap, 10);
    insert(heap, 5);
    insert(heap, 20);
    insert(heap, 2);
    insert(heap, 8);
    printHeap(heap);
    printf("\n");
    
    printf("--- Extracting minimum ---\n");
    extractMin(heap);
    printHeap(heap);
    extractMin(heap);
    printHeap(heap);
    printf("\n");
    
    printf("=== SUMMARY ===\n");
    printf("Insert: O(log n)\n");
    printf("Extract Min: O(log n)\n");
    printf("Network: QoS scheduling, priority queues\n");
    
    free(heap);
    return 0;
}
