/*
================================================================================
TRIE (PREFIX TREE) DATA STRUCTURE
================================================================================

STORY:
------
A postal worker has 10 million addresses.

Instead of organizing by country, then city, then street, then house...

They organize by first letter, then second letter, then third...

This is a prefix tree (Trie).

Each node represents a letter. Paths from root to node spell words.

NETWORK APPLICATION:
- IP address routing (CIDR prefix matching)
- DNS name resolution
- Auto-complete in search engines
- Longest prefix matching in packet forwarding
- Spell checking and dictionary lookup
- IP blocking/filtering rules
- MAC address lookup with prefixes

COMPLEXITY:
- Insert: O(m) where m = key length
- Search: O(m)
- Delete: O(m)
- Space: O(ALPHABET_SIZE * N * M)
- Prefix search: O(m + k) where k = results

KEY INSIGHT:
Trie is not a binary tree - each node has up to ALPHABET_SIZE children.
Perfect for prefix-based queries that hash tables cannot handle.
Essential for IP routing where we need longest prefix match.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ALPHABET_SIZE 256  // For general strings, IP addresses, etc.
#define MAX_WORD_LEN 50

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    char value[100];  // Store associated value
} TrieNode;

typedef struct {
    TrieNode* root;
    int nodeCount;
    int wordCount;
} Trie;

TrieNode* create_node() {
    TrieNode* node = malloc(sizeof(TrieNode));
    node->isEndOfWord = false;
    node->value[0] = '\0';
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    
    return node;
}

Trie* trie_create() {
    Trie* trie = malloc(sizeof(Trie));
    trie->root = create_node();
    trie->nodeCount = 1;
    trie->wordCount = 0;
    return trie;
}

/*
INSERT into Trie - O(m) where m is key length
*/
void trie_insert(Trie* trie, const char* key, const char* value) {
    TrieNode* node = trie->root;
    
    for (int i = 0; key[i] != '\0'; i++) {
        unsigned char c = (unsigned char)key[i];
        
        if (node->children[c] == NULL) {
            node->children[c] = create_node();
            trie->nodeCount++;
        }
        
        node = node->children[c];
    }
    
    if (!node->isEndOfWord) {
        trie->wordCount++;
    }
    
    node->isEndOfWord = true;
    strcpy(node->value, value);
}

/*
SEARCH in Trie - O(m)
*/
bool trie_search(Trie* trie, const char* key, char* result_value) {
    TrieNode* node = trie->root;
    
    for (int i = 0; key[i] != '\0'; i++) {
        unsigned char c = (unsigned char)key[i];
        
        if (node->children[c] == NULL) {
            return false;
        }
        
        node = node->children[c];
    }
    
    if (node->isEndOfWord && result_value) {
        strcpy(result_value, node->value);
    }
    
    return node->isEndOfWord;
}

/*
PREFIX SEARCH - Find all words with given prefix
Perfect for IP routing with CIDR notation
*/
int trie_prefix_search(TrieNode* node, char prefix[MAX_WORD_LEN], 
                       char results[100][MAX_WORD_LEN], int index) {
    if (node->isEndOfWord) {
        strcpy(results[index], prefix);
        index++;
    }
    
    for (int c = 0; c < ALPHABET_SIZE; c++) {
        if (node->children[c] != NULL) {
            int len = strlen(prefix);
            prefix[len] = c;
            prefix[len + 1] = '\0';
            
            index = trie_prefix_search(node->children[c], prefix, results, index);
            
            prefix[len] = '\0';
        }
    }
    
    return index;
}

/*
LONGEST PREFIX MATCH - Essential for IP routing
Example: Match 192.168.1.0/24 before 192.168.0.0/16
*/
bool trie_longest_prefix_match(Trie* trie, const char* key, 
                                char* best_match, char* best_value) {
    TrieNode* node = trie->root;
    char current_match[MAX_WORD_LEN];
    current_match[0] = '\0';
    bool found = false;
    
    for (int i = 0; key[i] != '\0'; i++) {
        unsigned char c = (unsigned char)key[i];
        
        if (node->children[c] == NULL) {
            break;  // No longer matching
        }
        
        node = node->children[c];
        current_match[i] = key[i];
        current_match[i + 1] = '\0';
        
        if (node->isEndOfWord) {
            strcpy(best_match, current_match);
            strcpy(best_value, node->value);
            found = true;
        }
    }
    
    return found;
}

void basic_trie_demo() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: BASIC TRIE - Dictionary/Search                   ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    Trie* trie = trie_create();
    
    printf("\nInserting words:\n");
    
    const char* words[] = {"apple", "application", "apply", "apricot", "banana"};
    const char* defs[] = {"fruit", "software", "use", "fruit", "yellow"};
    
    for (int i = 0; i < 5; i++) {
        trie_insert(trie, words[i], defs[i]);
        printf("  '%s' → %s\n", words[i], defs[i]);
    }
    
    printf("\nTrie statistics:\n");
    printf("  Total nodes: %d\n", trie->nodeCount);
    printf("  Total words: %d\n", trie->wordCount);
    
    printf("\nSearching:\n");
    
    const char* searches[] = {"apple", "app", "banana", "orange"};
    for (int i = 0; i < 4; i++) {
        char value[100];
        if (trie_search(trie, searches[i], value)) {
            printf("  '%s' found: %s ✓\n", searches[i], value);
        } else {
            printf("  '%s' not found ✗\n", searches[i]);
        }
    }
    
    printf("\nPrefix matches for 'app':\n");
    char results[100][MAX_WORD_LEN];
    TrieNode* node = trie->root;
    
    // Navigate to 'app' prefix
    for (int i = 0; i < 3; i++) {
        unsigned char c = (unsigned char)"app"[i];
        node = node->children[c];
    }
    
    int count = trie_prefix_search(node, "app", results, 0);
    for (int i = 0; i < count; i++) {
        printf("  %s\n", results[i]);
    }
}

