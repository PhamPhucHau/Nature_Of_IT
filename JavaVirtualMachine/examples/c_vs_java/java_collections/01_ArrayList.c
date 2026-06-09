/*
 * 01_ArrayList.c  —  Java's ArrayList<Integer> implemented in C
 *
 * Build:  gcc -Wall -Wextra -o 01_ArrayList 01_ArrayList.c
 * Run:    ./01_ArrayList
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA ArrayList WORKS INTERNALLY:
 *
 *   ArrayList stores elements in a plain C array (Object[]).
 *   When that array is full, it creates a NEW array that is 1.5× bigger
 *   and copies everything over.  This is called "grow".
 *
 *   DEFAULT_CAPACITY = 10
 *   GROW factor      = newCapacity = oldCapacity + (oldCapacity >> 1)
 *                    = oldCapacity * 1.5   (right-shift divides by 2)
 *
 *   Java Source (simplified):
 *     private Object[] elementData;
 *     private int      size;
 *
 *     void add(E e) {
 *         ensureCapacityInternal(size + 1);
 *         elementData[size++] = e;
 *     }
 *     private void grow(int minCapacity) {
 *         int oldCap = elementData.length;
 *         int newCap = oldCap + (oldCap >> 1);   // *1.5
 *         elementData = Arrays.copyOf(elementData, newCap);
 *     }
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════
 *  STRUCTURE  — mirrors Java's ArrayList fields exactly
 * ═══════════════════════════════════════════════════════════ */
#define ARRAYLIST_DEFAULT_CAPACITY 10

typedef struct {
    int   *elementData;   /* Object[] elementData  — the backing array   */
    int    size;          /* int      size          — number of elements  */
    int    capacity;      /* elementData.length     — array length        */
} ArrayList;

/* ═══════════════════════════════════════════════════════════
 *  LIFECYCLE
 * ═══════════════════════════════════════════════════════════ */

/* new ArrayList<>()  — default constructor */
ArrayList *arraylist_new(void) {
    ArrayList *list = malloc(sizeof(ArrayList));
    list->elementData = malloc(ARRAYLIST_DEFAULT_CAPACITY * sizeof(int));
    list->size        = 0;
    list->capacity    = ARRAYLIST_DEFAULT_CAPACITY;
    printf("[ALLOC] ArrayList created  capacity=%d\n", list->capacity);
    return list;
}

/* new ArrayList<>(initialCapacity) */
ArrayList *arraylist_new_with_capacity(int initialCapacity) {
    ArrayList *list = malloc(sizeof(ArrayList));
    list->elementData = malloc(initialCapacity * sizeof(int));
    list->size        = 0;
    list->capacity    = initialCapacity;
    printf("[ALLOC] ArrayList created  capacity=%d\n", list->capacity);
    return list;
}

/* GC / free */
void arraylist_free(ArrayList *list) {
    free(list->elementData);
    free(list);
    printf("[FREE]  ArrayList destroyed\n");
}

/* ═══════════════════════════════════════════════════════════
 *  GROW  — the key private method in ArrayList
 *  newCapacity = oldCapacity + (oldCapacity >> 1)   = * 1.5
 * ═══════════════════════════════════════════════════════════ */
static void arraylist_grow(ArrayList *list) {
    int oldCap = list->capacity;
    int newCap = oldCap + (oldCap >> 1);   /* same formula as OpenJDK */
    list->elementData = realloc(list->elementData, newCap * sizeof(int));
    list->capacity    = newCap;
    printf("[GROW]  capacity: %d → %d  (×1.5)\n", oldCap, newCap);
}

static void ensure_capacity(ArrayList *list) {
    if (list->size == list->capacity)
        arraylist_grow(list);
}

/* ═══════════════════════════════════════════════════════════
 *  PUBLIC API
 * ═══════════════════════════════════════════════════════════ */

/* list.add(e) — append */
void arraylist_add(ArrayList *list, int e) {
    ensure_capacity(list);
    list->elementData[list->size++] = e;
}

/* list.add(index, e) — insert at position, shift right */
void arraylist_add_at(ArrayList *list, int index, int e) {
    if (index < 0 || index > list->size) {
        fprintf(stderr, "IndexOutOfBoundsException: index=%d size=%d\n",
                index, list->size);
        exit(1);
    }
    ensure_capacity(list);
    /* shift elements right — memmove handles overlap correctly */
    memmove(list->elementData + index + 1,
            list->elementData + index,
            (list->size - index) * sizeof(int));
    list->elementData[index] = e;
    list->size++;
}

