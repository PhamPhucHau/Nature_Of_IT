/**
 * @file memory.h
 * @brief Memory management utilities
 * 
 * This module provides memory management utilities for the robot-like human OS,
 * including allocation tracking and debugging capabilities.
 * 
 * @author Robot OS Team
 * @date 2026
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Memory statistics
 * 
 * Tracks memory usage statistics.
 */
typedef struct {
    size_t total_allocated;     /**< Total bytes allocated */
    size_t total_freed;         /**< Total bytes freed */
    size_t current_usage;       /**< Current memory usage */
    size_t peak_usage;          /**< Peak memory usage */
    uint32_t allocation_count;  /**< Number of allocations */
    uint32_t free_count;        /**< Number of frees */
} MemoryStats;

/**
 * @brief Initialize memory management system
 * 
 * Sets up memory tracking and management.
 * 
 * @return true if initialization successful, false otherwise
 */
bool memory_init(void);

/**
 * @brief Allocate memory with tracking
 * 
 * Allocates memory and tracks the allocation.
 * 
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory, NULL if failed
 * 
 * @example
 * int *array = (int *)memory_alloc(sizeof(int) * 100);
 * if (array == NULL) {
 *     printf("Memory allocation failed\n");
 * }
 */
void* memory_alloc(size_t size);

/**
 * @brief Allocate zeroed memory
 * 
 * Allocates memory and initializes it to zero.
 * 
 * @param count Number of elements
 * @param size Size of each element
 * @return Pointer to allocated memory, NULL if failed
 */
void* memory_calloc(size_t count, size_t size);

/**
 * @brief Reallocate memory
 * 
 * Resizes previously allocated memory.
 * 
 * @param ptr Pointer to previously allocated memory
 * @param size New size in bytes
 * @return Pointer to reallocated memory, NULL if failed
 */
void* memory_realloc(void *ptr, size_t size);

/**
 * @brief Free allocated memory
 * 
 * Frees previously allocated memory and updates tracking.
 * 
 * @param ptr Pointer to memory to free
 */
void memory_free(void *ptr);

/**
 * @brief Get memory statistics
 * 
 * Retrieves current memory usage statistics.
 * 
 * @param stats Pointer to store statistics
 * @return true if successful, false otherwise
 */
bool memory_get_stats(MemoryStats *stats);

/**
 * @brief Print memory statistics
 * 
 * Prints formatted memory statistics to stdout.
 */
void memory_print_stats(void);

/**
 * @brief Reset memory statistics
 * 
 * Resets memory tracking counters.
 */
void memory_reset_stats(void);

/**
 * @brief Check memory integrity
 * 
 * Performs basic memory integrity checks.
 * 
 * @return true if memory is intact, false if corruption detected
 */
bool memory_check_integrity(void);

/**
 * @brief Shutdown memory management
 * 
 * Cleans up memory management system.
 * 
 * @return true if successful, false otherwise
 */
bool memory_shutdown(void);

#endif /* MEMORY_H */
