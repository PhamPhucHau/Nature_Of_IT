/*
 * 02_safe_array.c — Bounds-checked array: what Java's int[] does internally
 *
 * Build: gcc -Wall -Wextra -o 02_safe_array 02_safe_array.c
 * Run:   ./02_safe_array
 *
 * Java's array is NOT a raw C array.
 * It's a HEAP-ALLOCATED OBJECT with:
 *   - a length field (stored in the object header)
 *   - bounds checking on EVERY access
 *   - GC-managed lifetime
 *
 * This file builds that structure in C to show what the JVM does.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Object Header — every Java object has one
 * The JVM stores type info and GC info here
 * ============================================================ */
typedef struct {
    const char *class_name; /* "int[]", "String[]", etc. */
    int         ref_count;  /* simplified GC: reference count */
    int         marked;     /* for mark-sweep GC */
} ObjectHeader;

/* ============================================================
 * IntArray — models Java's int[]
 * Java:  int[] arr = new int[5];
 * C:     IntArray *arr = int_array_new(5);
 * ============================================================ */
typedef struct {
    ObjectHeader header;
    int          length;  /* Java: arr.length */
    int         *data;    /* the actual integers */
} IntArray;

/* new int[length] */
IntArray *int_array_new(int length) {
    if (length < 0) {
        /* Java: NegativeArraySizeException */
        fprintf(stderr, "NegativeArraySizeException: %d\n", length);
        return NULL;
    }
    IntArray *arr = malloc(sizeof(IntArray));
    if (!arr) return NULL;

    arr->header.class_name = "int[]";
    arr->header.ref_count  = 1;
    arr->header.marked     = 0;
    arr->length = length;
    arr->data   = calloc(length, sizeof(int));  /* Java zero-initializes arrays */
    if (!arr->data) { free(arr); return NULL; }
    return arr;
}

/* arr[index] — bounds-checked read */
int int_array_get(const IntArray *arr, int index) {
    if (!arr) {
        fprintf(stderr, "NullPointerException\n");
        exit(1);
    }
    if (index < 0 || index >= arr->length) {
        /* Java: ArrayIndexOutOfBoundsException */
        fprintf(stderr, "ArrayIndexOutOfBoundsException: "
                        "Index %d out of bounds for length %d\n",
                        index, arr->length);
        exit(1);
    }
    return arr->data[index];
}

/* arr[index] = value — bounds-checked write */
void int_array_set(IntArray *arr, int index, int value) {
    if (!arr) {
        fprintf(stderr, "NullPointerException\n");
        exit(1);
    }
    if (index < 0 || index >= arr->length) {
        fprintf(stderr, "ArrayIndexOutOfBoundsException: "
                        "Index %d out of bounds for length %d\n",
                        index, arr->length);
        exit(1);
    }
    arr->data[index] = value;
}

/* arr.clone() — Java arrays are cloneable */
IntArray *int_array_clone(const IntArray *arr) {
    IntArray *copy = int_array_new(arr->length);
    if (!copy) return NULL;
    memcpy(copy->data, arr->data, arr->length * sizeof(int));
    return copy;
}

/* GC: free when no more references */
void int_array_free(IntArray *arr) {
    if (!arr) return;
    free(arr->data);
    free(arr);
}

/* Print like Java's Arrays.toString() */
void int_array_print(const IntArray *arr) {
    if (!arr) { printf("null"); return; }
    printf("[");
    for (int i = 0; i < arr->length; i++) {
        printf("%d", arr->data[i]);
        if (i < arr->length - 1) printf(", ");
    }
    printf("]");
}

/* ============================================================
 * StringArray — models Java's String[]
 * Shows how Java handles arrays of objects
 * ============================================================ */
typedef struct {
    ObjectHeader  header;
    int           length;
    char        **data;  /* each element is a heap-allocated string */
} StringArray;

StringArray *str_array_new(int length) {
    StringArray *arr = malloc(sizeof(StringArray));
    if (!arr) return NULL;
    arr->header.class_name = "String[]";
    arr->header.ref_count  = 1;
    arr->length = length;
    arr->data   = calloc(length, sizeof(char *));  /* all null initially (like Java) */
    if (!arr->data) { free(arr); return NULL; }
    return arr;
}