void ip_routing_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: IP ROUTING TABLE (Longest Prefix Match)           ║\n");
    printf("║ (Real network application)                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    Trie* routing_table = trie_create();
    
    printf("\nBuilding routing table (CIDR blocks):\n");
    
    trie_insert(routing_table, "10", "Gateway A");
    trie_insert(routing_table, "10.0", "Gateway B");
    trie_insert(routing_table, "10.0.0", "Gateway C");
    trie_insert(routing_table, "192.168", "Gateway D");
    trie_insert(routing_table, "172.16", "Gateway E");
    
    printf("  10 → Gateway A\n");
    printf("  10.0 → Gateway B\n");
    printf("  10.0.0 → Gateway C\n");
    printf("  192.168 → Gateway D\n");
    printf("  172.16 → Gateway E\n");
    
    printf("\nLongest prefix matching (routing lookups):\n");
    
    const char* ip_addresses[] = {"10.0.0.1", "10.0.5.1", "192.168.1.1", "172.16.0.1"};
    
    for (int i = 0; i < 4; i++) {
        char best_match[MAX_WORD_LEN];
        char best_value[100];
        
        if (trie_longest_prefix_match(routing_table, ip_addresses[i], 
                                       best_match, best_value)) {
            printf("  IP %s matches %s → %s\n", ip_addresses[i], best_match, best_value);
        } else {
            printf("  IP %s no match\n", ip_addresses[i]);
        }
    }
    
    printf("\nLongest prefix match is critical for routing:\n");
    printf("- More specific routes take precedence\n");
    printf("- 10.0.0.0/24 is more specific than 10.0.0.0/16\n");
    printf("- Trie efficiently finds longest match\n");
}

void autocomplete_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 3: AUTO-COMPLETE (Prefix Completion)                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    Trie* dict = trie_create();
    
    printf("\nDictionary words:\n");
    
    const char* words[] = {
        "network", "networking", "networked",
        "internet", "intercom", "intermediate",
        "packet", "packing", "packaged"
    };
    
    for (int i = 0; i < 9; i++) {
        trie_insert(dict, words[i], "");
        printf("  %s\n", words[i]);
    }
    
    printf("\nAuto-complete suggestions:\n");
    
    const char* prefixes[] = {"net", "inter", "pack"};
    
    for (int p = 0; p < 3; p++) {
        printf("\nUser typed: '%s'\n", prefixes[p]);
        
        TrieNode* node = dict->root;
        for (int i = 0; prefixes[p][i] != '\0'; i++) {
            unsigned char c = (unsigned char)prefixes[p][i];
            if (node->children[c] == NULL) {
                printf("  No matches\n");
                goto next_prefix;
            }
            node = node->children[c];
        }
        
        {
            char results[100][MAX_WORD_LEN];
            int count = trie_prefix_search(node, (char*)prefixes[p], results, 0);
            printf("  Suggestions:");
            for (int i = 0; i < count; i++) {
                printf(" %s", results[i]);
            }
            printf("\n");
        }
        
        next_prefix:;
    }
}

void space_comparison() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ SPACE COMPLEXITY COMPARISON                                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nFor storing 1 million IP addresses:\n\n");
    
    printf("1. Array (sorted):\n");
    printf("   Space: 1M * 16 bytes = 16 MB\n");
    printf("   Lookup: O(log N) binary search\n");
    printf("   Prefix match: Requires scanning results\n\n");
    
    printf("2. Hash Table:\n");
    printf("   Space: 1M * 40 bytes = 40 MB\n");
    printf("   Lookup: O(1)\n");
    printf("   Prefix match: IMPOSSIBLE with hash table\n\n");
    
    printf("3. Trie:\n");
    printf("   Space: ~50-100 MB (depends on overlap)\n");
    printf("   Lookup: O(m) where m = IP length\n");
    printf("   Prefix match: O(m) with lazy traversal\n");
    printf("   Advantage: Longest prefix match built-in!\n");
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nTrie Operations (m = key length, k = result count):\n");
    printf("- Insert: O(m)\n");
    printf("- Search: O(m)\n");
    printf("- Delete: O(m)\n");
    printf("- Prefix search: O(m + k)\n");
    printf("- Longest prefix match: O(m)\n");
    printf("- Space: O(ALPHABET_SIZE * total_chars)\n\n");
    
    printf("vs Hash Table:\n");
    printf("- Insert: O(1)\n");
    printf("- Search: O(1)\n");
    printf("- Prefix search: IMPOSSIBLE\n");
    printf("- Longest prefix match: IMPOSSIBLE\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ TRIE (PREFIX TREE) - COMPREHENSIVE DEMONSTRATION              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nTrie: Tree optimized for prefix-based operations\n");
    printf("Key insight: Trade space for fast prefix matching\n");
    printf("Essential for: IP routing, auto-complete, spell checking\n");
    
    basic_trie_demo();
    ip_routing_demo();
    autocomplete_demo();
    space_comparison();
    complexity_analysis();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Trie: O(m) operations where m = key length\n");
    printf("2. Perfect for prefix queries (hash tables can't do this!)\n");
    printf("3. Longest prefix match: Essential for IP routing\n");
    printf("4. Trade space for functionality (prefix queries)\n");
    printf("5. Used in: DNS, IP routing, auto-complete\n");
    
    return 0;
}
