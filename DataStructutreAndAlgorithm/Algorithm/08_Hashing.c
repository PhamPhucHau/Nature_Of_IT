/*
================================================================================
HASHING ALGORITHM
================================================================================

STORY:
------
A postal worker has 10 million addresses.

Finding an address by searching from A to Z would take forever.

So instead, the postal code is used as a formula.

The postal code directly tells you which district and building.

No searching needed.

Calculation is faster than searching.

That's Hashing: Turn a search problem into a calculation problem.

NETWORK APPLICATION:
- DNS lookups (domain → IP address)
- ARP table (IP → MAC address)
- Routing table lookups
- Load balancing (client → server)
- Hash-based load balancing (consistent hashing)
- Bloom filters (packet filtering)
- Hash tables for caching

COMPLEXITY:
- Time: O(1) average case, O(n) worst case (collisions)
- Space: O(n)

KEY INSIGHT:
Hashing converts search into computation.
The hash function is the key - it must distribute evenly and be fast.
Collisions must be handled efficiently.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 100
#define MAX_ENTRIES 1000
#define MAX_KEY_LEN 50
#define MAX_VALUE_LEN 100

// Hash table entry
typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    int used;
} HashEntry;

// Hash table with separate chaining
typedef struct {
    HashEntry* entries;
    int size;
    int collision_count;
    int lookup_count;
    int hash_calls;
} HashTable;

/*
HASH FUNCTIONS
*/

// Simple modulo hash
int hash_simple(const char* key, int size) {
    int hash = 0;
    for (int i = 0; key[i]; i++) {
        hash += key[i];
    }
    return hash % size;
}

// djb2 hash (better distribution)
int hash_djb2(const char* key, int size) {
    unsigned long hash = 5381;
    for (int i = 0; key[i]; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)key[i];
    }
    return hash % size;
}

// Multiplicative hash (Knuth's hash)
int hash_knuth(const char* key, int size) {
    unsigned long hash = 0;
    for (int i = 0; key[i]; i++) {
        hash = hash * 65599 + (unsigned char)key[i];
    }
    return hash % size;
}

HashTable* hash_table_create(int size) {
    HashTable* ht = malloc(sizeof(HashTable));
    ht->entries = malloc(size * sizeof(HashEntry));
    ht->size = size;
    ht->collision_count = 0;
    ht->lookup_count = 0;
    ht->hash_calls = 0;
    
    for (int i = 0; i < size; i++) {
        ht->entries[i].used = 0;
    }
    
    return ht;
}

/*
LINEAR PROBING: Handle collision by finding next empty slot
*/
void hash_insert_linear_probe(HashTable* ht, const char* key, const char* value) {
    int index = hash_simple(key, ht->size);
    ht->hash_calls++;
    
    int initial_index = index;
    int collision = 0;
    
    // Linear probing: find empty slot
    while (ht->entries[index].used) {
        collision = 1;
        index = (index + 1) % ht->size;
        
        if (index == initial_index) {
            printf("Hash table full!\n");
            return;
        }
    }
    
    if (collision) {
        ht->collision_count++;
    }
    
    strcpy(ht->entries[index].key, key);
    strcpy(ht->entries[index].value, value);
    ht->entries[index].used = 1;
}

char* hash_lookup_linear_probe(HashTable* ht, const char* key) {
    int index = hash_simple(key, ht->size);
    ht->hash_calls++;
    ht->lookup_count++;
    
    int initial_index = index;
    
    while (ht->entries[index].used) {
        if (strcmp(ht->entries[index].key, key) == 0) {
            return ht->entries[index].value;  // Found
        }
        index = (index + 1) % ht->size;
        
        if (index == initial_index) {
            break;  // Searched entire table
        }
    }
    
    return NULL;  // Not found
}

/*
CHAINING: Handle collision by storing multiple items in list
*/
typedef struct ChainNode {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    struct ChainNode* next;
} ChainNode;

typedef struct {
    ChainNode** chains;
    int size;
    int collision_count;
} ChainHashTable;

ChainHashTable* chain_hash_create(int size) {
    ChainHashTable* ht = malloc(sizeof(ChainHashTable));
    ht->chains = malloc(size * sizeof(ChainNode*));
    ht->size = size;
    ht->collision_count = 0;
    
    for (int i = 0; i < size; i++) {
        ht->chains[i] = NULL;
    }
    
    return ht;
}

