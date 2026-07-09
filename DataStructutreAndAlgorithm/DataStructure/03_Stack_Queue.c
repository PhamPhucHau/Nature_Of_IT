/**
 * DATA STRUCTURES: Stack & Queue
 * 
 * STORY:
 * Stack: "Last In, First Out" (LIFO)
 * Ví dụ: Chồng đĩa. Đĩa được xếp lên trên cùng sẽ bị lấy ra trước.
 * 
 * Queue: "First In, First Out" (FIFO)
 * Ví dụ: Hàng chờ. Người vào trước sẽ được phục vụ trước.
 * 
 * NETWORK APPLICATION:
 * - Stack: Protocol parsing, undo/redo, backtracking
 * - Queue: Packet scheduling, task queue, breadth-first search
 * 
 * KIẾN TRÚC:
 * 
 * STACK:              QUEUE:
 * ┌─────┐            ┌─────┬─────┬─────┐
 * │  3  │ push →     │  1  │  2  │  3  │ ← enqueue
 * │  2  │            │     │     │     │
 * │  1  │ pop ↑      │ ↑dequeue       │
 * └─────┘            └─────┴─────┴─────┘
 */

#include <stdio.h>
#include <stdlib.h>

// ============================================================
// PART 1: STACK
// ============================================================

typedef struct {
    int *data;
    int top;          // Index của phần tử trên cùng (top = -1 khi rỗng)
    int capacity;
} Stack;

Stack* createStack(int capacity) {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->data = (int *)malloc(capacity * sizeof(int));
    stack->capacity = capacity;
    stack->top = -1;
    printf("✓ Tạo Stack (capacity: %d)\n", capacity);
    return stack;
}

/**
 * Push: Thêm phần tử vào stack
 * 
 * top++
 * data[top] = value
 * 
 * Độ phức tạp: O(1)
 * Mô phỏng (push 30):
 *     Before:        After:
 *     [10, 20, _, _] [10, 20, 30, _]
 *      top = 1        top = 2
 */
void push(Stack *stack, int value) {
    if (stack->top >= stack->capacity - 1) {
        printf("⚠ Stack đầy!\n");
        return;
    }
    
    stack->top++;
    stack->data[stack->top] = value;
    printf("  ↓ Push %d. Top: %d\n", value, stack->top);
}

/**
 * Pop: Lấy phần tử từ stack
 * 
 * value = data[top]
 * top--
 * 
 * Độ phức tạp: O(1)
 */
int pop(Stack *stack) {
    if (stack->top < 0) {
        printf("⚠ Stack rỗng!\n");
        return -1;
    }
    
    int value = stack->data[stack->top];
    stack->top--;
    printf("  ↑ Pop %d. Top: %d\n", value, stack->top);
    return value;
}

/**
 * Peek: Nhìn phần tử trên cùng mà không xóa
 */
int peek(Stack *stack) {
    if (stack->top < 0) {
        printf("⚠ Stack rỗng!\n");
        return -1;
    }
    return stack->data[stack->top];
}

int isStackEmpty(Stack *stack) {
    return stack->top < 0;
}

void printStack(Stack *stack) {
    printf("Stack: [");
    for (int i = 0; i <= stack->top; i++) {
        printf("%d", stack->data[i]);
        if (i < stack->top) printf(", ");
    }
    printf("] (top: %d)\n", stack->top);
}

void freeStack(Stack *stack) {
    free(stack->data);
    free(stack);
}

// ============================================================
// PART 2: QUEUE
// ============================================================

typedef struct {
    int *data;
    int front;        // Index của phần tử đầu tiên
    int rear;         // Index của phần tử cuối cùng
    int size;         // Số phần tử hiện có
    int capacity;
} Queue;

Queue* createQueue(int capacity) {
    Queue *queue = (Queue *)malloc(sizeof(Queue));
    queue->data = (int *)malloc(capacity * sizeof(int));
    queue->capacity = capacity;
    queue->front = 0;
    queue->rear = -1;
    queue->size = 0;
    printf("✓ Tạo Queue (capacity: %d)\n", capacity);
    return queue;
}