/* list.get(index) */
int arraylist_get(const ArrayList *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBoundsException: index=%d size=%d\n",
                index, list->size);
        exit(1);
    }
    return list->elementData[index];
}

/* list.set(index, e) — replace, return old value */
int arraylist_set(ArrayList *list, int index, int e) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBoundsException\n"); exit(1);
    }
    int old = list->elementData[index];
    list->elementData[index] = e;
    return old;
}

/* list.remove(index) — remove at position, shift left */
int arraylist_remove(ArrayList *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBoundsException\n"); exit(1);
    }
    int removed = list->elementData[index];
    /* shift elements left */
    memmove(list->elementData + index,
            list->elementData + index + 1,
            (list->size - index - 1) * sizeof(int));
    list->size--;
    /* Java sets the freed slot to null (for GC) */
    list->elementData[list->size] = 0;
    return removed;
}

/* list.size() */
int arraylist_size(const ArrayList *list) { return list->size; }

/* list.isEmpty() */
int arraylist_is_empty(const ArrayList *list) { return list->size == 0; }

/* list.contains(value) — linear scan O(n) */
int arraylist_contains(const ArrayList *list, int value) {
    for (int i = 0; i < list->size; i++)
        if (list->elementData[i] == value) return 1;
    return 0;
}

/* list.indexOf(value) — first occurrence, -1 if missing */
int arraylist_index_of(const ArrayList *list, int value) {
    for (int i = 0; i < list->size; i++)
        if (list->elementData[i] == value) return i;
    return -1;
}

/* list.clear() */
void arraylist_clear(ArrayList *list) {
    memset(list->elementData, 0, list->size * sizeof(int));
    list->size = 0;
}

/* Arrays.sort equivalent — insertion sort (simple demo) */
void arraylist_sort(ArrayList *list) {
    for (int i = 1; i < list->size; i++) {
        int key = list->elementData[i];
        int j   = i - 1;
        while (j >= 0 && list->elementData[j] > key) {
            list->elementData[j + 1] = list->elementData[j];
            j--;
        }
        list->elementData[j + 1] = key;
    }
}

/* list.subList(from, to) — returns new ArrayList */
ArrayList *arraylist_sublist(const ArrayList *list, int from, int to) {
    if (from < 0 || to > list->size || from > to) {
        fprintf(stderr, "IndexOutOfBoundsException: subList(%d,%d)\n", from, to);
        exit(1);
    }
    ArrayList *sub = arraylist_new_with_capacity(to - from);
    for (int i = from; i < to; i++)
        arraylist_add(sub, list->elementData[i]);
    return sub;
}

/* forEach(consumer) — equivalent of list.forEach(System.out::println) */
void arraylist_foreach(const ArrayList *list, void (*consumer)(int)) {
    for (int i = 0; i < list->size; i++)
        consumer(list->elementData[i]);
}

/* Pretty print — like Arrays.toString() */
void arraylist_print(const ArrayList *list) {
    printf("[");
    for (int i = 0; i < list->size; i++) {
        printf("%d", list->elementData[i]);
        if (i < list->size - 1) printf(", ");
    }
    printf("]  size=%d capacity=%d\n", list->size, list->capacity);
}

/* Internal memory layout — educational dump */
void arraylist_debug(const ArrayList *list) {
    printf("  ┌─ elementData (backing array at %p) ──────────────\n",
           (void *)list->elementData);
    printf("  │  capacity = %d slots  (allocated: %zu bytes)\n",
           list->capacity, list->capacity * sizeof(int));
    printf("  │  size     = %d  (elements in use)\n", list->size);
    printf("  │  wasted   = %d  (empty slots)\n", list->capacity - list->size);
    printf("  │\n");
    printf("  │  index: ");
    for (int i = 0; i < list->capacity; i++) printf("[%2d] ", i);
    printf("\n");
    printf("  │  value: ");
    for (int i = 0; i < list->capacity; i++) {
        if (i < list->size) printf("[%3d] ", list->elementData[i]);
        else                printf("[  _] ");
    }
    printf("  ← _ = unused\n");
    printf("  └──────────────────────────────────────────────────\n");
}

/* ═══════════════════════════════════════════════════════════
 *  DEMOS
 * ═══════════════════════════════════════════════════════════ */

