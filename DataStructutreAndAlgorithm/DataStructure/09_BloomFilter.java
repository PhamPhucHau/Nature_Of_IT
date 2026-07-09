/*
================================================================================
BLOOM FILTER - JAVA VERSION
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
- Distributed caching
- Packet filtering
- Deduplication (approximate)
- DDoS detection

COMPLEXITY:
- Insert: O(k) where k = number of hash functions
- Lookup: O(k)
- Space: O(m) bits, usually 10-100x smaller than original data
- False positive rate: ~0.1% to 1%
- False negative rate: ZERO (never false negatives!)

KEY INSIGHT:
Trade small false positive rate for massive space savings.
================================================================================
*/

import java.util.*;

public class BloomFilterDS {
    
    static class BloomFilter {
        private BitSet bits;
        private int size;
        private int insertCount;
        private static final int NUM_HASHES = 3;
        
        BloomFilter(int sizeInBits) {
            this.bits = new BitSet(sizeInBits);
            this.size = sizeInBits;
            this.insertCount = 0;
        }
        
        // Hash functions
        private int hash1(String item) {
            int hash = 5381;
            for (char c : item.toCharArray()) {
                hash = ((hash << 5) + hash) + c;
            }
            return Math.abs(hash % size);
        }
        
        private int hash2(String item) {
            int hash = 33;
            for (char c : item.toCharArray()) {
                hash = hash * 33 + c;
            }
            return Math.abs(hash % size);
        }
        
        private int hash3(String item) {
            int hash = 0;
            for (char c : item.toCharArray()) {
                hash = (hash << 7) - hash + c;
            }
            return Math.abs(hash % size);
        }
        
        // Insert - O(k) where k = number of hashes
        void insert(String item) {
            bits.set(hash1(item));
            bits.set(hash2(item));
            bits.set(hash3(item));
            insertCount++;
        }
        
        // Lookup - O(k)
        // Returns: true if POSSIBLY in set, false if DEFINITELY not in set
        boolean contains(String item) {
            return bits.get(hash1(item)) && 
                   bits.get(hash2(item)) && 
                   bits.get(hash3(item));
        }
        
        int getBitsSet() {
            return bits.cardinality();
        }
        
        double getFalsePositiveRate() {
            double n = insertCount;
            double m = size;
            double k = NUM_HASHES;
            // P = (1 - e^(-kn/m))^k
            double exponent = -k * n / m;
            double base = 1.0 - Math.exp(exponent);
            return Math.pow(base, k);
        }
    }
    
    static void basicBloomFilterDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: BASIC BLOOM FILTER                               ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BloomFilter bf = new BloomFilter(1000);
        
        System.out.println("\nInserting items:");
        
        String[] items = {"apple", "banana", "cherry", "date", "elderberry"};
        
        for (String item : items) {
            bf.insert(item);
            System.out.println("  Inserted: " + item);
        }
        
        System.out.println("\nChecking membership:");
        
        String[] checks = {"apple", "banana", "grape", "orange", "date"};
        
        for (String item : checks) {
            boolean result = bf.contains(item);
            if (result) {
                System.out.println("  " + item + ": POSSIBLY in set");
            } else {
                System.out.println("  " + item + ": DEFINITELY not in set ✓");
            }
        }
        