/**
 * Enqueue: Thêm phần tử vào cuối queue
 * 
 * rear++
 * data[rear] = value
 * size++
 * 
 * Độ phức tạp: O(1)
 * Mô phỏng (enqueue 30):
 *     Before:        After:
 *     [10, 20, _, _] [10, 20, 30, _]
 *      F  R           F  R
 */
void enqueue(Queue *queue, int value) {
    if (queue->size >= queue->capacity) {
        printf("⚠ Queue đầy!\n");
        return;
    }
    
    queue->rear++;
    queue->data[queue->rear] = value;
    queue->size++;
    printf("  ← Enqueue %d. Rear: %d, Size: %d\n", value, queue->rear, queue->size);
}

/**
 * Dequeue: Lấy phần tử từ đầu queue
 * 
 * value = data[front]
 * front++
 * size--
 * 
 * Độ phức tạp: O(1)
 * 
 * ⚠ VẤNĐỀ: front tăng nhưng không bao giờ reset!
 * Sau n lần dequeue, front = n, và bộ nhớ bên trái lãng phí.
 * 
 * Giải pháp: Circular Queue
 */
int dequeue(Queue *queue) {
    if (queue->size <= 0) {
        printf("⚠ Queue rỗng!\n");
        return -1;
    }
    
    int value = queue->data[queue->front];
    queue->front++;
    queue->size--;
    printf("  → Dequeue %d. Front: %d, Size: %d\n", value, queue->front, queue->size);
    return value;
}

int isQueueEmpty(Queue *queue) {
    return queue->size == 0;
}

void printQueue(Queue *queue) {
    printf("Queue: [");
    for (int i = queue->front; i <= queue->rear; i++) {
        printf("%d", queue->data[i]);
        if (i < queue->rear) printf(", ");
    }
    printf("] (size: %d)\n", queue->size);
}

void freeQueue(Queue *queue) {
    free(queue->data);
    free(queue);
}

// ============================================================
// PART 3: CIRCULAR QUEUE (Khắc phục vấn đề Queue bình thường)
// ============================================================

/**
 * Circular Queue: Bộ nhớ được tái sử dụng theo vòng tròn
 * 
 * Mô phỏng (capacity = 5):
 * 
 * Bước 1: Enqueue 10,20,30,40,50
 * [10, 20, 30, 40, 50]
 *  F                R
 * 
 * Bước 2: Dequeue 10,20
 * [__, __, 30, 40, 50]  → front = 2
 * 
 * Bước 3: Enqueue 60
 * [60, __, 30, 40, 50]  → rear = 0 (quay lại vòng)
 *  R        F
 * 
 * Điều kiện:
 * rear = (rear + 1) % capacity
 * front = (front + 1) % capacity
 */
typedef struct {
    int *data;
    int front;
    int rear;
    int size;
    int capacity;
} CircularQueue;

CircularQueue* createCircularQueue(int capacity) {
    CircularQueue *q = (CircularQueue *)malloc(sizeof(CircularQueue));
    q->data = (int *)malloc(capacity * sizeof(int));
    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->size = 0;
    printf("✓ Tạo CircularQueue (capacity: %d)\n", capacity);
    return q;
}

void enqueueCircular(CircularQueue *q, int value) {
    if (q->size >= q->capacity) {
        printf("⚠ CircularQueue đầy!\n");
        return;
    }
    
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = value;
    q->size++;
    printf("  ← Enqueue %d. Rear: %d, Size: %d\n", value, q->rear, q->size);
}

int dequeueCircular(CircularQueue *q) {
    if (q->size <= 0) {
        printf("⚠ CircularQueue rỗng!\n");
        return -1;
    }
    
    int value = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->size--;
    printf("  → Dequeue %d. Front: %d, Size: %d\n", value, q->front, q->size);
    return value;
}

