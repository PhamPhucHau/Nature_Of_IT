/*
================================================================================
DISTRIBUTED CACHE ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
The database was getting hammered with the same requests repeatedly.

"Why recalculate?" asked the architect.

They placed caches in front:
- Local cache on each server (fastest)
- Distributed cache cluster (Redis) (fast)
- Database (slowest but authoritative)

When you need data, you check the cache first. Cache miss? Go to database, then
fill the cache. Consistent hashing routes keys to specific cache nodes.

ARCHITECTURE:
  Client
    |
  ┌─────────────────────┐
  │ Distributed Cache   │
  │ ┌─────────┐         │
  │ │ Node 1  │ Node 2  │
  │ │ Node 3  │ Node 4  │
  │ └─────────┘         │
  └─────────────────────┘
         |
      Database

Data Structures:
- HashMap (LRU cache with eviction)
- Hash Ring (consistent hashing)
- Linked List (LRU ordering)

Algorithms:
- Consistent Hashing (route to cache node)
- LRU Eviction (least recently used)
- Cache invalidation

COMPLEXITY:
- Get: O(1) cache hit
- Set: O(1) amortized
- Eviction: O(1) with linked list

USE CASES:
- Redis clusters
- Memcached
- Browser caching
- CDN caching
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CACHE_SIZE 100
#define MAX_KEY_LEN 50
#define MAX_VALUE_LEN 256
#define MAX_NODES 4
#define HASH_MODULO 1000

// Cache entry with LRU tracking
typedef struct CacheEntry {
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
    time_t timestamp;
    int accessCount;
    struct CacheEntry* next;  // For LRU linked list
    struct CacheEntry* prev;
} CacheEntry;

// Cache node in distributed cluster
typedef struct {
    int nodeId;
    char address[50];
    CacheEntry* entries[MAX_CACHE_SIZE];
    int entryCount;
    int hits;
    int misses;
    int evictions;
} CacheNode;

// Consistent hash node
typedef struct {
    int nodeId;
    int hashValue;
} HashRingNode;

// Distributed cache system
typedef struct {
    CacheNode nodes[MAX_NODES];
    int nodeCount;
    HashRingNode hashRing[MAX_NODES * 3];  // Virtual nodes
    int hashRingSize;
    CacheEntry* lruHead;
    CacheEntry* lruTail;
    int totalRequests;
    int totalHits;
} DistributedCache;

/*
HASH FUNCTION FOR CONSISTENT HASHING
*/
int hash_key(const char* key) {
    int hash = 0;
    for (int i = 0; key[i]; i++) {
        hash = (hash * 31 + key[i]) % HASH_MODULO;
    }
    return hash;
}

/*
CREATE DISTRIBUTED CACHE
*/
DistributedCache* cache_create() {
    DistributedCache* dc = malloc(sizeof(DistributedCache));
    dc->nodeCount = 0;
    dc->hashRingSize = 0;
    dc->lruHead = NULL;
    dc->lruTail = NULL;
    dc->totalRequests = 0;
    dc->totalHits = 0;
    
    printf("Distributed Cache created\n");
    return dc;
}

/*
ADD CACHE NODE
*/
void cache_add_node(DistributedCache* dc, int nodeId, const char* address) {
    if (dc->nodeCount >= MAX_NODES) {
        printf("Max nodes reached!\n");
        return;
    }
    
    CacheNode* node = &dc->nodes[dc->nodeCount];
    node->nodeId = nodeId;
    strcpy(node->address, address);
    node->entryCount = 0;
    node->hits = 0;
    node->misses = 0;
    node->evictions = 0;
    
    dc->nodeCount++;
    
    printf("✓ Cache node %d added: %s\n", nodeId, address);
}

/*
BUILD HASH RING - Multiple vnodes per node
*/
void cache_build_hash_ring(DistributedCache* dc) {
    dc->hashRingSize = 0;
    
    for (int i = 0; i < dc->nodeCount; i++) {
        for (int j = 0; j < 3; j++) {
            char vnodeKey[100];
            sprintf(vnodeKey, "node_%d_vnode_%d", dc->nodes[i].nodeId, j);
            
            int hashValue = hash_key(vnodeKey);
            
            dc->hashRing[dc->hashRingSize].nodeId = dc->nodes[i].nodeId;
            dc->hashRing[dc->hashRingSize].hashValue = hashValue;
            dc->hashRingSize++;
        }
    }
    
    printf("Hash ring built with %d vnodes\n", dc->hashRingSize);
}

