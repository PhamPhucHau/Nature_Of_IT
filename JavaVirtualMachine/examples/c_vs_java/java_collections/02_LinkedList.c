/*
 * 02_LinkedList.c  —  Java's LinkedList<Integer> implemented in C
 *
 * Build:  gcc -Wall -Wextra -o 02_LinkedList 02_LinkedList.c
 * Run:    ./02_LinkedList
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA LinkedList WORKS INTERNALLY:
 *
 *   LinkedList is a DOUBLY-LINKED LIST with sentinel head+tail nodes.
 *   Each node (called "Node<E>") holds:
 *       E    item;         // the element
 *       Node prev;         // pointer to previous
 *       Node next;         // pointer to next
 *
 *   The LinkedList itself holds:
 *       int  size;
 *       Node first;        // = head sentinel
 *       Node last;         // = tail sentinel
 *
 *   Java Source (simplified from OpenJDK):
 *     private static class Node<E> {
 *         E item;
 *         Node<E> next;
 *         Node<E> prev;
 *         Node(Node<E> prev, E element, Node<E> next) { ... }
 *     }
 *     private Node<E> first;
 *     private Node<E> last;
 *     private int     size;
 *
 *   addFirst(e):  linkFirst(e)  → new node before first
 *   addLast(e):   linkLast(e)   → new node after last
 *   get(i):       node(i)       → traverse from head or tail (O(n/2))
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  STRUCTURE  — mirrors Java's LinkedList.Node
 * ═══════════════════════════════════════════════════════════ */

typedef struct Node {
    int          item;   /* E item  */
    struct Node *prev;   /* Node<E> prev */
    struct Node *next;   /* Node<E> next */
} Node;

typedef struct {
    Node *first;    /* first node  (Java: first) */
    Node *last;     /* last node   (Java: last)  */
    int   size;     /* element count             */
} LinkedList;

/* ═══════════════════════════════════════════════════════════
 *  LIFECYCLE
 * ═══════════════════════════════════════════════════════════ */

LinkedList *linkedlist_new(void) {
    LinkedList *list = calloc(1, sizeof(LinkedList));
    printf("[ALLOC] LinkedList created\n");
    return list;
}

void linkedlist_free(LinkedList *list) {
    Node *cur = list->first;
    int freed = 0;
    while (cur) {
        Node *next = cur->next;
        free(cur);
        cur = next;
        freed++;
    }
    free(list);
    printf("[FREE]  LinkedList destroyed  (%d nodes freed)\n", freed);
}

/* ═══════════════════════════════════════════════════════════
 *  PRIVATE HELPERS — direct mirror of OpenJDK's linkFirst / linkLast
 * ═══════════════════════════════════════════════════════════ */

static Node *new_node(Node *prev, int item, Node *next) {
    Node *n = malloc(sizeof(Node));
    n->prev = prev;
    n->item = item;
    n->next = next;
    return n;
}

/* linkFirst(e) — insert before first */
static void link_first(LinkedList *list, int e) {
    Node *f    = list->first;
    Node *node = new_node(NULL, e, f);  /* prev=null, next=f */
    list->first = node;
    if (f == NULL)
        list->last = node;   /* was empty — this is also last */
    else
        f->prev = node;
    list->size++;
}

/* linkLast(e) — append after last */
static void link_last(LinkedList *list, int e) {
    Node *l    = list->last;
    Node *node = new_node(l, e, NULL);  /* prev=l, next=null */
    list->last = node;
    if (l == NULL)
        list->first = node;  /* was empty — this is also first */
    else
        l->next = node;
    list->size++;
}

/* node(index) — O(n/2): traverse from nearest end */
static Node *node_at(const LinkedList *list, int index) {
    if (index < (list->size >> 1)) {
        /* traverse forward from first */
        Node *x = list->first;
        for (int i = 0; i < index; i++) x = x->next;
        return x;
    } else {
        /* traverse backward from last */
        Node *x = list->last;
        for (int i = list->size - 1; i > index; i--) x = x->prev;
        return x;
    }
}

/* unlink(node) — remove a known node in O(1) */
static int unlink(LinkedList *list, Node *node) {
    int element = node->item;
    Node *next  = node->next;
    Node *prev  = node->prev;

    if (prev == NULL)
        list->first = next;   /* was first node */
    else {
        prev->next  = next;
        node->prev  = NULL;
    }

    if (next == NULL)
        list->last = prev;    /* was last node */
    else {
        next->prev = prev;
        node->next = NULL;
    }

    node->item = 0;           /* Java nulls item for GC */
    free(node);
    list->size--;
    return element;
}