        System.out.println("\nBloom Filter statistics:");
        System.out.println("  Items inserted: " + items.length);
        System.out.println("  Bits set: " + bf.getBitsSet());
        System.out.printf("  Estimated false positive: %.2f%%\n", 
                         bf.getFalsePositiveRate() * 100);
    }
    
    static void ipBlocklistDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: IP BLOCKLIST (Real Network Application)           ║");
        System.out.println("║ Spam detection with minimal memory                           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BloomFilter blocklist = new BloomFilter(100000);
        
        System.out.println("\nSimulating blocklist with spam IPs:");
        
        String[] spamIps = {
            "192.0.2.1",
            "198.51.100.5",
            "203.0.113.42",
            "192.88.99.1",
            "198.18.0.0"
        };
        
        for (String ip : spamIps) {
            blocklist.insert(ip);
            System.out.println("  Blocked: " + ip);
        }
        
        System.out.println("\nIncoming email checks:");
        
        String[] incomingIps = {
            "192.0.2.1",      // Spam
            "1.2.3.4",        // Unknown
            "198.51.100.5",   // Spam
            "8.8.8.8",        // Unknown
            "203.0.113.42"    // Spam
        };
        
        int blocked = 0;
        for (String ip : incomingIps) {
            if (blocklist.contains(ip)) {
                System.out.println("  " + ip + ": BLOCK (might be spam)");
                blocked++;
            } else {
                System.out.println("  " + ip + ": ALLOW (definitely not spam) ✓");
            }
        }
        
        System.out.println("\nBlocking rate: " + blocked + "/" + incomingIps.length);
        System.out.println("\nMemory usage:");
        System.out.printf("  Bloom Filter: %.2f KB\n", 100000.0 / 8 / 1024);
        System.out.println("  Hash Set (same IPs): ~0.5 KB");
        System.out.println("  Advantage: Fast pre-filter before expensive lookups!");
    }
    
    static void urlMalwareDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: MALWARE URL DETECTION                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BloomFilter malwareUrls = new BloomFilter(1000000);  // 1M bits
        
        System.out.println("\nSimulating malware URL database (100K URLs):");
        
        for (int i = 0; i < 100000; i++) {
            malwareUrls.insert("malware-" + i + ".com");
        }
        
        System.out.println("  Inserted: 100,000 URLs");
        
        System.out.println("\nChecking user visits:");
        
        int blocked = 0;
        int allowed = 0;
        int total = 10000;
        
        for (int i = 0; i < total; i++) {
            String url;
            if (i % 100 < 50) {
                // 50% actual malware
                url = "malware-" + (int)(Math.random() * 100000) + ".com";
            } else {
                // 50% benign
                url = "safe-" + i + ".com";
            }
            
            if (malwareUrls.contains(url)) {
                blocked++;
            } else {
                allowed++;
            }
        }
        
        System.out.println("Results:");
        System.out.println("  Blocked (maybe malware): " + blocked);
        System.out.println("  Allowed (safe): " + allowed);
        System.out.printf("  False positive rate: %.2f%%\n", 
                         malwareUrls.getFalsePositiveRate() * 100);
        
        System.out.println("\nBenefit: Protects users from malware with minimal memory!");
    }
    
    static void complexityComparison() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nBloom Filter vs Hash Set (1 billion items):");
        System.out.println("\nOperation    | Bloom Filter | Hash Set    | Winner");
        System.out.println("-------------|--------------|-------------|--------");
        System.out.println("Insert       | O(3)         | O(1)        | Hash Set");
        System.out.println("Lookup       | O(3)         | O(1)        | Hash Set");
        System.out.println("Space        | 1.25 GB      | 100 GB      | Bloom ✓");
        System.out.println("Deletion     | Not possible | O(1)        | Hash Set");
        System.out.println("False +ve    | ~1%          | 0%          | Hash Set");
        System.out.println("False -ve    | 0% ✓         | 0%          | Tie");
        
        System.out.println("\nWhen to use Bloom Filter:");
        System.out.println("1. Memory is critical (embedded systems, IoT)");
        System.out.println("2. Data is huge (1B+ items)");
        System.out.println("3. 1% false positive is acceptable");
        System.out.println("4. Quick pre-filter before expensive lookup");
    }
    
    static void scalabilityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ REAL-WORLD SCALABILITY EXAMPLE                              ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nScenario: Google Safe Browsing for malware URLs");
        System.out.println("\nDatabase: 10 million malicious URLs");
        System.out.println("Per URL: average 50 bytes\n");
        
        System.out.println("Traditional approach:");
        System.out.println("  Storage: 10M * 50 bytes = 500 MB");
        System.out.println("  Network: Download full database (~100 MB every hour)");
        System.out.println("  Memory: Load into browser RAM\n");
        
        System.out.println("Bloom Filter approach:");
        System.out.println("  Storage: 10M * 10 bits = 12.5 MB");
        System.out.println("  Network: Download just 12.5 MB");
        System.out.println("  Memory: Minimal (fits in browser cache)");
        System.out.println("  False positive: Send 1% safe URLs to server for verification\n");
        
        System.out.println("Result: 40x smaller database, 95% reduction in network!");
    }
    
    static void advancedVariants() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ ADVANCED VARIANTS                                           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Counting Bloom Filter:");
        System.out.println("   - Use counter instead of single bit");
        System.out.println("   - Allows deletion");
        System.out.println("   - Trade-off: 4x more memory\n");
        
        System.out.println("2. Partitioned Bloom Filter:");
        System.out.println("   - Split into multiple smaller filters");
        System.out.println("   - Better locality of reference");
        System.out.println("   - Easier caching\n");
        
        System.out.println("3. Layered Bloom Filter:");
        System.out.println("   - Multiple layers with different false positive rates");
        System.out.println("   - Trade latency for accuracy\n");
        
        System.out.println("4. Scalable Bloom Filter:");
        System.out.println("   - Grow dynamically as data increases");
        System.out.println("   - Maintain false positive rate\n");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ BLOOM FILTER - JAVA COMPREHENSIVE DEMONSTRATION             ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nBloom Filter: Space-efficient membership testing");
        System.out.println("Key insight: NO false negatives, only small false positive rate");
        System.out.println("Perfect for: Filtering, caching, IP reputation\n");
        
        basicBloomFilterDemo();
        ipBlocklistDemo();
        urlMalwareDemo();
        complexityComparison();
        scalabilityAnalysis();
        advancedVariants();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Bloom Filter: O(k) insert/lookup, O(m) bits space");
        System.out.println("2. Zero false negatives (NEVER miss items)");
        System.out.println("3. Controllable false positive (~1%)");
        System.out.println("4. Perfect pre-filter for expensive operations");
        System.out.println("5. Save 10-100x memory vs traditional storage!");
    }
}
