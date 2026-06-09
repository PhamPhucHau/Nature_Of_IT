/*
 * 03_HashMap.c  —  Java's HashMap<String,Integer> implemented in C
 *
 * Build:  gcc -Wall -Wextra -o 03_HashMap 03_HashMap.c
 * Run:    ./03_HashMap
 *
 * ─────────────────────────────────────────────────────────────────────
 * HOW JAVA HashMap WORKS INTERNALLY:
 *
 *   HashMap uses an array of "buckets" (Node[]).
 *   Each bucket is the head of a singly-linked chain (chaining collision).
 *   Java 8+ upgrades long chains (≥8) to a TreeMap (red-black tree).
 *
 *   DEFAULT_INITIAL_CAPACITY = 16   (must be power of 2)
 *   DEFAULT_LOAD_FACTOR      = 0.75
 *   TREEIFY_THRESHOLD        = 8
 *
 *   Hash: spread bits with h ^ (h >>> 16) to reduce collisions.
 *   Index: (n - 1) & hash   (fast modulo for power-of-2 sizes)
 *
 *   When size > capacity * loadFactor → RESIZE (double capacity, rehash all)
 *
 *   Java Source (simplified):
 *     static class Node<K,V> {
 *         final int    hash;
 *         final K      key;
 *         V            value;
 *         Node<K,V>    next;    // chaining
 *     }
 *     Node<K,V>[] table;        // the bucket array
 *     int         size;
 *     int         threshold;    // = capacity * loadFactor
 * ─────────────────────────────────────────────────────────────────────
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_INITIAL_CAPACITY 16
#define DEFAULT_LOAD_FACTOR      0.75f
#define TREEIFY_THRESHOLD        8      /* chain length before tree conversion */

/* ═══════════════════════════════════════════════════════════
 *  STRUCTURE — mirrors Java's HashMap.Node
 * ═══════════════════════════════════════════════════════════ */

typedef struct HashNode {
    unsigned int    hash;          /* cached hash of key */
    char           *key;           /* K key   (we use String keys) */
    int             value;         /* V value (we use int values)  */
    struct HashNode *next;         /* Node<K,V> next — chain link  */
} HashNode;

typedef struct {
    HashNode  **table;      /* Node<K,V>[] table  — bucket array  */
    int         size;       /* number of key-value mappings        */
    int         capacity;   /* table.length                        */
    int         threshold;  /* = capacity * loadFactor             */
    float       loadFactor;
} HashMap;

/* ═══════════════════════════════════════════════════════════
 *  HASH FUNCTION — mirrors Java's HashMap.hash()
 *  Java: h = key.hashCode(); return h ^ (h >>> 16);
 * ═══════════════════════════════════════════════════════════ */
static unsigned int java_string_hashcode(const char *key) {
    /* Java String.hashCode():
     *   s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
     */
    unsigned int h = 0;
    for (const char *p = key; *p; p++)
        h = 31 * h + (unsigned char)*p;
    return h;
}

static unsigned int java_hash(const char *key) {
    unsigned int h = java_string_hashcode(key);
    return h ^ (h >> 16);   /* spread high bits — same as OpenJDK */
}

/* Index in bucket array: (capacity-1) & hash */
static int bucket_index(unsigned int hash, int capacity) {
    return (int)((capacity - 1) & hash);
}

/* ═══════════════════════════════════════════════════════════
 *  LIFECYCLE
 * ═══════════════════════════════════════════════════════════ */

HashMap *hashmap_new(void) {
    HashMap *map      = malloc(sizeof(HashMap));
    map->capacity     = DEFAULT_INITIAL_CAPACITY;
    map->loadFactor   = DEFAULT_LOAD_FACTOR;
    map->threshold    = (int)(map->capacity * map->loadFactor); /* 12 */
    map->size         = 0;
    map->table        = calloc(map->capacity, sizeof(HashNode *));
    printf("[ALLOC] HashMap  capacity=%d  threshold=%d  loadFactor=%.2f\n",
           map->capacity, map->threshold, map->loadFactor);
    return map;
}

static void free_chain(HashNode *node) {
    while (node) {
        HashNode *next = node->next;
        free(node->key);
        free(node);
        node = next;
    }
}

void hashmap_free(HashMap *map) {
    for (int i = 0; i < map->capacity; i++)
        free_chain(map->table[i]);
    free(map->table);
    free(map);
    printf("[FREE]  HashMap destroyed\n");
}

/* ═══════════════════════════════════════════════════════════
 *  RESIZE  — double capacity, rehash all nodes
 *  Java: resize() doubles table, recalculates bucket positions
 * ═══════════════════════════════════════════════════════════ */
