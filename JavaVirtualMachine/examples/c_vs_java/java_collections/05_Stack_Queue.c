/*
 * 05_Stack_Queue.c  —  Java Stack, ArrayDeque, PriorityQueue in C
 *
 * Build:  gcc -Wall -Wextra -o 05_Stack_Queue 05_Stack_Queue.c
 * Run:    ./05_Stack_Queue
 *
 * ─────────────────────────────────────────────────────────────────────
 * WHAT THIS COVERS:
 *
 *  1. Stack  — LIFO  — backed by array  (Java: Deque<E> stack = new ArrayDeque<>())
 *  2. Queue  — FIFO  — circular buffer  (Java: Queue<E> q = new ArrayDeque<>())
 *  3. PriorityQueue — min-heap          (Java: PriorityQueue<Integer>)
 *
 * HOW JAVA IMPLEMENTS THESE:
 *
 *  ArrayDeque (used for Stack + Queue):
 *    - Backed by a CIRCULAR ARRAY (Object[])
 *    - head and tail pointers
 *    - double capacity when full
 *    - addFirst/addLast: O(1) amortized
 *    - Not thread-safe (use ArrayBlockingQueue for concurrent)
 *
 *  PriorityQueue:
 *    - Backed by a BINARY HEAP stored in an array
 *    - Parent at i → children at 2i+1 and 2i+2
 *    - offer(e): O(log n)  — sift up
 *    - poll():   O(log n)  — sift down (heapify)
 *    - peek():   O(1)      — just read index 0
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  1. STACK — backed by dynamic array
 *  Java: Deque<Integer> stack = new ArrayDeque<>();
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int *data;
    int  top;       /* index of next empty slot */
    int  capacity;
} Stack;

Stack *stack_new(void) {
    Stack *s = malloc(sizeof(Stack));
    s->capacity = 8;
    s->data     = malloc(s->capacity * sizeof(int));
    s->top      = 0;
    printf("[ALLOC] Stack  capacity=%d\n", s->capacity);
    return s;
}

void stack_free(Stack *s) { free(s->data); free(s); }

static void stack_grow(Stack *s) {
    s->capacity *= 2;
    s->data = realloc(s->data, s->capacity * sizeof(int));
    printf("[GROW]  Stack capacity → %d\n", s->capacity);
}

/* stack.push(e) */
void stack_push(Stack *s, int e) {
    if (s->top == s->capacity) stack_grow(s);
    s->data[s->top++] = e;
}

/* stack.pop() */
int stack_pop(Stack *s) {
    if (s->top == 0) { fprintf(stderr, "EmptyStackException\n"); exit(1); }
    return s->data[--s->top];
}

/* stack.peek() */
int stack_peek(const Stack *s) {
    if (s->top == 0) { fprintf(stderr, "EmptyStackException\n"); exit(1); }
    return s->data[s->top - 1];
}

int stack_size(const Stack *s)     { return s->top; }
int stack_is_empty(const Stack *s) { return s->top == 0; }

void stack_print(const Stack *s) {
    printf("Stack (top→): [");
    for (int i = s->top - 1; i >= 0; i--) {
        printf("%d", s->data[i]);
        if (i > 0) printf(", ");
    }
    printf("]  size=%d\n", s->top);
}

/* ═══════════════════════════════════════════════════════════
 *  2. QUEUE — circular buffer (how Java's ArrayDeque works)
 *  Java: Queue<Integer> q = new ArrayDeque<>();
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int *data;
    int  head;      /* index of next element to dequeue */
    int  tail;      /* index of next empty slot         */
    int  size;
    int  capacity;
} Queue;

Queue *queue_new(void) {
    Queue *q = malloc(sizeof(Queue));
    q->capacity = 8;
    q->data     = malloc(q->capacity * sizeof(int));
    q->head = q->tail = q->size = 0;
    printf("[ALLOC] Queue  capacity=%d  (circular buffer)\n", q->capacity);
    return q;
}

void queue_free(Queue *q) { free(q->data); free(q); }

static void queue_grow(Queue *q) {
    int new_cap = q->capacity * 2;
    int *new_data = malloc(new_cap * sizeof(int));

    /* Copy elements in order from head to tail */
    for (int i = 0; i < q->size; i++)
        new_data[i] = q->data[(q->head + i) % q->capacity];

    free(q->data);
    q->data     = new_data;
    q->head     = 0;
    q->tail     = q->size;
    q->capacity = new_cap;
    printf("[GROW]  Queue capacity → %d\n", new_cap);
}

/* queue.offer(e) — enqueue at tail */
void queue_offer(Queue *q, int e) {
    if (q->size == q->capacity) queue_grow(q);
    q->data[q->tail] = e;
    q->tail  = (q->tail + 1) % q->capacity;  /* wrap around */
    q->size++;
}

