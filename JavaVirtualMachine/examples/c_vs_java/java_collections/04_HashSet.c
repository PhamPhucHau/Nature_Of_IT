/*
 * 04_HashSet.c  —  Java's HashSet<String> implemented in C
 *
 * Build:  gcc -Wall -Wextra -o 04_HashSet 04_HashSet.c
 * Run:    ./04_HashSet
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA HashSet WORKS INTERNALLY:
 *
 *   HashSet is literally just a wrapper around HashMap!
 *   It stores keys in the HashMap and uses a DUMMY constant as the value.
 *
 *   Java Source (OpenJDK):
 *     private transient HashMap<E, Object> map;
 *     private static final Object PRESENT = new Object();  // dummy value
 *
 *     public boolean add(E e)    { return map.put(e, PRESENT) == null; }
 *     public boolean remove(E e) { return map.remove(e) == PRESENT;    }
 *     public boolean contains(E e) { return map.containsKey(e);        }
 *     public int     size()        { return map.size();                 }
 *
 *   That's ALL of HashSet — it's a thin layer over HashMap.
 *
 *   Set properties (from HashMap's key uniqueness):
 *     - No duplicate elements
 *     - O(1) add, remove, contains
 *     - No guaranteed iteration order
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHSET_DEFAULT_CAPACITY 16
#define HASHSET_LOAD_FACTOR      0.75f
#define PRESENT                  1      /* dummy value — equivalent to Java's PRESENT */

/* ═══════════════════════════════════════════════════════════
 *  INTERNAL NODE — same as HashMap node but value is always PRESENT
 * ═══════════════════════════════════════════════════════════ */
typedef struct SetNode {
    unsigned int  hash;
    char         *key;
    struct SetNode *next;
} SetNode;

typedef struct {
    SetNode **table;
    int       size;
    int       capacity;
    int       threshold;
} HashSet;

/* ═══════════════════════════════════════════════════════════
 *  HASH  — same spread function as HashMap
 * ═══════════════════════════════════════════════════════════ */
static unsigned int hashset_hash(const char *key) {
    unsigned int h = 0;
    for (const char *p = key; *p; p++)
        h = 31 * h + (unsigned char)*p;
    return h ^ (h >> 16);
}
static int bucket_idx(unsigned int hash, int cap) {
    return (int)((cap - 1) & hash);
}

/* ═══════════════════════════════════════════════════════════
 *  LIFECYCLE
 * ═══════════════════════════════════════════════════════════ */
HashSet *hashset_new(void) {
    HashSet *set = malloc(sizeof(HashSet));
    set->capacity  = HASHSET_DEFAULT_CAPACITY;
    set->threshold = (int)(HASHSET_DEFAULT_CAPACITY * HASHSET_LOAD_FACTOR);
    set->size      = 0;
    set->table     = calloc(set->capacity, sizeof(SetNode *));
    printf("[ALLOC] HashSet  capacity=%d  threshold=%d\n",
           set->capacity, set->threshold);
    return set;
}

void hashset_free(HashSet *set) {
    for (int i = 0; i < set->capacity; i++) {
        SetNode *n = set->table[i];
        while (n) {
            SetNode *next = n->next;
            free(n->key);
            free(n);
            n = next;
        }
    }
    free(set->table);
    free(set);
    printf("[FREE]  HashSet destroyed\n");
}

static void hashset_resize(HashSet *set) {
    int old_cap = set->capacity;
    int new_cap = old_cap * 2;
    printf("[RESIZE] %d → %d\n", old_cap, new_cap);
    SetNode **new_table = calloc(new_cap, sizeof(SetNode *));
    for (int i = 0; i < old_cap; i++) {
        SetNode *n = set->table[i];
        while (n) {
            SetNode *next  = n->next;
            int new_idx    = bucket_idx(n->hash, new_cap);
            n->next        = new_table[new_idx];
            new_table[new_idx] = n;
            n = next;
        }
    }
    free(set->table);
    set->table     = new_table;
    set->capacity  = new_cap;
    set->threshold = (int)(new_cap * HASHSET_LOAD_FACTOR);
}

/* ═══════════════════════════════════════════════════════════
 *  PUBLIC API
 * ═══════════════════════════════════════════════════════════ */

/* set.add(element) — returns 1 if added, 0 if already present */
int hashset_add(HashSet *set, const char *key) {
    unsigned int h   = hashset_hash(key);
    int          idx = bucket_idx(h, set->capacity);

    /* Check for duplicate */
    for (SetNode *n = set->table[idx]; n; n = n->next)
        if (n->hash == h && strcmp(n->key, key) == 0)
            return 0;   /* already present — set ignores duplicates */

    /* Insert new node */
    SetNode *node = malloc(sizeof(SetNode));
    node->hash  = h;
    node->key   = malloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->next  = set->table[idx];
    set->table[idx] = node;
    set->size++;

    if (set->size > set->threshold) hashset_resize(set);
    return 1;
}

/* set.contains(element) */
int hashset_contains(const HashSet *set, const char *key) {
    unsigned int h   = hashset_hash(key);
    int          idx = bucket_idx(h, set->capacity);
    for (SetNode *n = set->table[idx]; n; n = n->next)
        if (n->hash == h && strcmp(n->key, key) == 0) return 1;
    return 0;
}

/* set.remove(element) */
int hashset_remove(HashSet *set, const char *key) {
    unsigned int h   = hashset_hash(key);
    int          idx = bucket_idx(h, set->capacity);
    SetNode *prev = NULL, *cur = set->table[idx];
    while (cur) {
        if (cur->hash == h && strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else      set->table[idx] = cur->next;
            free(cur->key); free(cur);
            set->size--;
            return 1;
        }
        prev = cur; cur = cur->next;
    }
    return 0;
}

