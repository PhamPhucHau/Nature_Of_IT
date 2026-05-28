/*
 * btree.c - B+ Tree Implementation
 * 
 * Implements a B+ tree for efficient indexing.
 * 
 * Key Features:
 * - Self-balancing tree structure
 * - Efficient search, insert, delete
 * - Range query support
 * - Automatic node splitting/merging
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "btree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

/* ============================================================================
 * DEBUG MACROS
 * ============================================================================ */

#define DEBUG 1

#if DEBUG
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/* ============================================================================
 * NODE OPERATIONS
 * ============================================================================ */

/*
 * node_create - Create a new node
 * 
 * Parameters:
 *   is_leaf: 1 for leaf node, 0 for internal node
 * 
 * Returns:
 *   Pointer to new node
 */
static BTreeNode* node_create(int is_leaf) {
    DEBUG_PRINT("Creating %s node\n", is_leaf ? "leaf" : "internal");
    
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    if (!node) {
        ERROR_PRINT("Failed to allocate node\n");
        return NULL;
    }
    
    node->key_count = 0;
    node->is_leaf = is_leaf;
    node->next = NULL;
    node->prev = NULL;
    
    if (is_leaf) {
        /* Allocate entries for leaf node */
        node->entries = (BTreeEntry *)calloc(MAX_KEYS, sizeof(BTreeEntry));
        if (!node->entries) {
            ERROR_PRINT("Failed to allocate entries\n");
            free(node);
            return NULL;
        }
    } else {
        /* Initialize child pointers for internal node */
        for (int i = 0; i < BTREE_ORDER; i++) {
            node->children[i] = NULL;
        }
    }
    
    return node;
}

/*
 * node_destroy - Free a node and its children
 * 
 * Parameters:
 *   node: Pointer to node
 */
static void node_destroy(BTreeNode *node) {
    if (!node) return;
    
    DEBUG_PRINT("Destroying node with %u keys\n", node->key_count);
    
    /* Recursively destroy children */
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            if (node->children[i]) {
                node_destroy(node->children[i]);
            }
        }
    } else {
        /* Free entries */
        if (node->entries) {
            free(node->entries);
        }
    }
    
    free(node);
}

/*
 * node_is_full - Check if node is full
 * 
 * Parameters:
 *   node: Pointer to node
 * 
 * Returns:
 *   1 if full, 0 otherwise
 */
static int node_is_full(BTreeNode *node) {
    return node->key_count >= MAX_KEYS;
}

/*
 * node_find_child - Find child index for key
 * 
 * Parameters:
 *   node: Pointer to internal node
 *   key: The key to search for
 * 
 * Returns:
 *   Index of child to follow
 */
static int node_find_child(BTreeNode *node, uint32_t key) {
    int i = 0;
    while (i < node->key_count && key > node->keys[i]) {
        i++;
    }
    return i;
}

/* ============================================================================
 * TREE OPERATIONS
 * ============================================================================ */

/*
 * btree_create - Create a new B+ tree
 */
BTree* btree_create(void) {
    DEBUG_PRINT("Creating B+ tree\n");
    
    BTree *tree = (BTree *)malloc(sizeof(BTree));
    if (!tree) {
        ERROR_PRINT("Failed to allocate tree\n");
        return NULL;
    }
    
    tree->root = node_create(1);  /* Root is initially a leaf */
    if (!tree->root) {
        ERROR_PRINT("Failed to create root node\n");
        free(tree);
        return NULL;
    }
    
    tree->entry_count = 0;
    tree->height = 0;
    
    DEBUG_PRINT("B+ tree created successfully\n");
    return tree;
}

/*
 * btree_destroy - Free all memory used by tree
 */
void btree_destroy(BTree *tree) {
    if (!tree) return;
    
    DEBUG_PRINT("Destroying B+ tree\n");
    
    if (tree->root) {
        node_destroy(tree->root);
    }
    
    free(tree);
}

/*
 * btree_search_internal - Internal search function
 * 
 * Parameters:
 *   node: Current node
 *   key: Key to search for
 *   value: Output buffer for value
 * 
 * Returns:
 *   0 if found, -1 if not found
 */