void chain_hash_insert(ChainHashTable* ht, const char* key, const char* value) {
    int index = hash_djb2(key, ht->size);
    
    ChainNode* node = malloc(sizeof(ChainNode));
    strcpy(node->key, key);
    strcpy(node->value, value);
    
    if (ht->chains[index] != NULL) {
        ht->collision_count++;
    }
    
    node->next = ht->chains[index];
    ht->chains[index] = node;
}

char* chain_hash_lookup(ChainHashTable* ht, const char* key) {
    int index = hash_djb2(key, ht->size);
    
    ChainNode* current = ht->chains[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    return NULL;
}

void dns_cache_demo() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: DNS CACHE - Domain to IP lookup                  ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    ChainHashTable* dns_cache = chain_hash_create(50);
    
    printf("\nAdding DNS entries:\n");
    chain_hash_insert(dns_cache, "google.com", "142.250.80.46");
    chain_hash_insert(dns_cache, "facebook.com", "31.13.64.35");
    chain_hash_insert(dns_cache, "github.com", "140.82.121.4");
    chain_hash_insert(dns_cache, "twitter.com", "104.244.42.1");
    chain_hash_insert(dns_cache, "youtube.com", "142.251.32.1");
    
    printf("Added 5 domain entries\n");
    printf("Collisions encountered: %d\n", dns_cache->collision_count);
    
    printf("\nDNS lookup tests:\n");
    
    const char* lookups[] = {"google.com", "github.com", "unknown.com", "facebook.com"};
    for (int i = 0; i < 4; i++) {
        char* ip = chain_hash_lookup(dns_cache, lookups[i]);
        if (ip) {
            printf("  %s → %s ✓\n", lookups[i], ip);
        } else {
            printf("  %s → NOT FOUND ✗\n", lookups[i]);
        }
    }
    
    printf("\nNETWORK APPLICATION:\n");
    printf("- Typical DNS servers handle ~100M entries\n");
    printf("- Hashing provides O(1) lookup\n");
    printf("- Without hashing: O(log N) with B-tree\n");
    printf("- Hashing: ~100x faster for large datasets\n");
}

void arp_table_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: ARP TABLE - IP to MAC address lookup             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    ChainHashTable* arp_table = chain_hash_create(100);
    
    printf("\nAdding ARP entries (IP → MAC):\n");
    chain_hash_insert(arp_table, "192.168.1.1", "00:1A:2B:3C:4D:5E");
    chain_hash_insert(arp_table, "192.168.1.10", "00:11:22:33:44:55");
    chain_hash_insert(arp_table, "192.168.1.20", "00:AA:BB:CC:DD:EE");
    chain_hash_insert(arp_table, "192.168.1.100", "00:22:44:66:88:AA");
    chain_hash_insert(arp_table, "192.168.1.200", "00:FF:EE:DD:CC:BB");
    
    printf("Added 5 ARP entries\n");
    printf("Collisions: %d\n", arp_table->collision_count);
    
    printf("\nARP lookup tests:\n");
    
    const char* ips[] = {"192.168.1.1", "192.168.1.100", "192.168.1.50"};
    for (int i = 0; i < 3; i++) {
        char* mac = chain_hash_lookup(arp_table, ips[i]);
        if (mac) {
            printf("  %s → %s ✓\n", ips[i], mac);
        } else {
            printf("  %s → NOT FOUND ✗\n", ips[i]);
        }
    }
    
    printf("\nNETWORK APPLICATION:\n");
    printf("- ARP protocol maps IP to MAC address\n");
    printf("- Hash table stores local network mappings\n");
    printf("- Enables fast layer 2 forwarding\n");
}

