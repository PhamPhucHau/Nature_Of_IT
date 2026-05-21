/*
 * storage.c - Key-Value Store Implementation
 * 
 * This file implements a simple key-value store with file persistence.
 * It uses a hash table for fast in-memory lookups and binary files for
 * persistent storage.
 * 
 * Key Implementation Details:
 * - Hash table with separate chaining for collision handling
 * - Dynamic memory allocation for keys and values
 * - File I/O for persistence
 * - Error handling and validation
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "storage.h"
#include "serializer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/*
 * ============================================================================
 * DEBUG MACROS
 * ============================================================================
 */

/* Enable debug output by setting DEBUG to 1 */
#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

/* Error macro for consistent error reporting */
#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/*
 * ============================================================================
 * HASH FUNCTION
 * ============================================================================
 */

/*
 * hash_function - Simple hash function for strings
 * 
 * Uses the DJB2 algorithm (Dan Bernstein's hash function).
 * This is a simple but effective hash function for strings.
 * 
 * Algorithm:
 *   hash = 5381
 *   for each character c in string:
 *       hash = ((hash << 5) + hash) + c
 * 
 * Parameters:
 *   key: The string to hash
 * 
 * Returns:
 *   Hash value (0 to HASH_TABLE_SIZE-1)
 */
static uint32_t hash_function(const char *key) {
    uint32_t hash = 5381;
    int c;
    
    DEBUG_PRINT("Hashing key: %s\n", key);
    
    while ((c = *key++)) {
        /* hash = hash * 33 + c */
        hash = ((hash << 5) + hash) + c;
    }
    
    uint32_t index = hash % HASH_TABLE_SIZE;
    DEBUG_PRINT("Hash index: %u\n", index);
    
    return index;
}

/*
 * ============================================================================
 * STORAGE CREATION AND DESTRUCTION
 * ============================================================================
 */

Storage* storage_create(const char *filename) {
    DEBUG_PRINT("Creating storage with filename: %s\n", filename);
    
    /* Allocate memory for the storage structure */
    Storage *store = (Storage *)malloc(sizeof(Storage));
    if (!store) {
        ERROR_PRINT("Failed to allocate memory for Storage\n");
        return NULL;
    }
    
    /* Allocate memory for the hash table */
    store->table = (KeyValuePair **)calloc(HASH_TABLE_SIZE, sizeof(KeyValuePair *));
    if (!store->table) {
        ERROR_PRINT("Failed to allocate memory for hash table\n");
        free(store);
        return NULL;
    }
    
    /* Initialize fields */
    store->entry_count = 0;
    
    /* Copy filename if provided */
    if (filename) {
        store->filename = (char *)malloc(strlen(filename) + 1);
        if (!store->filename) {
            ERROR_PRINT("Failed to allocate memory for filename\n");
            free(store->table);
            free(store);
            return NULL;
        }
        strcpy(store->filename, filename);
    } else {
        store->filename = NULL;
    }
    
    DEBUG_PRINT("Storage created successfully\n");
    return store;
}

void storage_destroy(Storage *store) {
    if (!store) {
        DEBUG_PRINT("Storage is NULL, nothing to destroy\n");
        return;
    }
    
    DEBUG_PRINT("Destroying storage\n");
    
    /* Free all entries */
    storage_clear(store);
    
    /* Free the hash table */
    if (store->table) {
        free(store->table);
        store->table = NULL;
    }
    
    /* Free the filename */
    if (store->filename) {
        free(store->filename);
        store->filename = NULL;
    }
    
    /* Free the storage structure itself */
    free(store);
    
    DEBUG_PRINT("Storage destroyed successfully\n");
}

/*
 * ============================================================================
 * CORE OPERATIONS: PUT, GET, DELETE
 * ============================================================================
 */

