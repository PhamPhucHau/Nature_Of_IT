/*
================================================================================
BINARY SEARCH TREE (BST) DATA STRUCTURE
================================================================================

STORY:
------
A librarian has 1 million books to organize.

She can't use an array: searching would take forever.

She can't just store in order: inserting a new book would require moving all others.

So she uses a tree: Split into left (smaller) and right (larger).

Every search cuts the problem in half.

This is a Binary Search Tree.

NETWORK APPLICATION:
- IP routing table lookups
- DNS hierarchical name resolution
- Network range queries (CIDR blocks)
- Packet classification
- Hierarchical resource allocation
- B-Trees in databases and file systems

COMPLEXITY:
- Search: O(log n) average, O(n) worst (unbalanced)
- Insert: O(log n) average, O(n) worst
- Delete: O(log n) average, O(n) worst
- Space: O(n)

KEY INSIGHT:
BST maintains sorted order implicitly through structure.
Not as efficient as hash table for simple lookup,
but excellent for range queries and ordered traversal.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int key;
    char value[100];
    struct Node* left;
    struct Node* right;
    int height;  // For balancing
} Node;

typedef struct {
    Node* root;
    int nodes_created;
    int comparisons;
} BST;

Node* create_node(int key, const char* value) {
    Node* node = malloc(sizeof(Node));
    node->key = key;
    strcpy(node->value, value);
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    return node;
}

int get_height(Node* node) {
    if (!node) return 0;
    return node->height;
}

int get_balance(Node* node) {
    if (!node) return 0;
    return get_height(node->left) - get_height(node->right);
}

void update_height(Node* node) {
    if (!node) return;
    node->height = 1 + (get_height(node->left) > get_height(node->right) ?
                        get_height(node->left) : get_height(node->right));
}

/*
BST INSERT - O(log n) average
*/
Node* bst_insert(Node* node, int key, const char* value, BST* tree) {
    if (!node) {
        tree->nodes_created++;
        return create_node(key, value);
    }
    
    tree->comparisons++;
    
    if (key < node->key) {
        node->left = bst_insert(node->left, key, value, tree);
    } else if (key > node->key) {
        node->right = bst_insert(node->right, key, value, tree);
    } else {
        // Key already exists - update value
        strcpy(node->value, value);
        return node;
    }
    
    update_height(node);
    return node;
}

/*
BST SEARCH - O(log n) average
*/
Node* bst_search(Node* node, int key, BST* tree) {
    if (!node) {
        return NULL;
    }
    
    tree->comparisons++;
    
    if (key == node->key) {
        return node;
    } else if (key < node->key) {
        return bst_search(node->left, key, tree);
    } else {
        return bst_search(node->right, key, tree);
    }
}

/*
IN-ORDER TRAVERSAL: Print sorted keys
*/
void inorder_traversal(Node* node) {
    if (!node) return;
    
    inorder_traversal(node->left);
    printf("%d ", node->key);
    inorder_traversal(node->right);
}

/*
RANGE QUERY: Find all keys in range [min_key, max_key]
Perfect for networking!
*/
int range_query(Node* node, int min_key, int max_key, int* result) {
    if (!node) return 0;
    
    int count = 0;
    
    // Search in left subtree
    if (node->key > min_key) {
        count += range_query(node->left, min_key, max_key, result);
    }
    
    // Check current node
    if (node->key >= min_key && node->key <= max_key) {
        result[count++] = node->key;
    }
    
    // Search in right subtree
    if (node->key < max_key) {
        count += range_query(node->right, min_key, max_key, result + count);
    }
    
    return count;
}

void simple_bst_demo() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: BINARY SEARCH TREE BASICS                        ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    BST tree;
    tree.root = NULL;
    tree.nodes_created = 0;
    tree.comparisons = 0;
    
    printf("\nInserting values (50, 30, 70, 20, 40, 60, 80):\n");
    
    tree.root = bst_insert(tree.root, 50, "router-50", &tree);
    tree.root = bst_insert(tree.root, 30, "router-30", &tree);
    tree.root = bst_insert(tree.root, 70, "router-70", &tree);
    tree.root = bst_insert(tree.root, 20, "router-20", &tree);
    tree.root = bst_insert(tree.root, 40, "router-40", &tree);
    tree.root = bst_insert(tree.root, 60, "router-60", &tree);
    tree.root = bst_insert(tree.root, 80, "router-80", &tree);
    
    printf("Nodes created: %d\n", tree.nodes_created);
    
    printf("\nTree structure (in-order traversal - sorted):\n");
    inorder_traversal(tree.root);
    printf("\n");
    
    printf("\nSearching for values:\n");
    
    int search_keys[] = {40, 100, 60};
    for (int i = 0; i < 3; i++) {
        tree.comparisons = 0;
        Node* found = bst_search(tree.root, search_keys[i], &tree);
        if (found) {
            printf("  Found %d (%s) after %d comparisons\n",
                   search_keys[i], found->value, tree.comparisons);
        } else {
            printf("  Not found %d after %d comparisons\n",
                   search_keys[i], tree.comparisons);
        }
    }
}