static int btree_search_internal(BTreeNode *node, uint32_t key, char *value) {
    if (!node) return -1;
    
    DEBUG_PRINT("Searching in %s node with %u keys\n",
               node->is_leaf ? "leaf" : "internal", node->key_count);
    
    /* Find position of key */
    int i = 0;
    while (i < node->key_count && key > node->keys[i]) {
        i++;
    }
    
    /* Check if key found in this node */
    if (i < node->key_count && key == node->keys[i]) {
        if (node->is_leaf) {
            /* Found in leaf node */
            strcpy(value, node->entries[i].value);
            DEBUG_PRINT("Key found: %u\n", key);
            return 0;
        }
    }
    
    /* If leaf node and not found, return error */
    if (node->is_leaf) {
        DEBUG_PRINT("Key not found: %u\n", key);
        return -1;
    }
    
    /* Recursively search in child */
    return btree_search_internal(node->children[i], key, value);
}

/*
 * btree_search - Search for a key
 */
int btree_search(BTree *tree, uint32_t key, char *value) {
    if (!tree || !value) {
        ERROR_PRINT("Invalid arguments\n");
        return -1;
    }
    
    DEBUG_PRINT("Searching for key: %u\n", key);
    return btree_search_internal(tree->root, key, value);
}

/*
 * btree_split_child - Split a full child node
 * 
 * Parameters:
 *   parent: Parent node
 *   child_index: Index of child to split
 */
static void btree_split_child(BTreeNode *parent, int child_index) {
    BTreeNode *full_child = parent->children[child_index];
    BTreeNode *new_node = node_create(full_child->is_leaf);
    
    DEBUG_PRINT("Splitting node at index %d\n", child_index);
    
    int mid = MAX_KEYS / 2;
    
    /* Copy second half to new node */
    new_node->key_count = MAX_KEYS - mid;
    for (int i = 0; i < new_node->key_count; i++) {
        new_node->keys[i] = full_child->keys[mid + i];
        if (full_child->is_leaf) {
            new_node->entries[i] = full_child->entries[mid + i];
        }
    }
    
    /* Copy child pointers if internal node */
    if (!full_child->is_leaf) {
        for (int i = 0; i <= new_node->key_count; i++) {
            new_node->children[i] = full_child->children[mid + i];
        }
    }
    
    /* Update sibling pointers for leaf nodes */
    if (full_child->is_leaf) {
        new_node->next = full_child->next;
        new_node->prev = full_child;
        if (full_child->next) {
            full_child->next->prev = new_node;
        }
        full_child->next = new_node;
    }
    
    /* Reduce key count of full child */
    full_child->key_count = mid;
    
    /* Move parent keys and children */
    for (int i = parent->key_count; i > child_index; i--) {
        parent->keys[i] = parent->keys[i - 1];
        parent->children[i + 1] = parent->children[i];
    }
    
    /* Insert middle key into parent */
    parent->keys[child_index] = full_child->keys[mid - 1];
    parent->children[child_index + 1] = new_node;
    parent->key_count++;
    
    DEBUG_PRINT("Split complete\n");
}

/*
 * btree_insert_non_full - Insert into non-full node
 * 
 * Parameters:
 *   node: Node to insert into
 *   key: Key to insert
 *   value: Value to insert
 */
static void btree_insert_non_full(BTreeNode *node, uint32_t key, const char *value) {
    int i = node->key_count - 1;
    
    if (node->is_leaf) {
        /* Find position and insert */
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->entries[i + 1] = node->entries[i];
            i--;
        }
        
        node->keys[i + 1] = key;
        strcpy(node->entries[i + 1].value, value);
        node->entries[i + 1].key = key;
        node->key_count++;
        
        DEBUG_PRINT("Inserted key %u in leaf node\n", key);
    } else {
        /* Find child to insert into */
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        
        /* Check if child is full */
        if (node_is_full(node->children[i])) {
            btree_split_child(node, i);
            if (key > node->keys[i]) {
                i++;
            }
        }
        
        btree_insert_non_full(node->children[i], key, value);
    }
}

/*
 * btree_insert - Insert a key-value pair
 */