void str_array_set(StringArray *arr, int index, const char *value) {
    if (!arr || index < 0 || index >= arr->length) {
        fprintf(stderr, "ArrayIndexOutOfBoundsException\n"); exit(1);
    }
    /* Free old value if set */
    if (arr->data[index]) free(arr->data[index]);
    if (!value) { arr->data[index] = NULL; return; }
    arr->data[index] = malloc(strlen(value) + 1);
    strcpy(arr->data[index], value);
}

const char *str_array_get(const StringArray *arr, int index) {
    if (!arr || index < 0 || index >= arr->length) {
        fprintf(stderr, "ArrayIndexOutOfBoundsException\n"); exit(1);
    }
    return arr->data[index];  /* may be NULL — like Java's null */
}

void str_array_free(StringArray *arr) {
    if (!arr) return;
    for (int i = 0; i < arr->length; i++)
        if (arr->data[i]) free(arr->data[i]);
    free(arr->data);
    free(arr);
}

void str_array_print(const StringArray *arr) {
    if (!arr) { printf("null"); return; }
    printf("[");
    for (int i = 0; i < arr->length; i++) {
        if (arr->data[i]) printf("\"%s\"", arr->data[i]);
        else               printf("null");
        if (i < arr->length - 1) printf(", ");
    }
    printf("]");
}

/* ============================================================
 * Demos
 * ============================================================ */

void demo_int_array(void) {
    printf("=== int[] — Bounds-Checked Integer Array ===\n");

    /* Java: int[] arr = new int[5]; */
    IntArray *arr = int_array_new(5);
    printf("  new int[5]: "); int_array_print(arr); printf("\n");
    printf("  arr.length = %d\n", arr->length);

    /* Java: arr[0]=10; arr[1]=20; ... */
    for (int i = 0; i < arr->length; i++)
        int_array_set(arr, i, (i + 1) * 10);

    printf("  After filling: "); int_array_print(arr); printf("\n");
    printf("  arr[2] = %d\n", int_array_get(arr, 2));

    /* Java: arr.clone() */
    IntArray *copy = int_array_clone(arr);
    int_array_set(copy, 0, 999);
    printf("  original: "); int_array_print(arr);  printf("\n");
    printf("  clone:    "); int_array_print(copy); printf("\n");

    /* Negative size */
    printf("\n  Trying new int[-1]:\n  ");
    IntArray *bad = int_array_new(-1);  /* prints NegativeArraySizeException */
    (void)bad;

    int_array_free(arr);
    int_array_free(copy);
    printf("\n");
}

void demo_bounds_check(void) {
    printf("=== Bounds Check — Java's ArrayIndexOutOfBoundsException ===\n");

    IntArray *arr = int_array_new(3);
    int_array_set(arr, 0, 100);
    int_array_set(arr, 1, 200);
    int_array_set(arr, 2, 300);
    printf("  arr = "); int_array_print(arr); printf("\n");
    printf("  arr[0] = %d  (valid)\n", int_array_get(arr, 0));
    printf("  arr[2] = %d  (valid, last element)\n", int_array_get(arr, 2));

    printf("  Trying arr[3] (one past end):\n  ");
    /* This will print the exception and exit */
    int_array_get(arr, 3);

    int_array_free(arr);
}

void demo_string_array(void) {
    printf("=== String[] — Array of Objects ===\n");

    /* Java: String[] names = new String[4]; */
    StringArray *names = str_array_new(4);
    printf("  new String[4]: "); str_array_print(names); printf("\n");
    printf("  (all null initially, like Java)\n");

    /* Java: names[0]="Alice"; names[1]="Bob"; ... */
    str_array_set(names, 0, "Alice");
    str_array_set(names, 1, "Bob");
    str_array_set(names, 2, "Charlie");
    /* names[3] stays null */

    printf("  After setting: "); str_array_print(names); printf("\n");
    printf("  names[3] = %s (null element)\n",
           str_array_get(names, 3) ? str_array_get(names, 3) : "null");

    str_array_free(names);
    printf("\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Era 01 — Safe Array (how Java's int[] works)       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demo_int_array();
    demo_string_array();

    /* This demo calls exit() — must be last */
    demo_bounds_check();

    return 0;
}