void routing_table_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: IP ROUTING TABLE (Real network application)      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    BST routing_table;
    routing_table.root = NULL;
    routing_table.nodes_created = 0;
    routing_table.comparisons = 0;
    
    printf("\nBuilding routing table (CIDR blocks):\n");
    
    // Simulate IP subnets (simplified: just using numbers)
    routing_table.root = bst_insert(routing_table.root, 10, "10.0.0.0/8", &routing_table);
    routing_table.root = bst_insert(routing_table.root, 172, "172.16.0.0/12", &routing_table);
    routing_table.root = bst_insert(routing_table.root, 192, "192.168.0.0/16", &routing_table);
    routing_table.root = bst_insert(routing_table.root, 8, "8.8.8.8/32", &routing_table);
    routing_table.root = bst_insert(routing_table.root, 20, "20.0.0.0/8", &routing_table);
    
    printf("Routing entries in sorted order:\n");
    inorder_traversal(routing_table.root);
    printf("\n\n");
    
    printf("Routing table lookup:\n");
    int lookup_ips[] = {10, 50, 192};
    for (int i = 0; i < 3; i++) {
        routing_table.comparisons = 0;
        Node* route = bst_search(routing_table.root, lookup_ips[i], &routing_table);
        if (route) {
            printf("  IP %d → %s (found in %d steps)\n",
                   lookup_ips[i], route->value, routing_table.comparisons);
        } else {
            printf("  IP %d → NOT FOUND (checked in %d steps)\n",
                   lookup_ips[i], routing_table.comparisons);
        }
    }
}

void range_query_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 3: RANGE QUERIES (CIDR subnet matching)            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    BST tree;
    tree.root = NULL;
    tree.nodes_created = 0;
    tree.comparisons = 0;
    
    printf("\nBuilding tree with network prefixes:\n");
    
    // Simulate network prefixes
    int prefixes[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
    for (int i = 0; i < 9; i++) {
        tree.root = bst_insert(tree.root, prefixes[i], "", &tree);
    }
    
    printf("Prefixes in order: ");
    inorder_traversal(tree.root);
    printf("\n");
    
    printf("\nRange queries (find CIDR blocks in range):\n");
    
    int ranges[][2] = {
        {25, 75},  // Query: prefixes in [25, 75]
        {10, 50},  // Query: prefixes in [10, 50]
        {60, 100}  // Query: prefixes in [60, 100]
    };
    
    for (int i = 0; i < 3; i++) {
        int min = ranges[i][0];
        int max = ranges[i][1];
        
        int result[10];
        int count = range_query(tree.root, min, max, result);
        
        printf("  Range [%d, %d]: ", min, max);
        for (int j = 0; j < count; j++) {
            printf("%d ", result[j]);
        }
        printf("(%d results)\n", count);
    }
    
    printf("\nNETWORK APPLICATION:\n");
    printf("- BST perfect for range queries\n");
    printf("- Hash table cannot do range queries\n");
    printf("- Find all routes matching a prefix\n");
    printf("- Example: Find all IPs in 192.168.0.0/16\n");
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBinary Search Tree Performance:\n");
    printf("Average case (balanced):\n");
    printf("  - Search: O(log n)\n");
    printf("  - Insert: O(log n)\n");
    printf("  - Delete: O(log n)\n");
    printf("  - Range query: O(log n + k) where k = results\n\n");
    
    printf("Worst case (unbalanced/linear):\n");
    printf("  - Search: O(n)\n");
    printf("  - Insert: O(n)\n");
    printf("  - Delete: O(n)\n");
    printf("  - This happens when tree is skewed\n\n");
    
    printf("Comparison with other structures:\n");
    printf("Operation  | Hash Table | BST        | Array");
    printf("\n-----------|------------|------------|-------");
    printf("\nSearch     | O(1)       | O(log n)   | O(n)");
    printf("\nInsert     | O(1)       | O(log n)   | O(n)");
    printf("\nDelete     | O(1)       | O(log n)   | O(n)");
    printf("\nRange Q    | X (hard)   | O(log n+k) | O(n)");
    printf("\nOrdered    | X          | O(n)       | O(1)");
    printf("\n\nBST winner for: Ordered access + range queries!\n");
}

void balancing_explanation() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ BALANCING STRATEGIES                                           ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nUnbalanced BST problem:\n");
    printf("  Insert in order: 1, 2, 3, 4, 5\n");
    printf("  Creates linear chain: 1\n");
    printf("                          \\\n");
    printf("                           2\n");
    printf("                            \\\n");
    printf("                             3...\n");
    printf("  Performance: O(n) like linked list!\n\n");
    
    printf("Solutions:\n");
    printf("1. AVL Tree:\n");
    printf("   - Keep balance factor ∈ [-1, 1]\n");
    printf("   - Guaranteed O(log n) operations\n");
    printf("   - Rotations to rebalance\n\n");
    
    printf("2. Red-Black Tree:\n");
    printf("   - Simpler balancing rules\n");
    printf("   - Color nodes (red/black)\n");
    printf("   - Slightly faster than AVL\n\n");
    
    printf("3. B-Tree:\n");
    printf("   - Multi-way tree (not binary)\n");
    printf("   - Perfect for disk I/O\n");
    printf("   - Used in databases (InnoDB, PostgreSQL)\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ BINARY SEARCH TREE - COMPREHENSIVE DEMONSTRATION              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBST: Ordered tree where left < parent < right\n");
    printf("Key advantage: Range queries impossible in hash tables\n");
    printf("Trade-off: O(log n) instead of O(1) for single lookups\n");
    
    simple_bst_demo();
    routing_table_demo();
    range_query_demo();
    complexity_analysis();
    balancing_explanation();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. BST: O(log n) ops on average (balanced)\n");
    printf("2. Maintains sorted order implicitly\n");
    printf("3. Excellent for range queries\n");
    printf("4. Balancing required for guaranteed performance\n");
    printf("5. Perfect for IP routing and hierarchical data\n");
    
    return 0;
}