/* ═══════════════════════════════════════════════════════════
 *  PUBLIC API
 * ═══════════════════════════════════════════════════════════ */

/* list.add(e) = addLast(e) */
void linkedlist_add(LinkedList *list, int e) {
    link_last(list, e);
}

/* list.addFirst(e) */
void linkedlist_add_first(LinkedList *list, int e) {
    link_first(list, e);
}

/* list.addLast(e) */
void linkedlist_add_last(LinkedList *list, int e) {
    link_last(list, e);
}

/* list.add(index, e) */
void linkedlist_add_at(LinkedList *list, int index, int e) {
    if (index < 0 || index > list->size) {
        fprintf(stderr, "IndexOutOfBoundsException: %d\n", index); exit(1);
    }
    if (index == list->size)
        link_last(list, e);
    else {
        Node *succ = node_at(list, index);
        Node *pred = succ->prev;
        Node *node = new_node(pred, e, succ);
        succ->prev = node;
        if (pred == NULL)
            list->first = node;
        else
            pred->next = node;
        list->size++;
    }
}

/* list.get(index) — O(n/2) */
int linkedlist_get(const LinkedList *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBoundsException: %d\n", index); exit(1);
    }
    return node_at(list, index)->item;
}

/* list.getFirst() */
int linkedlist_get_first(const LinkedList *list) {
    if (!list->first) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return list->first->item;
}

/* list.getLast() */
int linkedlist_get_last(const LinkedList *list) {
    if (!list->last) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return list->last->item;
}

/* list.removeFirst() — O(1) */
int linkedlist_remove_first(LinkedList *list) {
    if (!list->first) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return unlink(list, list->first);
}

/* list.removeLast() — O(1) */
int linkedlist_remove_last(LinkedList *list) {
    if (!list->last) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return unlink(list, list->last);
}

/* list.remove(index) — O(n) to find node, O(1) to unlink */
int linkedlist_remove_at(LinkedList *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBoundsException: %d\n", index); exit(1);
    }
    return unlink(list, node_at(list, index));
}

/* list.size() */
int linkedlist_size(const LinkedList *list) { return list->size; }

/* list.isEmpty() */
int linkedlist_is_empty(const LinkedList *list) { return list->size == 0; }

/* list.contains(value) — linear scan O(n) */
int linkedlist_contains(const LinkedList *list, int value) {
    Node *cur = list->first;
    while (cur) {
        if (cur->item == value) return 1;
        cur = cur->next;
    }
    return 0;
}

/* Queue operations — LinkedList implements Deque */
void linkedlist_push(LinkedList *list, int e) { link_first(list, e); }   /* stack push */
int  linkedlist_pop (LinkedList *list)         { return linkedlist_remove_first(list); }  /* stack pop  */
void linkedlist_offer(LinkedList *list, int e) { link_last(list, e); }   /* queue enqueue */
int  linkedlist_poll (LinkedList *list)         { return linkedlist_remove_first(list); } /* queue dequeue */
int  linkedlist_peek (const LinkedList *list) {
    if (!list->first) { fprintf(stderr, "NoSuchElementException\n"); exit(1); }
    return list->first->item;
}

void linkedlist_print(const LinkedList *list) {
    printf("[");
    Node *cur = list->first;
    while (cur) {
        printf("%d", cur->item);
        if (cur->next) printf(" ↔ ");
        cur = cur->next;
    }
    printf("]  size=%d\n", list->size);
}