void load_balancing_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 3: LOAD BALANCING with Hash-based routing           ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nSimple hash-based load balancing:\n");
    printf("Route requests based on hash of client IP\n\n");
    
    const char* client_ips[] = {
        "203.0.113.1", "203.0.113.2", "203.0.113.3",
        "203.0.113.4", "203.0.113.5", "203.0.113.6"
    };
    int num_servers = 3;
    int requests[3] = {0, 0, 0};
    
    printf("Processing requests:\n");
    for (int i = 0; i < 6; i++) {
        int server_index = hash_simple(client_ips[i], num_servers);
        requests[server_index]++;
        printf("  Client %s → Server %d\n", client_ips[i], server_index);
    }
    
    printf("\nLoad distribution:\n");
    for (int i = 0; i < num_servers; i++) {
        printf("  Server %d: %d requests\n", i, requests[i]);
    }
    
    printf("\nBENEFITS:\n");
    printf("- Consistent hashing: Same client always goes to same server\n");
    printf("- O(1) lookup for routing decision\n");
    printf("- No centralized hash table needed\n");
}

void collision_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COLLISION ANALYSIS - Different Hash Functions                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Test data
    const char* urls[] = {
        "google.com", "facebook.com", "github.com",
        "twitter.com", "youtube.com", "linkedin.com",
        "amazon.com", "reddit.com", "stackoverflow.com",
        "medium.com"
    };
    int num_urls = 10;
    int table_size = 13;
    
    printf("\nTesting 3 hash functions with %d entries and table size %d:\n\n",
           num_urls, table_size);
    
    // Test simple hash
    printf("1. Simple hash (sum % size):\n");
    int collisions1 = 0;
    int used1[13] = {0};
    for (int i = 0; i < num_urls; i++) {
        int idx = hash_simple(urls[i], table_size);
        if (used1[idx]) collisions1++;
        used1[idx] = 1;
        printf("   %s → index %d\n", urls[i], idx);
    }
    printf("   Collisions: %d\n\n", collisions1);
    
    // Test DJB2 hash
    printf("2. DJB2 hash (better distribution):\n");
    int collisions2 = 0;
    int used2[13] = {0};
    for (int i = 0; i < num_urls; i++) {
        int idx = hash_djb2(urls[i], table_size);
        if (used2[idx]) collisions2++;
        used2[idx] = 1;
        printf("   %s → index %d\n", urls[i], idx);
    }
    printf("   Collisions: %d\n\n", collisions2);
    
    printf("Key takeaway: Better hash functions reduce collisions\n");
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nLinear Probing:\n");
    printf("  Average case: O(1) lookup, insert\n");
    printf("  Worst case: O(n) if table is full\n");
    printf("  Problem: Clustering (collisions cause more collisions)\n\n");
    
    printf("Chaining:\n");
    printf("  Average case: O(1) lookup, insert\n");
    printf("  Worst case: O(n) if all hash to same bucket\n");
    printf("  Better: No clustering\n\n");
    
    printf("Hash Function Quality:\n");
    printf("  Load factor = entries / table_size\n");
    printf("  Optimal: Load factor < 0.75\n");
    printf("  Too high: More collisions\n");
    printf("  Too low: Wasted memory\n\n");
    
    printf("Real-world DNS:\n");
    printf("  Queries per second: ~1 billion\n");
    printf("  Without hashing: Impossible to handle\n");
    printf("  With hashing: ~microseconds per lookup\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ HASHING ALGORITHM - COMPREHENSIVE DEMONSTRATION               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nHashing: Convert search → computation\n");
    printf("Time complexity: O(1) average case\n");
    printf("Essential for: DNS, ARP, caching, load balancing\n");
    
    dns_cache_demo();
    arp_table_demo();
    load_balancing_demo();
    collision_analysis();
    complexity_analysis();
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ KEY PRINCIPLES                                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. HASH FUNCTION DESIGN:\n");
    printf("   - Fast to compute\n");
    printf("   - Uniform distribution\n");
    printf("   - Minimize collisions\n\n");
    
    printf("2. COLLISION HANDLING:\n");
    printf("   - Chaining: Linked lists per bucket\n");
    printf("   - Probing: Find next empty slot\n");
    printf("   - Double hashing: Use second hash function\n\n");
    
    printf("3. LOAD FACTOR:\n");
    printf("   - Monitor entries/table_size ratio\n");
    printf("   - Resize table when load > threshold\n");
    printf("   - Maintain O(1) average performance\n\n");
    
    printf("4. APPLICATIONS:\n");
    printf("   - DNS (domain → IP)\n");
    printf("   - ARP (IP → MAC)\n");
    printf("   - Routing tables\n");
    printf("   - Caches (memcached, Redis)\n");
    printf("   - Load balancing\n");
    
    return 0;
}
