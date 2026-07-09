/*
================================================================================
BLOOM FILTER DATA STRUCTURE
================================================================================

STORY:
------
A network administrator wants to know: "Is this IP address in our blocklist?"

She has 100 million blocked IPs.

Storing all 100 million is expensive (400 MB).

So she uses a trick: A bit array with multiple hash functions.

If an IP is in the list, its bits are definitely set.

If bits are not set, it's definitely NOT in the list.

But if bits ARE set, it MIGHT be in the list (false positive).

This is a Bloom Filter: Fast, space-efficient, but allows false positives.

NETWORK APPLICATION:
- IP reputation / blocklists (spam detection)
- URL blocking / malware detection
- Cache membership testing
- Distributed caching (to avoid asking all servers)
- Packet filtering
- Deduplication (approximate)
- DDoS detection (who have we seen before?)

COMPLEXITY:
- Insert: O(k) where k = number of hash functions (usually 3-5)
- Lookup: O(k)
- Space: O(m) bits, usually much smaller than original data
- False positive rate: ~0.1% to 1% (configurable)
- False negative rate: ZERO (never false negatives!)

KEY INSIGHT:
Bloom Filter is the only structure with NO false negatives.
If it says "not in set", it's definitely not there.
Trade-off: Can have false positives, but saves 10-100x space.
Perfect for quick rejection before expensive lookups.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define FILTER_SIZE 1000000  // 1 million bits
#define NUM_HASHES 3         // Number of hash functions

typedef struct {
    uint8_t bits[FILTER_SIZE / 8];  // Bit array
    uint32_t size;
    uint32_t insertCount;
    uint32_t falsePositives;
} BloomFilter;

// Simple hash functions
uint32_t hash1(const char* str) {
    uint32_t hash = 5381;
    for (int i = 0; str[i]; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)str[i];
    }
    return hash % FILTER_SIZE;
}

uint32_t hash2(const char* str) {
    uint32_t hash = 33;
    for (int i = 0; str[i]; i++) {
        hash = hash * 33 + (unsigned char)str[i];
    }
    return hash % FILTER_SIZE;
}

uint32_t hash3(const char* str) {
    uint32_t hash = 0;
    for (int i = 0; str[i]; i++) {
        hash = (hash << 7) - hash + (unsigned char)str[i];
    }
    return hash % FILTER_SIZE;
}

BloomFilter* bloom_filter_create() {
    BloomFilter* bf = malloc(sizeof(BloomFilter));
    bf->size = FILTER_SIZE;
    bf->insertCount = 0;
    bf->falsePositives = 0;
    memset(bf->bits, 0, FILTER_SIZE / 8);
    return bf;
}

void set_bit(BloomFilter* bf, uint32_t index) {
    uint32_t byteIndex = index / 8;
    uint32_t bitIndex = index % 8;
    bf->bits[byteIndex] |= (1 << bitIndex);
}

int get_bit(BloomFilter* bf, uint32_t index) {
    uint32_t byteIndex = index / 8;
    uint32_t bitIndex = index % 8;
    return (bf->bits[byteIndex] >> bitIndex) & 1;
}

/*
INSERT into Bloom Filter - O(k) where k = number of hashes
*/
void bloom_filter_insert(BloomFilter* bf, const char* item) {
    uint32_t h1 = hash1(item);
    uint32_t h2 = hash2(item);
    uint32_t h3 = hash3(item);
    
    set_bit(bf, h1);
    set_bit(bf, h2);
    set_bit(bf, h3);
    
    bf->insertCount++;
}

/*
LOOKUP in Bloom Filter - O(k)
Returns: 1 if POSSIBLY in set, 0 if DEFINITELY not in set
*/
int bloom_filter_contains(BloomFilter* bf, const char* item) {
    uint32_t h1 = hash1(item);
    uint32_t h2 = hash2(item);
    uint32_t h3 = hash3(item);
    
    return get_bit(bf, h1) && get_bit(bf, h2) && get_bit(bf, h3);
}

void basic_bloom_filter_demo() {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: BASIC BLOOM FILTER                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    BloomFilter* bf = bloom_filter_create();
    
    printf("\nInserting items:\n");
    
    const char* items[] = {"apple", "banana", "cherry", "date", "elderberry"};
    
    for (int i = 0; i < 5; i++) {
        bloom_filter_insert(bf, items[i]);
        printf("  Inserted: %s\n", items[i]);
    }
    
    printf("\nChecking membership:\n");
    
    const char* checks[] = {"apple", "banana", "grape", "orange", "date"};
    
    for (int i = 0; i < 5; i++) {
        int result = bloom_filter_contains(bf, checks[i]);
        if (result) {
            printf("  %s: POSSIBLY in set\n", checks[i]);
        } else {
            printf("  %s: DEFINITELY not in set ✓\n", checks[i]);
        }
    }
    
    printf("\nBloom Filter properties:\n");
    printf("  Items inserted: %d\n", bf->insertCount);
    printf("  Bits set: ~%.1f%%\n", 
           (float)bf->insertCount * NUM_HASHES * 100 / FILTER_SIZE);
}