int storage_put(Storage *store, const char *key, const char *value) {
    if (!store || !key || !value) {
        ERROR_PRINT("Invalid arguments: store=%p, key=%p, value=%p\n", 
                   store, key, value);
        return -1;
    }
    
    DEBUG_PRINT("Putting key='%s', value='%s'\n", key, value);
    
    /* Validate key and value lengths */
    if (strlen(key) >= MAX_KEY_LENGTH) {
        ERROR_PRINT("Key too long: %zu >= %d\n", strlen(key), MAX_KEY_LENGTH);
        return -1;
    }
    
    if (strlen(value) >= MAX_VALUE_LENGTH) {
        ERROR_PRINT("Value too long: %zu >= %d\n", strlen(value), MAX_VALUE_LENGTH);
        return -1;
    }
    
    /* Get hash index */
    uint32_t index = hash_function(key);
    
    /* Check if key already exists */
    KeyValuePair *current = store->table[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            /* Key exists, update the value */
            DEBUG_PRINT("Key exists, updating value\n");
            
            char *new_value = (char *)malloc(strlen(value) + 1);
            if (!new_value) {
                ERROR_PRINT("Failed to allocate memory for new value\n");
                return -1;
            }
            
            strcpy(new_value, value);
            free(current->value);
            current->value = new_value;
            
            DEBUG_PRINT("Value updated successfully\n");
            return 0;
        }
        current = current->next;
    }
    
    /* Key doesn't exist, create new entry */
    DEBUG_PRINT("Creating new entry\n");
    
    KeyValuePair *new_pair = (KeyValuePair *)malloc(sizeof(KeyValuePair));
    if (!new_pair) {
        ERROR_PRINT("Failed to allocate memory for KeyValuePair\n");
        return -1;
    }
    
    /* Allocate and copy key */
    new_pair->key = (char *)malloc(strlen(key) + 1);
    if (!new_pair->key) {
        ERROR_PRINT("Failed to allocate memory for key\n");
        free(new_pair);
        return -1;
    }
    strcpy(new_pair->key, key);
    
    /* Allocate and copy value */
    new_pair->value = (char *)malloc(strlen(value) + 1);
    if (!new_pair->value) {
        ERROR_PRINT("Failed to allocate memory for value\n");
        free(new_pair->key);
        free(new_pair);
        return -1;
    }
    strcpy(new_pair->value, value);
    
    /* Insert at the beginning of the chain (O(1) operation) */
    new_pair->next = store->table[index];
    store->table[index] = new_pair;
    store->entry_count++;
    
    DEBUG_PRINT("Entry created successfully, total entries: %u\n", store->entry_count);
    return 0;
}

const char* storage_get(Storage *store, const char *key) {
    if (!store || !key) {
        ERROR_PRINT("Invalid arguments: store=%p, key=%p\n", store, key);
        return NULL;
    }
    
    DEBUG_PRINT("Getting key='%s'\n", key);
    
    /* Get hash index */
    uint32_t index = hash_function(key);
    
    /* Search in the chain */
    KeyValuePair *current = store->table[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            DEBUG_PRINT("Key found, value='%s'\n", current->value);
            return current->value;
        }
        current = current->next;
    }
    
    DEBUG_PRINT("Key not found\n");
    return NULL;
}

int storage_delete(Storage *store, const char *key) {
    if (!store || !key) {
        ERROR_PRINT("Invalid arguments: store=%p, key=%p\n", store, key);
        return -1;
    }
    
    DEBUG_PRINT("Deleting key='%s'\n", key);
    
    /* Get hash index */
    uint32_t index = hash_function(key);
    
    /* Search and delete from the chain */
    KeyValuePair *current = store->table[index];
    KeyValuePair *prev = NULL;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            /* Found the key, remove it */
            if (prev) {
                prev->next = current->next;
            } else {
                store->table[index] = current->next;
            }
            
            /* Free memory */
            free(current->key);
            free(current->value);
            free(current);
            store->entry_count--;
            
            DEBUG_PRINT("Key deleted successfully, total entries: %u\n", store->entry_count);
            return 0;
        }
        
        prev = current;
        current = current->next;
    }
    
    DEBUG_PRINT("Key not found\n");
    return -1;
}

/*
 * ============================================================================
 * UTILITY OPERATIONS
 * ============================================================================
 */

int storage_exists(Storage *store, const char *key) {
    return storage_get(store, key) != NULL ? 1 : 0;
}

uint32_t storage_count(Storage *store) {
    if (!store) {
        ERROR_PRINT("Storage is NULL\n");
        return 0;
    }
    return store->entry_count;
}

int storage_list(Storage *store,
                 void (*callback)(const char *key, const char *value, void *data),
                 void *user_data) {
    if (!store || !callback) {
        ERROR_PRINT("Invalid arguments: store=%p, callback=%p\n", store, callback);
        return -1;
    }
    
    DEBUG_PRINT("Listing all entries\n");
    
    int count = 0;
    
    /* Iterate through all buckets */
    for (uint32_t i = 0; i < HASH_TABLE_SIZE; i++) {
        KeyValuePair *current = store->table[i];
        
        /* Iterate through the chain */
        while (current) {
            callback(current->key, current->value, user_data);
            count++;
            current = current->next;
        }
    }
    
    DEBUG_PRINT("Listed %d entries\n", count);
    return count;
}

void storage_clear(Storage *store) {
    if (!store) {
        ERROR_PRINT("Storage is NULL\n");
        return;
    }
    
    DEBUG_PRINT("Clearing storage\n");
    
    /* Iterate through all buckets */
    for (uint32_t i = 0; i < HASH_TABLE_SIZE; i++) {
        KeyValuePair *current = store->table[i];
        
        /* Free all entries in the chain */
        while (current) {
            KeyValuePair *next = current->next;
            free(current->key);
            free(current->value);
            free(current);
            current = next;
        }
        
        store->table[i] = NULL;
    }
    
    store->entry_count = 0;
    DEBUG_PRINT("Storage cleared\n");
}

/*
 * ============================================================================
 * PERSISTENCE OPERATIONS: SAVE AND LOAD
 * ============================================================================
 */