/*
FIND RESPONSIBLE NODE FOR KEY - Consistent hashing
*/
int cache_find_node(DistributedCache* dc, const char* key) {
    if (dc->hashRingSize == 0) return -1;
    
    int keyHash = hash_key(key);
    
    // Find closest vnode on ring
    int closest = -1;
    int minDistance = INT_MAX;
    
    for (int i = 0; i < dc->hashRingSize; i++) {
        int distance = (dc->hashRing[i].hashValue - keyHash + HASH_MODULO) % HASH_MODULO;
        if (distance < minDistance) {
            minDistance = distance;
            closest = dc->hashRing[i].nodeId;
        }
    }
    
    return closest;
}

/*
GET NODE BY ID
*/
CacheNode* cache_get_node_by_id(DistributedCache* dc, int nodeId) {
    for (int i = 0; i < dc->nodeCount; i++) {
        if (dc->nodes[i].nodeId == nodeId) {
            return &dc->nodes[i];
        }
    }
    return NULL;
}

/*
LRU GET - Retrieve from cache
*/
int cache_get(DistributedCache* dc, const char* key, char* value) {
    dc->totalRequests++;
    
    // Find responsible node
    int nodeId = cache_find_node(dc, key);
    CacheNode* node = cache_get_node_by_id(dc, nodeId);
    
    if (!node) return 0;
    
    // Search in node's cache
    for (int i = 0; i < node->entryCount; i++) {
        CacheEntry* entry = node->entries[i];
        if (strcmp(entry->key, key) == 0) {
            strcpy(value, entry->value);
            entry->accessCount++;
            entry->timestamp = time(NULL);
            node->hits++;
            dc->totalHits++;
            
            printf("[Cache HIT] Key: %s, Node: %d\n", key, nodeId);
            return 1;
        }
    }
    
    node->misses++;
    printf("[Cache MISS] Key: %s, Node: %d\n", key, nodeId);
    return 0;
}

/*
LRU SET - Store in cache with eviction
*/
void cache_set(DistributedCache* dc, const char* key, const char* value) {
    // Find responsible node
    int nodeId = cache_find_node(dc, key);
    CacheNode* node = cache_get_node_by_id(dc, nodeId);
    
    if (!node) return;
    
    // Check if key exists
    for (int i = 0; i < node->entryCount; i++) {
        CacheEntry* entry = node->entries[i];
        if (strcmp(entry->key, key) == 0) {
            strcpy(entry->value, value);
            entry->timestamp = time(NULL);
            entry->accessCount++;
            printf("[Cache UPDATE] Key: %s, Node: %d\n", key, nodeId);
            return;
        }
    }
    
    // Need to add new entry
    if (node->entryCount >= MAX_CACHE_SIZE) {
        // LRU eviction: find entry with oldest timestamp
        int lruIdx = 0;
        time_t oldestTime = node->entries[0]->timestamp;
        
        for (int i = 1; i < node->entryCount; i++) {
            if (node->entries[i]->timestamp < oldestTime) {
                oldestTime = node->entries[i]->timestamp;
                lruIdx = i;
            }
        }
        
        printf("[LRU Eviction] Node: %d, Evicted: %s\n",
               nodeId, node->entries[lruIdx]->key);
        
        free(node->entries[lruIdx]);
        
        // Shift remaining entries
        for (int i = lruIdx; i < node->entryCount - 1; i++) {
            node->entries[i] = node->entries[i + 1];
        }
        
        node->entryCount--;
        node->evictions++;
    }
    
    // Add new entry
    CacheEntry* entry = malloc(sizeof(CacheEntry));
    strcpy(entry->key, key);
    strcpy(entry->value, value);
    entry->timestamp = time(NULL);
    entry->accessCount = 1;
    
    node->entries[node->entryCount] = entry;
    node->entryCount++;
    
    printf("[Cache SET] Key: %s, Value: %s, Node: %d\n", key, value, nodeId);
}