/* Visual memory layout */
void linkedlist_debug(const LinkedList *list) {
    printf("  first→%p   last→%p   size=%d\n",
           (void *)list->first, (void *)list->last, list->size);
    Node *cur = list->first;
    int i = 0;
    while (cur) {
        printf("  [%d] addr=%p  item=%3d  prev=%p  next=%p\n",
               i++, (void *)cur, cur->item,
               (void *)cur->prev, (void *)cur->next);
        cur = cur->next;
    }
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  02_LinkedList.c — LinkedList<Integer> in C         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── Demo 1: addLast / addFirst ── */
    printf("━━━ 1. addLast / addFirst ━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    LinkedList *list = linkedlist_new();
    linkedlist_add(list, 20);
    linkedlist_add(list, 30);
    linkedlist_add(list, 40);
    linkedlist_add_first(list, 10);   /* prepend */
    linkedlist_add_last(list, 50);    /* append  */
    printf("After adds: "); linkedlist_print(list);
    printf("\n");

    /* ── Demo 2: Memory layout ── */
    printf("━━━ 2. Internal Node Layout ━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    linkedlist_debug(list);
    printf("\n");

    /* ── Demo 3: get (bidirectional traversal) ── */
    printf("━━━ 3. get(index) — O(n/2) bidirectional search ━━━━━━\n");
    printf("  size=%d, searching from %s for index %d\n",
           list->size, "head", 1);
    printf("  list.get(1) = %d  (from head: 1 step)\n",   linkedlist_get(list, 1));
    printf("  list.get(4) = %d  (from tail: 0 steps)\n",  linkedlist_get(list, 4));
    printf("  list.get(0) = %d  (getFirst)\n", linkedlist_get_first(list));
    printf("  list.getLast() = %d\n", linkedlist_get_last(list));
    printf("\n");

    /* ── Demo 4: remove ── */
    printf("━━━ 4. remove operations ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Before: "); linkedlist_print(list);
    printf("removeFirst() = %d\n", linkedlist_remove_first(list));
    printf("removeLast()  = %d\n", linkedlist_remove_last(list));
    printf("remove_at(1)  = %d\n", linkedlist_remove_at(list, 1));
    printf("After:  "); linkedlist_print(list);
    printf("\n");

    /* ── Demo 5: LinkedList as Stack (LIFO) ── */
    printf("━━━ 5. LinkedList as Stack (LIFO) ━━━━━━━━━━━━━━━━━━━━\n");
    LinkedList *stack = linkedlist_new();
    printf("  push(1), push(2), push(3):\n");
    linkedlist_push(stack, 1);
    linkedlist_push(stack, 2);
    linkedlist_push(stack, 3);
    linkedlist_print(stack);
    printf("  pop() = %d\n", linkedlist_pop(stack));
    printf("  pop() = %d\n", linkedlist_pop(stack));
    printf("  peek() = %d\n", linkedlist_peek(stack));
    linkedlist_free(stack);
    printf("\n");

    /* ── Demo 6: LinkedList as Queue (FIFO) ── */
    printf("━━━ 6. LinkedList as Queue (FIFO) ━━━━━━━━━━━━━━━━━━━━\n");
    LinkedList *queue = linkedlist_new();
    printf("  offer(A=1), offer(B=2), offer(C=3):\n");
    linkedlist_offer(queue, 1);
    linkedlist_offer(queue, 2);
    linkedlist_offer(queue, 3);
    linkedlist_print(queue);
    printf("  poll() = %d  (FIFO order)\n", linkedlist_poll(queue));
    printf("  poll() = %d\n", linkedlist_poll(queue));
    linkedlist_free(queue);
    printf("\n");

    /* ── Summary ── */
    printf("━━━ Complexity Summary ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  addFirst/addLast:  O(1)  — just pointer updates\n");
    printf("  removeFirst/Last:  O(1)  — just pointer updates\n");
    printf("  get(i):            O(n)  — traverse n/2 nodes\n");
    printf("  add(i, e):         O(n)  — find position then O(1) insert\n");
    printf("  contains:          O(n)  — full scan\n");
    printf("\n");
    printf("  ArrayList vs LinkedList:\n");
    printf("  ┌──────────────────┬───────────┬──────────────┐\n");
    printf("  │ Operation        │ ArrayList │ LinkedList   │\n");
    printf("  ├──────────────────┼───────────┼──────────────┤\n");
    printf("  │ get(i)           │ O(1)  ←   │ O(n)         │\n");
    printf("  │ add(end)         │ O(1)*     │ O(1)         │\n");
    printf("  │ add(front)       │ O(n)      │ O(1)  ←      │\n");
    printf("  │ remove(middle)   │ O(n)      │ O(n) + O(1)  │\n");
    printf("  │ memory per elem  │ 4 bytes ← │ 24 bytes     │\n");
    printf("  └──────────────────┴───────────┴──────────────┘\n");
    printf("  * amortized\n");

    linkedlist_free(list);
    return 0;
}