int storage_save(Storage *store) {
    if (!store || !store->filename) {
        ERROR_PRINT("Invalid storage or filename\n");
        return -1;
    }
    
    DEBUG_PRINT("Saving storage to file: %s\n", store->filename);
    
    /* Open file for writing */
    FILE *file = fopen(store->filename, "wb");
    if (!file) {
        ERROR_PRINT("Failed to open file for writing: %s\n", store->filename);
        return -1;
    }
    
    /* Write header */
    if (serializer_write_header(file) != 0) {
        ERROR_PRINT("Failed to write header\n");
        fclose(file);
        return -1;
    }
    
    /* Write entry count */
    if (serializer_write_uint32(file, store->entry_count) != 0) {
        ERROR_PRINT("Failed to write entry count\n");
        fclose(file);
        return -1;
    }
    
    DEBUG_PRINT("Writing %u entries\n", store->entry_count);
    
    /* Write all entries */
    for (uint32_t i = 0; i < HASH_TABLE_SIZE; i++) {
        KeyValuePair *current = store->table[i];
        
        while (current) {
            /* Write key */
            if (serializer_write_string(file, current->key) != 0) {
                ERROR_PRINT("Failed to write key\n");
                fclose(file);
                return -1;
            }
            
            /* Write value */
            if (serializer_write_string(file, current->value) != 0) {
                ERROR_PRINT("Failed to write value\n");
                fclose(file);
                return -1;
            }
            
            current = current->next;
        }
    }
    
    fclose(file);
    DEBUG_PRINT("Storage saved successfully\n");
    return 0;
}

int storage_load(Storage *store) {
    if (!store || !store->filename) {
        ERROR_PRINT("Invalid storage or filename\n");
        return -1;
    }
    
    DEBUG_PRINT("Loading storage from file: %s\n", store->filename);
    
    /* Check if file exists */
    if (!serializer_file_exists(store->filename)) {
        DEBUG_PRINT("File does not exist, creating new storage\n");
        return 0;
    }
    
    /* Open file for reading */
    FILE *file = fopen(store->filename, "rb");
    if (!file) {
        ERROR_PRINT("Failed to open file for reading: %s\n", store->filename);
        return -1;
    }
    
    /* Read and verify header */
    if (serializer_read_header(file) != 0) {
        ERROR_PRINT("Invalid file format\n");
        fclose(file);
        return -1;
    }
    
    /* Read entry count */
    uint32_t entry_count;
    if (serializer_read_uint32(file, &entry_count) != 0) {
        ERROR_PRINT("Failed to read entry count\n");
        fclose(file);
        return -1;
    }
    
    DEBUG_PRINT("Reading %u entries\n", entry_count);
    
    /* Clear existing entries */
    storage_clear(store);
    
    /* Read all entries */
    for (uint32_t i = 0; i < entry_count; i++) {
        char *key = NULL;
        char *value = NULL;
        
        /* Read key */
        if (serializer_read_string(file, &key) != 0) {
            ERROR_PRINT("Failed to read key\n");
            fclose(file);
            return -1;
        }
        
        /* Read value */
        if (serializer_read_string(file, &value) != 0) {
            ERROR_PRINT("Failed to read value\n");
            free(key);
            fclose(file);
            return -1;
        }
        
        /* Put the key-value pair */
        if (storage_put(store, key, value) != 0) {
            ERROR_PRINT("Failed to put key-value pair\n");
            free(key);
            free(value);
            fclose(file);
            return -1;
        }
        
        free(key);
        free(value);
    }
    
    fclose(file);
    DEBUG_PRINT("Storage loaded successfully\n");
    return 0;
}

/*
 * ============================================================================
 * STATISTICS
 * ============================================================================
 */

int storage_get_stats(Storage *store,
                      uint32_t *total_entries,
                      uint32_t *empty_buckets,
                      uint32_t *max_chain_length) {
    if (!store || !total_entries || !empty_buckets || !max_chain_length) {
        ERROR_PRINT("Invalid arguments\n");
        return -1;
    }
    
    DEBUG_PRINT("Calculating statistics\n");
    
    *total_entries = store->entry_count;
    *empty_buckets = 0;
    *max_chain_length = 0;
    
    /* Iterate through all buckets */
    for (uint32_t i = 0; i < HASH_TABLE_SIZE; i++) {
        if (store->table[i] == NULL) {
            (*empty_buckets)++;
        } else {
            /* Count chain length */
            uint32_t chain_length = 0;
            KeyValuePair *current = store->table[i];
            
            while (current) {
                chain_length++;
                current = current->next;
            }
            
            if (chain_length > *max_chain_length) {
                *max_chain_length = chain_length;
            }
        }
    }
    
    DEBUG_PRINT("Stats: entries=%u, empty=%u, max_chain=%u\n",
               *total_entries, *empty_buckets, *max_chain_length);
    
    return 0;
}