void printCircularQueue(CircularQueue *q) {
    printf("CircularQueue: [");
    if (q->size > 0) {
        int idx = q->front;
        for (int i = 0; i < q->size; i++) {
            printf("%d", q->data[idx]);
            if (i < q->size - 1) printf(", ");
            idx = (idx + 1) % q->capacity;
        }
    }
    printf("] (size: %d)\n", q->size);
}

void freeCircularQueue(CircularQueue *q) {
    free(q->data);
    free(q);
}

// ============================================================
// DEMO & TEST
// ============================================================

int main() {
    printf("===== STACK & QUEUE DEMO =====\n\n");
    
    // ---- STACK DEMO ----
    printf("--- STACK DEMO ---\n");
    Stack *stack = createStack(10);
    
    printf("\n1. Push phần tử:\n");
    push(stack, 10);
    push(stack, 20);
    push(stack, 30);
    printStack(stack);
    
    printf("\n2. Peek (nhìn mà không xóa):\n");
    printf("Top element: %d\n", peek(stack));
    printStack(stack);
    
    printf("\n3. Pop phần tử:\n");
    pop(stack);
    pop(stack);
    printStack(stack);
    
    freeStack(stack);
    
    // ---- QUEUE DEMO ----
    printf("\n--- QUEUE DEMO (Bình thường) ---\n");
    Queue *queue = createQueue(10);
    
    printf("\n1. Enqueue phần tử:\n");
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);
    printQueue(queue);
    
    printf("\n2. Dequeue phần tử:\n");
    dequeue(queue);
    printQueue(queue);
    
    printf("\n3. Enqueue thêm:\n");
    enqueue(queue, 40);
    printQueue(queue);
    
    printf("\n⚠ VẤN ĐỀ: front = %d (lãng phí bộ nhớ)\n", queue->front);
    
    freeQueue(queue);
    
    // ---- CIRCULAR QUEUE DEMO ----
    printf("\n--- CIRCULAR QUEUE DEMO (Cải tiến) ---\n");
    CircularQueue *cq = createCircularQueue(5);
    
    printf("\n1. Enqueue 10, 20, 30:\n");
    enqueueCircular(cq, 10);
    enqueueCircular(cq, 20);
    enqueueCircular(cq, 30);
    printCircularQueue(cq);
    
    printf("\n2. Dequeue 2 phần tử:\n");
    dequeueCircular(cq);
    dequeueCircular(cq);
    printCircularQueue(cq);
    
    printf("\n3. Enqueue 40, 50 (quay lại vòng):\n");
    enqueueCircular(cq, 40);
    enqueueCircular(cq, 50);
    printCircularQueue(cq);
    
    printf("\n✓ Không lãng phí bộ nhớ nữa!\n");
    
    freeCircularQueue(cq);
    
    // ---- SUMMARY ----
    printf("\n=== TÓM TẮT ===\n\n");
    printf("STACK (LIFO):\n");
    printf("  Push: O(1), Pop: O(1)\n");
    printf("  Ứng dụng: Protocol parsing, undo/redo, backtracking\n\n");
    
    printf("QUEUE (FIFO):\n");
    printf("  Enqueue: O(1), Dequeue: O(1)\n");
    printf("  Ứng dụng: Packet queue, task scheduler\n\n");
    
    printf("CIRCULAR QUEUE:\n");
    printf("  Tái sử dụng bộ nhớ hiệu quả\n");
    printf("  Dùng modulo (%) để quay vòng\n");
    
    return 0;
}

/*
 * ============================================================
 * NETWORK APPLICATION
 * ============================================================
 * 
 * 1. STACK:
 *    - HTTP parsing: match brackets, parentheses
 *    - DFS traversal: graph algorithms
 *    - Backtracking: routing protocols
 * 
 * 2. QUEUE:
 *    - Packet queue tại NIC
 *    - TCP SYN queue
 *    - Message queue trong RabbitMQ, Kafka
 * 
 * 3. CIRCULAR QUEUE:
 *    - Ring buffer tại NIC driver (cao hiệu suất)
 *    - DPDK (Data Plane Development Kit) dùng ring buffers
 *    - Network interface scheduling
 * 
 * ============================================================
 */
