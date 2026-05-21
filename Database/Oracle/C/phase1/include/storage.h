/*
 * storage.h - Key-Value Store Header
 * 
 * This header defines the API for a simple key-value store with file persistence.
 * 
 * Key Concepts:
 * - Hash table for fast in-memory lookup
 * - File I/O for persistence
 * - Serialization for binary storage
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef STORAGE_H
#define STORAGE_H

#include <stddef.h>  // For size_t
#include <stdint.h>  // For uint32_t

/*
 * ============================================================================
 * CONSTANTS AND CONFIGURATION
 * ============================================================================
 */

/* Maximum number of entries in the hash table */
#define MAX_ENTRIES 1000

/* Maximum length for keys and values */
#define MAX_KEY_LENGTH 256
#define MAX_VALUE_LENGTH 4096

/* Hash table size (should be prime for better distribution) */
#define HASH_TABLE_SIZE 997

/* File format version for compatibility checking */
#define STORAGE_VERSION 1

/*
 * ============================================================================
 * DATA STRUCTURES
 * ============================================================================
 */

/*
 * KeyValuePair - Represents a single key-value entry
 * 
 * This structure holds a key-value pair in memory. It's used both for
 * in-memory storage and for serialization to disk.
 * 
 * Fields:
 *   - key: The key string (null-terminated)
 *   - value: The value string (null-terminated)
 *   - next: Pointer to next entry (for collision handling in hash table)
 */
typedef struct KeyValuePair {
    char *key;                    // Dynamically allocated key
    char *value;                  // Dynamically allocated value
    struct KeyValuePair *next;    // For chaining in hash table
} KeyValuePair;

/*
 * Storage - Main storage structure
 * 
 * This structure represents the entire key-value store. It contains:
 * - A hash table for fast lookups
 * - Metadata about the store
 * - File information for persistence
 * 
 * Fields:
 *   - table: Hash table (array of pointers to KeyValuePair)
 *   - entry_count: Number of entries currently stored
 *   - filename: Path to the persistent storage file
 */
typedef struct {
    KeyValuePair **table;         // Hash table (array of linked lists)
    uint32_t entry_count;         // Number of entries
    char *filename;               // File path for persistence
} Storage;

/*
 * ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================
 */

/*
 * storage_create - Create a new key-value store
 * 
 * Allocates memory for a new storage structure and initializes it.
 * The hash table is allocated but empty.
 * 
 * Parameters:
 *   filename: Path to file for persistence (can be NULL)
 * 
 * Returns:
 *   Pointer to new Storage structure, or NULL on error
 * 
 * Example:
 *   Storage *store = storage_create("data.db");
 *   if (!store) {
 *       fprintf(stderr, "Failed to create storage\n");
 *       return 1;
 *   }
 */
Storage* storage_create(const char *filename);

/*
 * storage_destroy - Free all memory used by the storage
 * 
 * Frees all entries, the hash table, and the storage structure itself.
 * This should be called when done with the storage to prevent memory leaks.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   storage_destroy(store);
 *   store = NULL;  // Good practice
 */
void storage_destroy(Storage *store);

/*
 * storage_put - Store a key-value pair
 * 
 * Stores a key-value pair in the hash table. If the key already exists,
 * the value is updated. Memory is allocated for both key and value.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   key: The key string (will be copied)
 *   value: The value string (will be copied)
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (storage_put(store, "user:1", "Alice") != 0) {
 *       fprintf(stderr, "Failed to put key-value pair\n");
 *   }
 */
int storage_put(Storage *store, const char *key, const char *value);

/*
 * storage_get - Retrieve a value by key
 * 
 * Looks up a key in the hash table and returns the associated value.
 * The returned pointer points to the internal storage and should not
 * be modified or freed by the caller.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   key: The key to look up
 * 
 * Returns:
 *   Pointer to value string, or NULL if key not found
 * 
 * Example:
 *   const char *value = storage_get(store, "user:1");
 *   if (value) {
 *       printf("Found: %s\n", value);
 *   } else {
 *       printf("Key not found\n");
 *   }
 */
const char* storage_get(Storage *store, const char *key);

/*
 * storage_delete - Remove a key-value pair
 * 
 * Removes a key-value pair from the hash table and frees its memory.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   key: The key to delete
 * 
 * Returns:
 *   0 on success, -1 if key not found
 * 
 * Example:
 *   if (storage_delete(store, "user:1") != 0) {
 *       fprintf(stderr, "Key not found\n");
 *   }
 */
int storage_delete(Storage *store, const char *key);

/*
 * storage_exists - Check if a key exists
 * 
 * Checks whether a key exists in the storage without retrieving the value.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   key: The key to check
 * 
 * Returns:
 *   1 if key exists, 0 if not
 * 
 * Example:
 *   if (storage_exists(store, "user:1")) {
 *       printf("Key exists\n");
 *   }
 */
int storage_exists(Storage *store, const char *key);

/*
 * storage_list - List all key-value pairs
 * 
 * Iterates through all entries in the storage and calls a callback function
 * for each entry. Useful for debugging and iteration.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   callback: Function to call for each entry (key, value, user_data)
 *   user_data: User-provided data passed to callback
 * 
 * Returns:
 *   Number of entries listed
 * 
 * Example:
 *   void print_entry(const char *key, const char *value, void *data) {
 *       printf("%s => %s\n", key, value);
 *   }
 *   storage_list(store, print_entry, NULL);
 */
int storage_list(Storage *store, 
                 void (*callback)(const char *key, const char *value, void *data),
                 void *user_data);

/*
 * storage_count - Get number of entries
 * 
 * Returns the current number of key-value pairs in the storage.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 * 
 * Returns:
 *   Number of entries
 * 
 * Example:
 *   printf("Total entries: %u\n", storage_count(store));
 */
uint32_t storage_count(Storage *store);

/*
 * storage_clear - Remove all entries
 * 
 * Removes all key-value pairs from the storage and frees their memory.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   storage_clear(store);
 */
void storage_clear(Storage *store);

/*
 * storage_save - Save storage to file
 * 
 * Serializes all key-value pairs to a binary file. The file format is:
 * - Version (4 bytes)
 * - Entry count (4 bytes)
 * - For each entry:
 *   - Key length (4 bytes)
 *   - Key data (variable)
 *   - Value length (4 bytes)
 *   - Value data (variable)
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (storage_save(store) != 0) {
 *       fprintf(stderr, "Failed to save storage\n");
 *   }
 */
int storage_save(Storage *store);

/*
 * storage_load - Load storage from file
 * 
 * Deserializes key-value pairs from a binary file. Clears existing entries
 * before loading. The file format is checked for compatibility.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (storage_load(store) != 0) {
 *       fprintf(stderr, "Failed to load storage\n");
 *   }
 */
int storage_load(Storage *store);

/*
 * storage_get_stats - Get storage statistics
 * 
 * Returns statistics about the storage for debugging and optimization.
 * 
 * Parameters:
 *   store: Pointer to Storage structure
 *   total_entries: Output parameter for total entries
 *   empty_buckets: Output parameter for empty hash buckets
 *   max_chain_length: Output parameter for longest collision chain
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   uint32_t total, empty, max_chain;
 *   storage_get_stats(store, &total, &empty, &max_chain);
 *   printf("Entries: %u, Empty buckets: %u, Max chain: %u\n",
 *          total, empty, max_chain);
 */
int storage_get_stats(Storage *store, 
                      uint32_t *total_entries,
                      uint32_t *empty_buckets,
                      uint32_t *max_chain_length);

#endif // STORAGE_H
