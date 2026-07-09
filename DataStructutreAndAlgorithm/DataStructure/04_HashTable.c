#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 100

typedef struct {
    char key[50];
    char value[100];
    int used;
} HashEntry;

typedef struct {
    HashEntry *table;
} HashTable;

/* Simple hash function */
int hash(char *key) {
    int h = 0;
    for (int i = 0; key[i]; i++) {
        h = (h * 31 + key[i]) % HASH_SIZE;
    }
    return h < 0 ? -h : h;
}

HashTable* createHashTable() {
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    ht->table = (HashEntry *)calloc(HASH_SIZE, sizeof(HashEntry));
    return ht;
}

void insert(HashTable *ht, char *key, char *value) {
    int idx = hash(key);
    strcpy(ht->table[idx].key, key);
    strcpy(ht->table[idx].value, value);
    ht->table[idx].used = 1;
    printf("  + Insert: %s → %s (hash: %d)\n", key, value, idx);
}

char* lookup(HashTable *ht, char *key) {
    int idx = hash(key);
    if (ht->table[idx].used && strcmp(ht->table[idx].key, key) == 0) {
        return ht->table[idx].value;
    }
    return NULL;
}

int main() {
    printf("===== HASH TABLE =====\n\n");
    
    HashTable *ht = createHashTable();
    
    printf("--- Inserting DNS entries ---\n");
    insert(ht, "google.com", "142.250.185.46");
    insert(ht, "amazon.com", "54.239.28.30");
    insert(ht, "facebook.com", "31.13.64.1");
    printf("\n");
    
    printf("--- Lookup DNS entries ---\n");
    char *ip = lookup(ht, "google.com");
    if (ip) printf("google.com → %s\n", ip);
    
    ip = lookup(ht, "amazon.com");
    if (ip) printf("amazon.com → %s\n", ip);
    
    ip = lookup(ht, "nonexistent.com");
    printf("nonexistent.com → %s\n\n", ip ? ip : "NOT FOUND");
    
    printf("=== SUMMARY ===\n");
    printf("Time: O(1) average, O(n) worst\n");
    printf("Space: O(n)\n");
    printf("Network: DNS cache, ARP table, routing cache\n");
    
    free(ht->table);
    free(ht);
    return 0;
}