void ip_blocklist_demo() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: IP BLOCKLIST (Real Network Application)           ║\n");
    printf("║ Spam detection with minimal memory                           ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    BloomFilter* blocklist = bloom_filter_create();
    
    printf("\nSimulating blocklist with spam IPs:\n");
    
    const char* spam_ips[] = {
        "192.0.2.1",
        "198.51.100.5",
        "203.0.113.42",
        "192.88.99.1",
        "198.18.0.0"
    };
    
    for (int i = 0; i < 5; i++) {
        bloom_filter_insert(blocklist, spam_ips[i]);
        printf("  Blocked: %s\n", spam_ips[i]);
    }
    
    printf("\nIncoming email checks:\n");
    
    const char* incoming_ips[] = {
        "192.0.2.1",      // Spam
        "1.2.3.4",        // Unknown
        "198.51.100.5",   // Spam
        "8.8.8.8",        // Unknown
        "203.0.113.42"    // Spam
    };
    
    for (int i = 0; i < 5; i++) {
        if (bloom_filter_contains(blocklist, incoming_ips[i])) {
            printf("  %s: BLOCK (might be spam)\n", incoming_ips[i]);
        } else {
            printf("  %s: ALLOW (definitely not spam) ✓\n", incoming_ips[i]);
        }
    }
    
    printf("\nMemory usage:\n");
    printf("  Bloom Filter: ~%.1f KB\n", (float)FILTER_SIZE / 8 / 1024);
    printf("  Hash Table (same IPs): ~5 KB\n");
    printf("  Memory saved: 99%% but with small false positive risk!\n");
}

void false_positive_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ FALSE POSITIVE ANALYSIS                                     ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBloom Filter guarantee:\n");
    printf("  False Negative Rate: 0%% (NO false negatives!)\n");
    printf("  False Positive Rate: Depends on:\n");
    printf("    - Filter size (m)\n");
    printf("    - Number of items inserted (n)\n");
    printf("    - Number of hash functions (k)\n\n");
    
    printf("Typical false positive rates:\n");
    printf("  Small filter (n/m = 0.01): ~0.1%% false positives\n");
    printf("  Medium filter (n/m = 0.1): ~1%% false positives\n");
    printf("  Large filter (n/m = 1.0): ~10%% false positives\n\n");
    
    printf("Formula: P(false positive) ≈ (1 - e^(-kn/m))^k\n");
    printf("  k = number of hash functions (usually 3-5)\n");
    printf("  n = items inserted\n");
    printf("  m = filter size in bits\n");
    
    printf("\nOptimal number of hash functions:\n");
    printf("  k_opt = (m/n) * ln(2) ≈ 0.693 * m/n\n");
    printf("  Usually 3-5 functions give best results\n");
}

void scalability_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ SCALABILITY FOR LARGE DATASETS                              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nScenario: Checking if URL is in malware database\n\n");
    
    printf("Database size: 1 billion URLs\n");
    printf("Each URL: ~100 bytes\n\n");
    
    printf("Traditional storage:\n");
    printf("  Hash Table: 1B * 100 bytes = 100 GB\n");
    printf("  Lookup time: Fast but huge memory\n\n");
    
    printf("Bloom Filter:\n");
    printf("  Size: 1B * 10 bits (optimal) = 1.25 GB\n");
    printf("  Memory saved: 99%%\n");
    printf("  False positive: ~1%% (acceptable for pre-filter)\n");
    printf("  Lookup: O(3) hash functions\n\n");
    
    printf("Workflow:\n");
    printf("  1. Check URL in Bloom Filter (1.25 GB)\n");
    printf("    - If NO: definitely not malware ✓ (99%% of URLs)\n");
    printf("    - If YES: check in full database (expensive)\n");
    printf("  2. This saves disk/network access for 99%% of URLs!\n");
}

void counting_bloom_filter_concept() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COUNTING BLOOM FILTER (Advanced)                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nStandard Bloom Filter limitation:\n");
    printf("  Cannot delete items (bits are never cleared)\n");
    printf("  Once inserted, stays forever\n\n");
    
    printf("Counting Bloom Filter solution:\n");
    printf("  Use counter (e.g., 4-bit) instead of single bit\n");
    printf("  Increment on insert, decrement on delete\n");
    printf("  Disadvantage: ~4x more memory than standard\n\n");
    
    printf("When to use:\n");
    printf("  Standard Bloom: Static data (whitelist, blocklist)\n");
    printf("  Counting Bloom: Dynamic data (cache membership)\n");
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBloom Filter:\n");
    printf("  Insert: O(k) where k = hash functions\n");
    printf("  Lookup: O(k)\n");
    printf("  Delete: O(k) with counting version\n");
    printf("  Space: O(m) bits\n");
    printf("  False negatives: 0%% (NEVER)\n");
    printf("  False positives: ~1%% (configurable)\n\n");
    
    printf("vs Hash Set:\n");
    printf("  Insert: O(1)\n");
    printf("  Lookup: O(1)\n");
    printf("  Space: O(n * element_size)\n");
    printf("  False positives: 0%%\n");
    printf("  False negatives: 0%%\n\n");
    
    printf("Bloom Filter wins when:\n");
    printf("  - Memory is scarce (embedded systems)\n");
    printf("  - Can tolerate ~1%% false positive\n");
    printf("  - Data is read-heavy, write-rare\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ BLOOM FILTER - COMPREHENSIVE DEMONSTRATION                    ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nBloom Filter: Space-efficient membership testing\n");
    printf("Key insight: NO false negatives, only false positives\n");
    printf("Perfect for: Filtering, caching, IP reputation\n");
    
    basic_bloom_filter_demo();
    ip_blocklist_demo();
    false_positive_analysis();
    scalability_analysis();
    counting_bloom_filter_concept();
    complexity_analysis();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Bloom Filter: O(k) insert/lookup, O(m) space\n");
    printf("2. Zero false negatives (NEVER miss items)\n");
    printf("3. Small false positive rate (~1%%)\n");
    printf("4. Use as pre-filter before expensive lookups\n");
    printf("5. Save 10-100x memory vs hash table!\n");
    
    return 0;
}