static void hashmap_resize(HashMap *map) {
    int old_cap = map->capacity;
    int new_cap = old_cap * 2;    /* always double — keeps power-of-2 */
    printf("[RESIZE] capacity: %d → %d  (size=%d exceeded threshold=%d)\n",
           old_cap, new_cap, map->size, map->threshold);

    HashNode **new_table = calloc(new_cap, sizeof(HashNode *));

    /* Rehash every existing node */
    int moved = 0;
    for (int i = 0; i < old_cap; i++) {
        HashNode *node = map->table[i];
        while (node) {
            HashNode *next = node->next;
            int new_idx = bucket_index(node->hash, new_cap);
            node->next = new_table[new_idx];   /* prepend to new chain */
            new_table[new_idx] = node;
            moved++;
            node = next;
        }
    }
    free(map->table);
    map->table     = new_table;
    map->capacity  = new_cap;
    map->threshold = (int)(new_cap * map->loadFactor);
    printf("[RESIZE] Moved %d entries  new_threshold=%d\n", moved, map->threshold);
}

/* ═══════════════════════════════════════════════════════════
 *  PUBLIC API
 * ═══════════════════════════════════════════════════════════ */

/* map.put(key, value) — returns old value or -1 */
int hashmap_put(HashMap *map, const char *key, int value) {
    unsigned int h   = java_hash(key);
    int          idx = bucket_index(h, map->capacity);

    /* Walk existing chain for this bucket */
    for (HashNode *n = map->table[idx]; n; n = n->next) {
        if (n->hash == h && strcmp(n->key, key) == 0) {
            int old = n->value;
            n->value = value;   /* UPDATE existing key */
            return old;
        }
    }

    /* Key not found — insert new node at chain head */
    HashNode *node = malloc(sizeof(HashNode));
    node->hash  = h;
    node->key   = malloc(strlen(key) + 1);
    strcpy(node->key, key);
    node->value = value;
    node->next  = map->table[idx];   /* prepend — O(1) */
    map->table[idx] = node;
    map->size++;

    /* Check if resize needed */
    if (map->size > map->threshold)
        hashmap_resize(map);

    return -1;  /* -1 means "no old value" (Java returns null) */
}

/* map.get(key) — returns value, or -1 if missing */
int hashmap_get(const HashMap *map, const char *key) {
    unsigned int h   = java_hash(key);
    int          idx = bucket_index(h, map->capacity);

    for (HashNode *n = map->table[idx]; n; n = n->next)
        if (n->hash == h && strcmp(n->key, key) == 0)
            return n->value;

    return -1;  /* null in Java */
}

/* map.containsKey(key) */
int hashmap_contains_key(const HashMap *map, const char *key) {
    return hashmap_get(map, key) != -1;
}

/* map.remove(key) */
int hashmap_remove(HashMap *map, const char *key) {
    unsigned int h   = java_hash(key);
    int          idx = bucket_index(h, map->capacity);

    HashNode *prev = NULL;
    HashNode *cur  = map->table[idx];
    while (cur) {
        if (cur->hash == h && strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else      map->table[idx] = cur->next;
            int value = cur->value;
            free(cur->key);
            free(cur);
            map->size--;
            return value;
        }
        prev = cur;
        cur  = cur->next;
    }
    return -1;  /* key not found */
}

/* map.size() */
int hashmap_size(const HashMap *map) { return map->size; }

/* map.isEmpty() */
int hashmap_is_empty(const HashMap *map) { return map->size == 0; }

/* map.getOrDefault(key, defaultValue) */
int hashmap_get_or_default(const HashMap *map, const char *key, int def) {
    int v = hashmap_get(map, key);
    return v == -1 ? def : v;
}

/* forEach — visits every key-value pair */
void hashmap_foreach(const HashMap *map,
                     void (*action)(const char *key, int value)) {
    for (int i = 0; i < map->capacity; i++)
        for (HashNode *n = map->table[i]; n; n = n->next)
            action(n->key, n->value);
}

/* ═══════════════════════════════════════════════════════════
 *  EDUCATIONAL: show internal bucket layout
 * ═══════════════════════════════════════════════════════════ */
