#ifndef STORAGE_H
#define STORAGE_H

#include <stdint.h>

#define MAX_ENTRIES 1000
#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 1024
#define HASH_TABLE_SIZE 100
#define STORAGE_VERSION 1

typedef struct KeyValuePair {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    struct KeyValuePair *next;
} KeyValuePair;

typedef struct {
    KeyValuePair **table;
    uint32_t entry_count;
    char *filename;
} Storage;

/* Create / Destroy */
Storage *storage_create(const char *filename);
void storage_destroy(Storage *storage);

/* CRUD */
int storage_put(Storage *storage,
                const char *key,
                const char *value);

char *storage_get(Storage *storage,
                  const char *key);

int storage_exists(Storage *storage,
                   const char *key);

int storage_delete(Storage *storage,
                   const char *key);

/* Utilities */
uint32_t storage_count(Storage *storage);

void storage_clear(Storage *storage);

/* Persistence */
int storage_save(Storage *storage);

int storage_load(Storage *storage);

#endif