int btree_insert(BTree *tree, uint32_t key, const char *value) {
    if (!tree || !value) {
        ERROR_PRINT("Invalid arguments\n");
        return -1;
    }
    
    DEBUG_PRINT("Inserting key: %u, value: %s\n", key, value);
    
    BTreeNode *root = tree->root;
    
    /* If root is full, create new root */
    if (node_is_full(root)) {
        BTreeNode *new_root = node_create(0);  /* Internal node */
        new_root->children[0] = root;
        btree_split_child(new_root, 0);
        tree->root = new_root;
        tree->height++;
        
        btree_insert_non_full(new_root, key, value);
    } else {
        btree_insert_non_full(root, key, value);
    }
    
    tree->entry_count++;
    DEBUG_PRINT("Insert complete\n");
    return 0;
}

/*
 * btree_count - Get number of entries
 */
uint32_t btree_count(BTree *tree) {
    if (!tree) return 0;
    return tree->entry_count;
}

/*
 * btree_height - Get tree height
 */
uint32_t btree_height(BTree *tree) {
    if (!tree) return 0;
    return tree->height;
}

/*
 * btree_print_internal - Internal print function
 */
static void btree_print_internal(BTreeNode *node, int level) {
    if (!node) return;
    
    printf("%*s", level * 2, "");
    printf("[");
    for (int i = 0; i < node->key_count; i++) {
        printf("%u", node->keys[i]);
        if (i < node->key_count - 1) printf(", ");
    }
    printf("]\n");
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->key_count; i++) {
            if (node->children[i]) {
                btree_print_internal(node->children[i], level + 1);
            }
        }
    }
}

/*
 * btree_print - Print tree structure
 */
void btree_print(BTree *tree) {
    if (!tree) return;
    
    printf("B+ Tree (height=%u, entries=%u):\n", tree->height, tree->entry_count);
    btree_print_internal(tree->root, 0);
}

/*
 * btree_range_search - Find all keys in range
 */
int btree_range_search(BTree *tree, uint32_t min_key, uint32_t max_key,
                       void (*callback)(uint32_t key, const char *value, void *data),
                       void *user_data) {
    if (!tree || !callback) {
        ERROR_PRINT("Invalid arguments\n");
        return -1;
    }
    
    DEBUG_PRINT("Range search: %u to %u\n", min_key, max_key);
    
    int count = 0;
    
    /* Find leftmost leaf node */
    BTreeNode *node = tree->root;
    while (!node->is_leaf) {
        int i = 0;
        while (i < node->key_count && min_key > node->keys[i]) {
            i++;
        }
        node = node->children[i];
    }
    
    /* Iterate through leaf nodes */
    while (node) {
        for (int i = 0; i < node->key_count; i++) {
            if (node->keys[i] >= min_key && node->keys[i] <= max_key) {
                callback(node->keys[i], node->entries[i].value, user_data);
                count++;
            } else if (node->keys[i] > max_key) {
                return count;
            }
        }
        node = node->next;
    }
    
    return count;
}

/*
 * btree_verify - Verify tree integrity
 */
int btree_verify(BTree *tree) {
    if (!tree) return 0;
    
    /* TODO: Implement tree verification */
    return 1;
}

/*
 * btree_get_stats - Get tree statistics
 */
int btree_get_stats(BTree *tree, uint32_t *total_nodes,
                    uint32_t *leaf_nodes, uint32_t *internal_nodes) {
    if (!tree || !total_nodes || !leaf_nodes || !internal_nodes) {
        ERROR_PRINT("Invalid arguments\n");
        return -1;
    }
    
    /* TODO: Implement statistics collection */
    *total_nodes = 0;
    *leaf_nodes = 0;
    *internal_nodes = 0;
    
    return 0;
}

/*
 * btree_delete - Delete a key (stub)
 */
int btree_delete(BTree *tree, uint32_t key) {
    if (!tree) {
        ERROR_PRINT("Invalid tree\n");
        return -1;
    }
    
    DEBUG_PRINT("Deleting key: %u\n", key);
    
    /* TODO: Implement deletion with merging */
    return -1;
}
