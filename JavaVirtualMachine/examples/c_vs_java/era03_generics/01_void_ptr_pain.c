/*
 * 01_void_ptr_pain.c — C void* containers: the pain Java Generics fixed
 *
 * Build: gcc -Wall -Wextra -o 01_void_ptr_pain 01_void_ptr_pain.c
 * Run:   ./01_void_ptr_pain
 *
 * Before Java 5 Generics, Java collections worked like C void* —
 * they accepted anything, and you had to cast on the way out.
 *
 * This file shows:
 *   1. C void* container — accepts anything (type-unsafe)
 *   2. The ClassCastException equivalent in C
 *   3. The type tag approach (C solution)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Part 1: Raw void* — the pain
 * Equivalent to Java's pre-generics "raw List"
 * ============================================================ */

#define RAWLIST_CAPACITY 10

typedef struct {
    void *items[RAWLIST_CAPACITY];  /* can hold ANYTHING */
    int   size;
} RawList;

void rawlist_add(RawList *list, void *item) {
    if (list->size >= RAWLIST_CAPACITY) {
        fprintf(stderr, "List full!\n");
        return;
    }
    list->items[list->size++] = item;
}

void *rawlist_get(RawList *list, int index) {
    if (index < 0 || index >= list->size) {
        fprintf(stderr, "IndexOutOfBounds: %d\n", index);
        return NULL;
    }
    return list->items[index];
}

void demonstrate_void_ptr_pain(void) {
    printf("=== Part 1: void* Container (C pain) ===\n");
    printf("    Java equivalent: pre-generics 'List list = new ArrayList();'\n\n");

    RawList list = { .size = 0 };

    /* Mix any types — no compiler warning */
    int    num    = 42;
    double pi     = 3.14;
    char   str[]  = "hello";

    rawlist_add(&list, &num);   /* int* */
    rawlist_add(&list, &pi);    /* double* */
    rawlist_add(&list, str);    /* char* */
    rawlist_add(&list, NULL);   /* null — valid! */

    printf("  List has %d items (mixed types!)\n", list.size);

    /* Reading back — must remember what type each slot holds */
    /* If you get it wrong → garbage data or crash */
    int    *n = (int    *)rawlist_get(&list, 0);  /* correct cast */
    double *d = (double *)rawlist_get(&list, 1);  /* correct cast */
    char   *s = (char   *)rawlist_get(&list, 2);  /* correct cast */

    printf("  [0] int    = %d\n", *n);
    printf("  [1] double = %.2f\n", *d);
    printf("  [2] string = \"%s\"\n", s);

    /* WRONG CAST — equivalent to Java's ClassCastException */
    printf("\n  WRONG CAST (like ClassCastException):\n");
    int *wrong = (int *)rawlist_get(&list, 1); /* [1] is double, not int! */
    printf("  Casting double* to int* → reads garbage: %d\n", *wrong);
    printf("  [In C: undefined behavior. In Java pre-5: ClassCastException at runtime]\n\n");
}

/* ============================================================
 * Part 2: Tagged Union — C's way to add type safety
 * This is the concept Java Generics compiles down to (with type erasure)
 * ============================================================ */

typedef enum {
    TYPE_INT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_OBJECT
} ValueType;

typedef struct {
    ValueType type;
    union {
        int    i;
        double d;
        char  *s;
        void  *obj;
    } value;
} TypedValue;

TypedValue make_int(int v)        { return (TypedValue){ TYPE_INT,    { .i   = v    } }; }
TypedValue make_double(double v)  { return (TypedValue){ TYPE_DOUBLE, { .d   = v    } }; }
TypedValue make_string(char *v)   { return (TypedValue){ TYPE_STRING, { .s   = v    } }; }

const char *type_name(ValueType t) {
    switch (t) {
        case TYPE_INT:    return "Integer";
        case TYPE_DOUBLE: return "Double";
        case TYPE_STRING: return "String";
        case TYPE_OBJECT: return "Object";
        default:          return "Unknown";
    }
}

/* "Safe cast" — throws if type doesn't match (like Java ClassCastException) */
int typed_get_int(TypedValue v) {
    if (v.type != TYPE_INT) {
        fprintf(stderr, "ClassCastException: %s cannot be cast to Integer\n",
                type_name(v.type));
        exit(1);
    }
    return v.value.i;
}

double typed_get_double(TypedValue v) {
    if (v.type != TYPE_DOUBLE) {
        fprintf(stderr, "ClassCastException: %s cannot be cast to Double\n",
                type_name(v.type));
        exit(1);
    }
    return v.value.d;
}

void demonstrate_tagged_union(void) {
    printf("=== Part 2: Tagged Union — adding runtime type safety ===\n");
    printf("    Java equivalent: Object type system (pre-generics)\n\n");

    TypedValue items[4];
    items[0] = make_int(42);
    items[1] = make_double(3.14);
    items[2] = make_string("hello");
    items[3] = make_int(100);

    for (int i = 0; i < 4; i++) {
        printf("  items[%d] = type=%s, ", i, type_name(items[i].type));
        switch (items[i].type) {
            case TYPE_INT:    printf("value=%d\n",   items[i].value.i); break;
            case TYPE_DOUBLE: printf("value=%.2f\n", items[i].value.d); break;
            case TYPE_STRING: printf("value=\"%s\"\n", items[i].value.s); break;
            default:          printf("value=?\n");
        }
    }

    printf("\n  Correct cast: items[0] as int = %d\n", typed_get_int(items[0]));
    printf("  Attempting wrong cast: items[1] (Double) as int...\n  ");
    typed_get_int(items[1]);  /* will print ClassCastException and exit */
}

/* ============================================================
 * Part 3: What Java Generics actually does (type erasure)
 * At compile time: List<Integer> → type-checked
 * At runtime:      List<Object>  → same as void* but with auto-casts
 * ============================================================ */
void explain_type_erasure(void) {
    printf("=== Part 3: Java Type Erasure Concept ===\n");
    printf("  Java:  List<Integer> list = new ArrayList<>();\n");
    printf("  After compile, bytecode sees: List list = new ArrayList();\n");
    printf("  But compiler INSERTS casts automatically and CHECKS types.\n");
    printf("\n");
    printf("  Source:    Integer x = list.get(0);\n");
    printf("  Bytecode:  Integer x = (Integer) list.get(0);  // auto-inserted\n");
    printf("\n");
    printf("  Compiler prevents you from adding wrong types:\n");
    printf("  list.add(\"wrong\"); → compile error (not runtime error!)\n");
    printf("\n");
    printf("  That's the key improvement: errors at COMPILE time, not RUNTIME.\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  Era 03 — void* Pain (what Java Generics solved)    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    demonstrate_void_ptr_pain();
    explain_type_erasure();
    demonstrate_tagged_union();  /* exits on ClassCastException demo */

    return 0;
}