/* queue.poll() — dequeue from head */
int queue_poll(Queue *q) {
    if (q->size == 0) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    int e    = q->data[q->head];
    q->head  = (q->head + 1) % q->capacity;  /* wrap around */
    q->size--;
    return e;
}

/* queue.peek() */
int queue_peek(const Queue *q) {
    if (q->size == 0) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return q->data[q->head];
}

int queue_size(const Queue *q)     { return q->size; }
int queue_is_empty(const Queue *q) { return q->size == 0; }

void queue_print(const Queue *q) {
    printf("Queue (head→tail): [");
    for (int i = 0; i < q->size; i++) {
        printf("%d", q->data[(q->head + i) % q->capacity]);
        if (i < q->size - 1) printf(", ");
    }
    printf("]  size=%d  head=%d  tail=%d  capacity=%d\n",
           q->size, q->head, q->tail, q->capacity);
}

/* Show raw circular buffer memory */
void queue_debug(const Queue *q) {
    printf("  Circular buffer (capacity=%d):\n", q->capacity);
    printf("  index: ");
    for (int i = 0; i < q->capacity; i++) printf("[%2d] ", i);
    printf("\n  value: ");
    for (int i = 0; i < q->capacity; i++) {
        int idx = (q->head + i) % q->capacity;
        if (i < q->size) printf("[%3d] ", q->data[idx]);
        else              printf("[  _] ");
    }
    printf("\n  head=%d  tail=%d\n", q->head, q->tail);
}

/* ═══════════════════════════════════════════════════════════
 *  3. PRIORITY QUEUE — min-heap stored in array
 *  Java: PriorityQueue<Integer> pq = new PriorityQueue<>();
 *
 *  HEAP PROPERTY: parent <= both children (min-heap)
 *  ARRAY LAYOUT:
 *    index 0 = root (minimum)
 *    parent of i   = (i-1) / 2
 *    left child  of i = 2*i + 1
 *    right child of i = 2*i + 2
 * ═══════════════════════════════════════════════════════════ */

typedef struct {
    int *data;
    int  size;
    int  capacity;
} PriorityQueue;

PriorityQueue *pq_new(void) {
    PriorityQueue *pq = malloc(sizeof(PriorityQueue));
    pq->capacity = 11;   /* Java's default initial capacity */
    pq->data     = malloc(pq->capacity * sizeof(int));
    pq->size     = 0;
    printf("[ALLOC] PriorityQueue  capacity=%d  (min-heap)\n", pq->capacity);
    return pq;
}

void pq_free(PriorityQueue *pq) { free(pq->data); free(pq); }

static void swap(int *a, int *b) { int t = *a; *a = *b; *b = t; }

/* Sift up — restore heap after insert */
static void sift_up(PriorityQueue *pq, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->data[parent] <= pq->data[i]) break;  /* heap satisfied */
        swap(&pq->data[parent], &pq->data[i]);
        i = parent;
    }
}

/* Sift down — restore heap after remove root */
static void sift_down(PriorityQueue *pq, int i) {
    int n = pq->size;
    while (1) {
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left  < n && pq->data[left]  < pq->data[smallest]) smallest = left;
        if (right < n && pq->data[right] < pq->data[smallest]) smallest = right;
        if (smallest == i) break;  /* heap satisfied */

        swap(&pq->data[i], &pq->data[smallest]);
        i = smallest;
    }
}

/* pq.offer(e) — insert O(log n) */
void pq_offer(PriorityQueue *pq, int e) {
    if (pq->size == pq->capacity) {
        pq->capacity = pq->capacity + (pq->capacity >> 1);
        pq->data = realloc(pq->data, pq->capacity * sizeof(int));
    }
    pq->data[pq->size++] = e;  /* append at end */
    sift_up(pq, pq->size - 1); /* restore heap */
}

/* pq.peek() — minimum O(1) */
int pq_peek(const PriorityQueue *pq) {
    if (pq->size == 0) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return pq->data[0];   /* root is always minimum in min-heap */
}

/* pq.poll() — remove minimum O(log n) */
int pq_poll(PriorityQueue *pq) {
    if (pq->size == 0) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    int min = pq->data[0];
    pq->data[0] = pq->data[--pq->size];  /* move last to root */
    if (pq->size > 0) sift_down(pq, 0);  /* restore heap from root */
    return min;
}

int pq_size(const PriorityQueue *pq)     { return pq->size; }
int pq_is_empty(const PriorityQueue *pq) { return pq->size == 0; }

void pq_print_heap_array(const PriorityQueue *pq) {
    printf("  Heap array: [");
    for (int i = 0; i < pq->size; i++) {
        printf("%d", pq->data[i]);
        if (i < pq->size - 1) printf(", ");
    }
    printf("]  size=%d\n", pq->size);
}