int hashset_size(const HashSet *set)     { return set->size; }
int hashset_is_empty(const HashSet *set) { return set->size == 0; }

void hashset_foreach(const HashSet *set, void (*action)(const char *)) {
    for (int i = 0; i < set->capacity; i++)
        for (SetNode *n = set->table[i]; n; n = n->next)
            action(n->key);
}

void hashset_debug(const HashSet *set) {
    printf("  HashSet  capacity=%d  size=%d\n", set->capacity, set->size);
    for (int i = 0; i < set->capacity; i++) {
        if (!set->table[i]) continue;
        printf("  [%2d] ", i);
        for (SetNode *n = set->table[i]; n; n = n->next)
            printf("{%s} ", n->key);
        printf("\n");
    }
}

static void print_elem(const char *key) { printf("  %s\n", key); }

/* ── SET OPERATIONS ───────────────────────────────────────── */

/* union: a ∪ b — all elements from both sets */
HashSet *hashset_union(const HashSet *a, const HashSet *b) {
    HashSet *result = hashset_new();
    for (int i = 0; i < a->capacity; i++)
        for (SetNode *n = a->table[i]; n; n = n->next)
            hashset_add(result, n->key);
    for (int i = 0; i < b->capacity; i++)
        for (SetNode *n = b->table[i]; n; n = n->next)
            hashset_add(result, n->key);
    return result;
}

/* intersection: a ∩ b — elements in both sets */
HashSet *hashset_intersection(const HashSet *a, const HashSet *b) {
    HashSet *result = hashset_new();
    for (int i = 0; i < a->capacity; i++)
        for (SetNode *n = a->table[i]; n; n = n->next)
            if (hashset_contains(b, n->key))
                hashset_add(result, n->key);
    return result;
}

/* difference: a - b — elements in a but not b */
HashSet *hashset_difference(const HashSet *a, const HashSet *b) {
    HashSet *result = hashset_new();
    for (int i = 0; i < a->capacity; i++)
        for (SetNode *n = a->table[i]; n; n = n->next)
            if (!hashset_contains(b, n->key))
                hashset_add(result, n->key);
    return result;
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  04_HashSet.c — HashSet<String> in C                ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── Demo 1: Basic operations ── */
    printf("━━━ 1. add / contains / remove ━━━━━━━━━━━━━━━━━━━━━━━\n");
    HashSet *set = hashset_new();
    printf("add(\"apple\")  = %d\n", hashset_add(set, "apple"));
    printf("add(\"banana\") = %d\n", hashset_add(set, "banana"));
    printf("add(\"cherry\") = %d\n", hashset_add(set, "cherry"));
    printf("add(\"apple\")  = %d  ← duplicate ignored!\n",
           hashset_add(set, "apple"));
    printf("size() = %d  (not 4 — set has no duplicates)\n", hashset_size(set));
    printf("contains(\"banana\") = %d\n", hashset_contains(set, "banana"));
    printf("contains(\"grape\")  = %d\n", hashset_contains(set, "grape"));
    printf("remove(\"banana\") = %d\n", hashset_remove(set, "banana"));
    printf("contains(\"banana\") = %d\n", hashset_contains(set, "banana"));
    printf("\n");

    /* ── Demo 2: Internal structure ── */
    printf("━━━ 2. Internal Structure ━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    hashset_add(set, "date");
    hashset_add(set, "elderberry");
    hashset_debug(set);
    printf("\n");

    /* ── Demo 3: forEach ── */
    printf("━━━ 3. forEach (unordered) ━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    hashset_foreach(set, print_elem);
    printf("\n");

    /* ── Demo 4: Set operations ── */
    printf("━━━ 4. Set Operations (union, intersection, difference)\n");
    HashSet *a = hashset_new();
    HashSet *b = hashset_new();
    hashset_add(a, "apple"); hashset_add(a, "banana"); hashset_add(a, "cherry");
    hashset_add(b, "banana"); hashset_add(b, "cherry"); hashset_add(b, "date");

    printf("Set A: "); hashset_foreach(a, print_elem); printf("\n");
    printf("Set B: "); hashset_foreach(b, print_elem); printf("\n");

    HashSet *u = hashset_union(a, b);
    printf("A ∪ B (union, size=%d):\n", hashset_size(u));
    hashset_foreach(u, print_elem);
    printf("\n");

    HashSet *i = hashset_intersection(a, b);
    printf("A ∩ B (intersection, size=%d):\n", hashset_size(i));
    hashset_foreach(i, print_elem);
    printf("\n");

    HashSet *d = hashset_difference(a, b);
    printf("A - B (difference, size=%d):\n", hashset_size(d));
    hashset_foreach(d, print_elem);
    printf("\n");

    hashset_free(u); hashset_free(i); hashset_free(d);
    hashset_free(a); hashset_free(b);

    /* ── Summary ── */
    printf("━━━ Key Insight: HashSet IS a HashMap ━━━━━━━━━━━━━━━━\n");
    printf("  Java source:  private HashMap<E, Object> map;\n");
    printf("  add(e)     →  map.put(e, PRESENT)\n");
    printf("  contains(e)→  map.containsKey(e)\n");
    printf("  remove(e)  →  map.remove(e)\n");
    printf("  set.size() →  map.size()\n");

    hashset_free(set);
    return 0;
}
