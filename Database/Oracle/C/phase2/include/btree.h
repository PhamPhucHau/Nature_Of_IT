/*
 * btree.h - B+ Tree Header
 * 
 * Defines the API for a B+ tree index structure.
 * B+ trees are self-balancing trees optimized for disk I/O.
 * 
 * Key Properties:
 * - All leaf nodes at same depth (balanced)
 * - Internal nodes contain only keys
 * - Leaf nodes contain key-value pairs
 * - Efficient range queries
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * CONSTANTS AND CONFIGURATION
 * ============================================================================ */

/* B+ tree order (max children per node) */
#define BTREE_ORDER 4

/* Max keys per node = BTREE_ORDER - 1 */
#define MAX_KEYS (BTREE_ORDER - 1)

/* Min keys per node (except root) */
#define MIN_KEYS (BTREE_ORDER / 2 - 1)

/* Max value length */
#define MAX_VALUE_LENGTH 256

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * BTreeEntry - Key-value pair in leaf node
 */
typedef struct {
    uint32_t key;
    char value[MAX_VALUE_LENGTH];
} BTreeEntry;

/*
 * BTreeNode - Node in B+ tree
 * 
 * Internal nodes: Contains keys and child pointers
 * Leaf nodes: Contains key-value pairs and sibling pointers
 */
typedef struct BTreeNode {
    uint32_t keys[MAX_KEYS];           /* Keys in this node */
    struct BTreeNode *children[BTREE_ORDER];  /* Child pointers (internal) */
    BTreeEntry *entries;               /* Entries (leaf nodes only) */
    struct BTreeNode *next;             /* Next sibling (leaf nodes) */
    struct BTreeNode *prev;             /* Previous sibling (leaf nodes) */
    uint32_t key_count;                 /* Number of keys in this node */
    int is_leaf;                        /* 1 if leaf node, 0 if internal */
} BTreeNode;

/*
 * BTree - B+ tree structure
 */
typedef struct {
    BTreeNode *root;                   /* Root node */
    uint32_t entry_count;              /* Total entries in tree */
    uint32_t height;                   /* Height of tree */
} BTree;

/*
 * BTreeIterator - For range queries
 */
typedef struct {
    BTreeNode *current_node;           /* Current leaf node */
    uint32_t current_index;            /* Index in current node */
    uint32_t min_key;                  /* Minimum key (inclusive) */
    uint32_t max_key;                  /* Maximum key (inclusive) */
} BTreeIterator;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * btree_create - Create a new B+ tree
 * 
 * Returns:
 *   Pointer to new BTree, or NULL on error
 */
BTree* btree_create(void);

/*
 * btree_destroy - Free all memory used by tree
 * 
 * Parameters:
 *   tree: Pointer to BTree
 */
void btree_destroy(BTree *tree);

/*
 * btree_insert - Insert a key-value pair
 * 
 * Parameters:
 *   tree: Pointer to BTree
 *   key: The key to insert
 *   value: The value to insert
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int btree_insert(BTree *tree, uint32_t key, const char *value);

/*
 * btree_search - Search for a key
 * 
 * Parameters:
 *   tree: Pointer to BTree
 *   key: The key to search for
 *   value: Output buffer for value
 * 
 * Returns:
 *   0 if found, -1 if not found
 */
int btree_search(BTree *tree, uint32_t key, char *value);

/*
 * btree_delete - Delete a key
 * 
 * Parameters:
 *   tree: Pointer to BTree
 *   key: The key to delete
 * 
 * Returns:
 *   0 on success, -1 if key not found
 */
int btree_delete(BTree *tree, uint32_t key);

/*
 * btree_range_search - Find all keys in range
 * 
 * Parameters:
 *   tree: Pointer to BTree
 *   min_key: Minimum key (inclusive)
 *   max_key: Maximum key (inclusive)
 *   callback: Function to call for each result
 *   user_data: User data passed to callback
 * 
 * Returns:
 *   Number of results found
 */
int btree_range_search(BTree *tree, uint32_t min_key, uint32_t max_key,
                       void (*callback)(uint32_t key, const char *value, void *data),
                       void *user_data);

/*
 * btree_count - Get number of entries
 * 
 * Parameters:
 *   tree: Pointer to BTree
 * 
 * Returns:
 *   Number of entries
 */
uint32_t btree_count(BTree *tree);

/*
 * btree_height - Get tree height
 * 
 * Parameters:
 *   tree: Pointer to BTree
 * 
 * Returns:
 *   Height of tree (0 for single node)
 */
uint32_t btree_height(BTree *tree);

/*
 * btree_print - Print tree structure (for debugging)
 * 
 * Parameters:
 *   tree: Pointer to BTree
 */
void btree_print(BTree *tree);

/*
 * btree_verify - Verify tree integrity
 * 
 * Parameters:
 *   tree: Pointer to BTree
 * 
 * Returns:
 *   1 if valid, 0 if invalid
 */
int btree_verify(BTree *tree);

/*
 * btree_get_stats - Get tree statistics
 * 
 * Parameters:
 *   tree: Pointer to BTree
 *   total_nodes: Output for total nodes
 *   leaf_nodes: Output for leaf nodes
 *   internal_nodes: Output for internal nodes
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int btree_get_stats(BTree *tree, uint32_t *total_nodes,
                    uint32_t *leaf_nodes, uint32_t *internal_nodes);

#endif // BTREE_H