static void print_item(int x) { printf("  forEach: %d\n", x); }

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  01_ArrayList.c  — ArrayList<Integer> in C          ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── Demo 1: Basic operations ── */
    printf("━━━ 1. Basic add / get / size ━━━━━━━━━━━━━━━━━━━━━━━━\n");
    ArrayList *list = arraylist_new();

    arraylist_add(list, 10);
    arraylist_add(list, 20);
    arraylist_add(list, 30);
    arraylist_add(list, 40);
    arraylist_add(list, 50);

    printf("After 5 adds: ");
    arraylist_print(list);
    printf("list.get(2) = %d\n", arraylist_get(list, 2));
    printf("list.size() = %d\n", arraylist_size(list));
    printf("\n");

    /* ── Demo 2: GROW — watch the capacity expand ── */
    printf("━━━ 2. GROW — adding 11 elements forces resize ━━━━━━━━\n");
    ArrayList *growing = arraylist_new();   /* capacity=10 */
    for (int i = 1; i <= 15; i++)
        arraylist_add(growing, i * 100);
    printf("After 15 adds: ");
    arraylist_print(growing);
    arraylist_debug(growing);
    arraylist_free(growing);
    printf("\n");

    /* ── Demo 3: Insert at index ── */
    printf("━━━ 3. add(index, e) — insert and shift ━━━━━━━━━━━━━━\n");
    printf("Before: ");
    arraylist_print(list);
    arraylist_add_at(list, 2, 999);  /* insert 999 at position 2 */
    printf("After add_at(2, 999): ");
    arraylist_print(list);
    printf("\n");

    /* ── Demo 4: remove ── */
    printf("━━━ 4. remove(index) — remove and shift left ━━━━━━━━━\n");
    int removed = arraylist_remove(list, 2);
    printf("Removed element: %d\n", removed);
    printf("After remove(2): ");
    arraylist_print(list);
    printf("\n");

    /* ── Demo 5: set / contains / indexOf ── */
    printf("━━━ 5. set / contains / indexOf ━━━━━━━━━━━━━━━━━━━━━━\n");
    int old = arraylist_set(list, 0, 100);
    printf("set(0, 100), old value = %d\n", old);
    printf("contains(30) = %d\n", arraylist_contains(list, 30));
    printf("contains(99) = %d\n", arraylist_contains(list, 99));
    printf("indexOf(40)  = %d\n", arraylist_index_of(list, 40));
    printf("\n");

    /* ── Demo 6: sort ── */
    printf("━━━ 6. sort ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    ArrayList *unsorted = arraylist_new();
    arraylist_add(unsorted, 50);
    arraylist_add(unsorted, 10);
    arraylist_add(unsorted, 40);
    arraylist_add(unsorted, 20);
    arraylist_add(unsorted, 30);
    printf("Before sort: "); arraylist_print(unsorted);
    arraylist_sort(unsorted);
    printf("After sort:  "); arraylist_print(unsorted);
    arraylist_free(unsorted);
    printf("\n");

    /* ── Demo 7: subList ── */
    printf("━━━ 7. subList(1, 4) ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Original: "); arraylist_print(list);
    ArrayList *sub = arraylist_sublist(list, 1, 4);
    printf("subList(1,4): "); arraylist_print(sub);
    arraylist_free(sub);
    printf("\n");

    /* ── Demo 8: forEach ── */
    printf("━━━ 8. forEach ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    arraylist_foreach(list, print_item);
    printf("\n");

    /* ── Demo 9: clear ── */
    printf("━━━ 9. clear ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    arraylist_clear(list);
    printf("After clear: "); arraylist_print(list);
    printf("isEmpty() = %d\n", arraylist_is_empty(list));
    printf("\n");

    /* ── Summary: memory layout explained ── */
    printf("━━━ Internal Layout ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Java ArrayList fields:\n");
    printf("  Object[] elementData  = the backing C array\n");
    printf("  int      size         = how many slots are used\n");
    printf("  (capacity             = elementData.length)\n\n");
    printf("Key insight:\n");
    printf("  add() is O(1) amortized — most of the time just arr[size++]=e\n");
    printf("  get(i) is O(1)          — direct array access\n");
    printf("  add(i, e) is O(n)       — must shift n-i elements right\n");
    printf("  remove(i) is O(n)       — must shift n-i elements left\n");

    arraylist_free(list);
    return 0;
}