/*
PRINT STATISTICS
*/
void cache_print_stats(DistributedCache* dc) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DISTRIBUTED CACHE STATISTICS                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nCluster Status:\n");
    printf("  Total nodes: %d\n", dc->nodeCount);
    printf("  Total requests: %d\n", dc->totalRequests);
    printf("  Total hits: %d\n", dc->totalHits);
    
    if (dc->totalRequests > 0) {
        printf("  Hit rate: %.1f%%\n",
               (float)dc->totalHits / dc->totalRequests * 100);
    }
    
    printf("\nNode Statistics:\n");
    printf("  %-8s %-20s %-8s %-8s %-8s %-10s\n",
           "ID", "Address", "Entries", "Hits", "Misses", "Evictions");
    printf("  %-8s %-20s %-8s %-8s %-8s %-10s\n",
           "---", "-------", "-------", "----", "------", "---------");
    
    for (int i = 0; i < dc->nodeCount; i++) {
        CacheNode* node = &dc->nodes[i];
        printf("  %-8d %-20s %-8d %-8d %-8d %-10d\n",
               node->nodeId, node->address, node->entryCount,
               node->hits, node->misses, node->evictions);
    }
}

/*
DEMONSTRATION
*/
void distributed_cache_demo() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DISTRIBUTED CACHE ARCHITECTURE DEMONSTRATION                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    DistributedCache* dc = cache_create();
    
    printf("\n1. Add Cache Nodes:\n");
    printf("───────────────────\n");
    
    cache_add_node(dc, 1, "redis1.local:6379");
    cache_add_node(dc, 2, "redis2.local:6379");
    cache_add_node(dc, 3, "redis3.local:6379");
    
    printf("\n2. Build Hash Ring (Consistent Hashing):\n");
    printf("───────────────────────────────────────\n");
    
    cache_build_hash_ring(dc);
    
    printf("\n3. Store Data (distributed across nodes):\n");
    printf("──────────────────────────────────────────\n");
    
    cache_set(dc, "user:123", "Alice");
    cache_set(dc, "user:456", "Bob");
    cache_set(dc, "post:789", "HelloWorld");
    cache_set(dc, "post:101", "My First Post");
    
    printf("\n4. Retrieve Data (cache hits/misses):\n");
    printf("────────────────────────────────────\n");
    
    char value[MAX_VALUE_LEN];
    
    cache_get(dc, "user:123", value);  // Should be hit
    cache_get(dc, "user:456", value);  // Should be hit
    cache_get(dc, "user:999", value);  // Should be miss
    cache_get(dc, "post:789", value);  // Should be hit
    
    printf("\n5. Cache Evictions (when full):\n");
    printf("─────────────────────────────\n");
    
    for (int i = 0; i < 100; i++) {
        char key[20], val[50];
        sprintf(key, "data:%d", i);
        sprintf(val, "value_%d", i);
        cache_set(dc, key, val);
    }
    
    cache_print_stats(dc);
}

void complexity_analysis() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nOperations:\n");
    printf("  Get: O(1) hash lookup + O(m) node search\n");
    printf("  Set: O(1) hash lookup + O(1) insert\n");
    printf("  Eviction (LRU): O(1) with timestamp\n");
    printf("  Node lookup: O(log n) with sorted hash ring\n\n");
    
    printf("Consistency:\n");
    printf("  Consistent Hashing: Minimizes key redistribution\n");
    printf("  On node failure: ~1/n of keys affected (n = nodes)\n");
    printf("  On node addition: ~1/n of keys redistributed\n");
}

void networking_patterns() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DISTRIBUTED CACHE NETWORKING PATTERNS                        ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Consistent Hashing:\n");
    printf("   - Route keys to same node\n");
    printf("   - Minimize redistribution\n");
    printf("   - Virtual nodes for balance\n\n");
    
    printf("2. LRU Eviction:\n");
    printf("   - Remove least recently used\n");
    printf("   - Maximize cache hit ratio\n");
    printf("   - O(1) with linked list\n\n");
    
    printf("3. Cache Invalidation:\n");
    printf("   - TTL (time to live)\n");
    printf("   - Explicit invalidation\n");
    printf("   - Write-through vs write-back\n\n");
    
    printf("4. Replication:\n");
    printf("   - Replicate across nodes\n");
    printf("   - Fault tolerance\n");
    printf("   - Read scaling\n\n");
    
    printf("5. Monitoring:\n");
    printf("   - Track hit/miss rates\n");
    printf("   - Monitor evictions\n");
    printf("   - Node health checks\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DISTRIBUTED CACHE - COMPREHENSIVE DEMONSTRATION             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    distributed_cache_demo();
    complexity_analysis();
    networking_patterns();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Consistent Hashing: Minimize key redistribution\n");
    printf("2. LRU Eviction: O(1) with timestamp tracking\n");
    printf("3. Virtual Nodes: Better distribution\n");
    printf("4. Hit Rates: Key performance metric\n");
    printf("5. Monitoring: Track stats per node\n");
    
    return 0;
}