void pq_print_tree(const PriorityQueue *pq) {
    if (pq->size == 0) { printf("  (empty)\n"); return; }
    printf("  Tree view (root = minimum):\n");
    int level_start = 0, level_size = 1;
    while (level_start < pq->size) {
        printf("    ");
        for (int i = level_start;
             i < level_start + level_size && i < pq->size; i++)
            printf("%3d ", pq->data[i]);
        printf("\n");
        level_start += level_size;
        level_size  *= 2;
    }
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

void demo_stack(void) {
    printf("━━━ 1. Stack (LIFO — Deque as stack) ━━━━━━━━━━━━━━━━━\n");
    Stack *s = stack_new();

    printf("push(10), push(20), push(30):\n");
    stack_push(s, 10);
    stack_push(s, 20);
    stack_push(s, 30);
    stack_print(s);

    printf("peek() = %d  (top, not removed)\n", stack_peek(s));
    printf("pop()  = %d\n", stack_pop(s));
    printf("pop()  = %d\n", stack_pop(s));
    stack_print(s);

    /* Use case: balanced parentheses check */
    printf("\nUse case: balanced parentheses '({[]})'\n");
    const char *expr = "({[]})";
    Stack *brackets = stack_new();
    int balanced = 1;
    for (int i = 0; expr[i] && balanced; i++) {
        char c = expr[i];
        if (c == '(' || c == '{' || c == '[')
            stack_push(brackets, c);
        else if (c == ')' || c == '}' || c == ']') {
            if (stack_is_empty(brackets)) { balanced = 0; break; }
            int top = stack_pop(brackets);
            if ((c == ')' && top != '(') ||
                (c == '}' && top != '{') ||
                (c == ']' && top != '[')) balanced = 0;
        }
    }
    balanced = balanced && stack_is_empty(brackets);
    printf("'%s' is %s\n", expr, balanced ? "balanced ✓" : "NOT balanced ✗");
    stack_free(brackets);
    stack_free(s);
    printf("\n");
}

void demo_queue(void) {
    printf("━━━ 2. Queue (FIFO — circular buffer) ━━━━━━━━━━━━━━━━\n");
    Queue *q = queue_new();

    printf("offer(1), offer(2), offer(3):\n");
    queue_offer(q, 1);
    queue_offer(q, 2);
    queue_offer(q, 3);
    queue_print(q);
    queue_debug(q);

    printf("poll() = %d  (FIFO — first in, first out)\n", queue_poll(q));
    printf("poll() = %d\n", queue_poll(q));
    queue_print(q);

    printf("\nForce circular wrap-around (offer 8 more to overflow head):\n");
    for (int i = 4; i <= 10; i++) queue_offer(q, i);
    queue_print(q);
    queue_debug(q);

    queue_free(q);
    printf("\n");
}

void demo_priority_queue(void) {
    printf("━━━ 3. PriorityQueue (min-heap) ━━━━━━━━━━━━━━━━━━━━━━\n");
    PriorityQueue *pq = pq_new();

    int values[] = {50, 30, 80, 10, 40, 20, 70};
    int n = 7;
    printf("Inserting: ");
    for (int i = 0; i < n; i++) {
        printf("%d ", values[i]);
        pq_offer(pq, values[i]);
    }
    printf("\n");

    printf("After all inserts:\n");
    pq_print_heap_array(pq);
    pq_print_tree(pq);

    printf("\npeek() = %d  (minimum, O(1))\n", pq_peek(pq));

    printf("\npoll() removes minimum each time — natural sort order:\n");
    printf("  ");
    while (!pq_is_empty(pq)) {
        printf("%d ", pq_poll(pq));
    }
    printf("\n");

    /* Real use case: top-K problem */
    printf("\nUse case: find 3 smallest from {9,4,7,1,8,2,6,3,5}\n");
    int nums[] = {9, 4, 7, 1, 8, 2, 6, 3, 5};
    PriorityQueue *kpq = pq_new();
    for (int i = 0; i < 9; i++) pq_offer(kpq, nums[i]);
    printf("Top 3 smallest: ");
    for (int i = 0; i < 3; i++) printf("%d ", pq_poll(kpq));
    printf("\n");

    pq_free(kpq);
    pq_free(pq);
    printf("\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  05_Stack_Queue.c — Stack / Queue / PriorityQueue   ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_stack();
    demo_queue();
    demo_priority_queue();

    printf("━━━ Complexity Summary ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  Stack (ArrayDeque):    push O(1) | pop O(1) | peek O(1)\n");
    printf("  Queue (ArrayDeque):    offer O(1)| poll O(1)| peek O(1)\n");
    printf("  PriorityQueue (heap):  offer O(log n)| poll O(log n)| peek O(1)\n");

    return 0;
}