void hashmap_debug(const HashMap *map) {
    printf("  HashMap internal layout:\n");
    printf("  capacity=%d  size=%d  threshold=%d  load=%.2f\n",
           map->capacity, map->size, map->threshold, map->loadFactor);
    printf("  ┌──────┬─────────────────────────────────────────┐\n");
    int used_buckets = 0;
    int max_chain = 0;
    for (int i = 0; i < map->capacity; i++) {
        if (!map->table[i]) continue;
        used_buckets++;
        int chain_len = 0;
        printf("  │ [%2d] │ ", i);
        for (HashNode *n = map->table[i]; n; n = n->next) {
            printf("{\"%s\":%d}", n->key, n->value);
            if (n->next) printf(" → ");
            chain_len++;
        }
        if (chain_len > max_chain) max_chain = chain_len;
        if (chain_len >= TREEIFY_THRESHOLD)
            printf("  ← [would convert to TreeMap!]");
        printf("\n");
    }
    printf("  └──────┴─────────────────────────────────────────┘\n");
    printf("  Used buckets: %d/%d  Max chain: %d\n",
           used_buckets, map->capacity, max_chain);
}

static void print_entry(const char *key, int value) {
    printf("  %s = %d\n", key, value);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  03_HashMap.c — HashMap<String,Integer> in C        ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");

    /* ── Demo 1: Basic put / get ── */
    printf("━━━ 1. put / get ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    HashMap *map = hashmap_new();
    hashmap_put(map, "apple",  5);
    hashmap_put(map, "banana", 3);
    hashmap_put(map, "cherry", 8);
    hashmap_put(map, "date",   2);
    printf("get(\"apple\")  = %d\n", hashmap_get(map, "apple"));
    printf("get(\"banana\") = %d\n", hashmap_get(map, "banana"));
    printf("get(\"grape\")  = %d  (not present → -1)\n",
           hashmap_get(map, "grape"));
    printf("\n");

    /* ── Demo 2: Update existing key ── */
    printf("━━━ 2. Update (put existing key) ━━━━━━━━━━━━━━━━━━━━━\n");
    int old = hashmap_put(map, "apple", 99);
    printf("Old value of 'apple': %d\n", old);
    printf("New value of 'apple': %d\n", hashmap_get(map, "apple"));
    printf("\n");

    /* ── Demo 3: Internal layout ── */
    printf("━━━ 3. Internal Bucket Layout ━━━━━━━━━━━━━━━━━━━━━━━━\n");
    hashmap_debug(map);
    printf("\n");

    /* ── Demo 4: containsKey / remove ── */
    printf("━━━ 4. containsKey / remove ━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("containsKey(\"cherry\") = %d\n", hashmap_contains_key(map, "cherry"));
    printf("containsKey(\"grape\")  = %d\n", hashmap_contains_key(map, "grape"));
    printf("remove(\"banana\") = %d\n", hashmap_remove(map, "banana"));
    printf("get(\"banana\") after remove = %d\n", hashmap_get(map, "banana"));
    printf("size() = %d\n", hashmap_size(map));
    printf("\n");

    /* ── Demo 5: getOrDefault ── */
    printf("━━━ 5. getOrDefault ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("getOrDefault(\"apple\", 0)  = %d\n",
           hashmap_get_or_default(map, "apple", 0));
    printf("getOrDefault(\"mango\", 0)  = %d  (default)\n",
           hashmap_get_or_default(map, "mango", 0));
    printf("\n");

    /* ── Demo 6: forEach ── */
    printf("━━━ 6. forEach (entrySet iteration) ━━━━━━━━━━━━━━━━━━\n");
    hashmap_foreach(map, print_entry);
    printf("\n");

    /* ── Demo 7: RESIZE — add many keys to trigger rehash ── */
    printf("━━━ 7. Resize — trigger when size > capacity * 0.75 ━━\n");
    HashMap *big = hashmap_new();   /* capacity=16, threshold=12 */
    const char *keys[] = {
        "a","b","c","d","e","f","g","h",
        "i","j","k","l","m","n","o","p"   /* 16 entries → forces resize at 13th */
    };
    for (int i = 0; i < 16; i++)
        hashmap_put(big, keys[i], i);
    printf("Final: size=%d capacity=%d\n", big->size, big->capacity);
    hashmap_free(big);
    printf("\n");

    /* ── Summary ── */
    printf("━━━ Complexity Summary ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("  put(k,v):       O(1) amortized  — hash + array access\n");
    printf("  get(k):         O(1) amortized  — hash + chain scan\n");
    printf("  remove(k):      O(1) amortized\n");
    printf("  containsKey(k): O(1) amortized\n");
    printf("  resize():       O(n)  — rehash all entries\n");
    printf("\n");
    printf("  Hash collision: handled by chaining (linked list per bucket)\n");
    printf("  Java 8 upgrade: chain ≥ 8 items → convert to TreeMap O(log n)\n");
    printf("  Load factor 0.75: tradeoff — low = less collision, more memory\n");

    hashmap_free(map);
    return 0;
}
