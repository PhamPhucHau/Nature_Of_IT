/*
================================================================================
HASHING ALGORITHM - JAVA VERSION
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
================================================================================
*/

import java.util.*;

public class Hashing {
    
    /*
    HASH FUNCTIONS
    */
    
    // Simple modulo hash
    static int hashSimple(String key, int size) {
        int hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash += key.charAt(i);
        }
        return hash % size;
    }
    
    // DJB2 hash (better distribution)
    static int hashDJB2(String key, int size) {
        long hash = 5381;
        for (int i = 0; i < key.length(); i++) {
            hash = ((hash << 5) + hash) + (long)key.charAt(i);
        }
        return (int)(hash % size);
    }
    
    // Knuth's hash
    static int hashKnuth(String key, int size) {
        long hash = 0;
        for (int i = 0; i < key.length(); i++) {
            hash = hash * 65599L + (long)key.charAt(i);
        }
        return (int)(hash % size);
    }
    
    /*
    CHAIN HASH TABLE: Handle collisions with chaining
    */
    static class ChainHashTable<K, V> {
        static class Entry<K, V> {
            K key;
            V value;
            Entry<K, V> next;
            
            Entry(K key, V value) {
                this.key = key;
                this.value = value;
            }
        }
        
        private List<Entry<K, V>>[] chains;
        private int size;
        private int collisionCount;
        
        @SuppressWarnings("unchecked")
        ChainHashTable(int size) {
            this.size = size;
            this.chains = new List[size];
            this.collisionCount = 0;
            
            for (int i = 0; i < size; i++) {
                this.chains[i] = new LinkedList<>();
            }
        }
        
        int hash(K key) {
            return hashDJB2(key.toString(), size);
        }
        
        void put(K key, V value) {
            int index = hash(key);
            
            if (!chains[index].isEmpty()) {
                collisionCount++;
            }
            
            // Check if key already exists
            for (Entry<K, V> entry : chains[index]) {
                if (entry.key.equals(key)) {
                    entry.value = value;  // Update
                    return;
                }
            }
            
            // Add new entry
            chains[index].add(0, new Entry<>(key, value));
        }
        
        V get(K key) {
            int index = hash(key);
            
            for (Entry<K, V> entry : chains[index]) {
                if (entry.key.equals(key)) {
                    return entry.value;
                }
            }
            
            return null;
        }
        
        int getCollisionCount() {
            return collisionCount;
        }
    }
    
    static void dnsCacheDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: DNS CACHE - Domain to IP lookup                  ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        ChainHashTable<String, String> dnsCache = new ChainHashTable<>(50);
        
        System.out.println("\nAdding DNS entries:");
        dnsCache.put("google.com", "142.250.80.46");
        dnsCache.put("facebook.com", "31.13.64.35");
        dnsCache.put("github.com", "140.82.121.4");
        dnsCache.put("twitter.com", "104.244.42.1");
        dnsCache.put("youtube.com", "142.251.32.1");
        
        System.out.println("Added 5 domain entries");
        System.out.println("Collisions encountered: " + dnsCache.getCollisionCount());
        
        System.out.println("\nDNS lookup tests:");
        
        String[] lookups = {"google.com", "github.com", "unknown.com", "facebook.com"};
        for (String domain : lookups) {
            String ip = dnsCache.get(domain);
            if (ip != null) {
                System.out.println("  " + domain + " → " + ip + " ✓");
            } else {
                System.out.println("  " + domain + " → NOT FOUND ✗");
            }
        }
        
        System.out.println("\nNETWORK APPLICATION:");
        System.out.println("- Typical DNS servers handle ~100M entries");
        System.out.println("- Hashing provides O(1) lookup");
        System.out.println("- Without hashing: O(log N) with B-tree");
        System.out.println("- Hashing: ~100x faster for large datasets");
    }
    
    static void arpTableDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: ARP TABLE - IP to MAC address lookup             ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        ChainHashTable<String, String> arpTable = new ChainHashTable<>(100);
        
        System.out.println("\nAdding ARP entries (IP → MAC):");
        arpTable.put("192.168.1.1", "00:1A:2B:3C:4D:5E");
        arpTable.put("192.168.1.10", "00:11:22:33:44:55");
        arpTable.put("192.168.1.20", "00:AA:BB:CC:DD:EE");
        arpTable.put("192.168.1.100", "00:22:44:66:88:AA");
        arpTable.put("192.168.1.200", "00:FF:EE:DD:CC:BB");
        
        System.out.println("Added 5 ARP entries");
        System.out.println("Collisions: " + arpTable.getCollisionCount());
        
        System.out.println("\nARP lookup tests:");
        
        String[] ips = {"192.168.1.1", "192.168.1.100", "192.168.1.50"};
        for (String ip : ips) {
            String mac = arpTable.get(ip);
            if (mac != null) {
                System.out.println("  " + ip + " → " + mac + " ✓");
            } else {
                System.out.println("  " + ip + " → NOT FOUND ✗");
            }
        }
        
        System.out.println("\nNETWORK APPLICATION:");
        System.out.println("- ARP protocol maps IP to MAC address");
        System.out.println("- Hash table stores local network mappings");
        System.out.println("- Enables fast layer 2 forwarding");
    }
    
    static void loadBalancingDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: LOAD BALANCING with Hash-based routing           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nSimple hash-based load balancing:");
        System.out.println("Route requests based on hash of client IP\n");
        
        String[] clientIps = {
            "203.0.113.1", "203.0.113.2", "203.0.113.3",
            "203.0.113.4", "203.0.113.5", "203.0.113.6"
        };
        int numServers = 3;
        int[] requests = new int[numServers];
        
        System.out.println("Processing requests:");
        for (String clientIp : clientIps) {
            int serverIndex = hashSimple(clientIp, numServers);
            requests[serverIndex]++;
            System.out.println("  Client " + clientIp + " → Server " + serverIndex);
        }
        
        System.out.println("\nLoad distribution:");
        for (int i = 0; i < numServers; i++) {
            System.out.println("  Server " + i + ": " + requests[i] + " requests");
        }
        
        System.out.println("\nBENEFITS:");
        System.out.println("- Consistent hashing: Same client always goes to same server");
        System.out.println("- O(1) lookup for routing decision");
        System.out.println("- No centralized hash table needed");
    }
    
    static void consistentHashingDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 4: CONSISTENT HASHING for Distributed Systems       ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nConsistent hashing properties:");
        System.out.println("1. When a server goes down, only its keys are redistributed");
        System.out.println("2. When adding a server, it takes its 'fair share' of keys");
        System.out.println("3. Minimal key movement - perfect for distributed caches\n");
        
        System.out.println("Use cases:");
        System.out.println("- Memcached: Distributed in-memory cache");
        System.out.println("- Redis Cluster: Distributed key-value store");
        System.out.println("- Load balancing: Sticky sessions");
        System.out.println("- CDN: Content distribution across nodes");
    }
    
    static void collisionAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COLLISION ANALYSIS - Different Hash Functions                ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        String[] urls = {
            "google.com", "facebook.com", "github.com",
            "twitter.com", "youtube.com", "linkedin.com",
            "amazon.com", "reddit.com", "stackoverflow.com",
            "medium.com"
        };
        int tableSize = 13;
        
        System.out.println("\nTesting 3 hash functions with " + urls.length + 
                         " entries and table size " + tableSize + ":\n");
        
        // Test simple hash
        System.out.println("1. Simple hash (sum % size):");
        int collisions1 = 0;
        boolean[] used1 = new boolean[tableSize];
        for (String url : urls) {
            int idx = hashSimple(url, tableSize);
            if (used1[idx]) collisions1++;
            used1[idx] = true;
            System.out.println("   " + url + " → index " + idx);
        }
        System.out.println("   Collisions: " + collisions1 + "\n");
        
        // Test DJB2 hash
        System.out.println("2. DJB2 hash (better distribution):");
        int collisions2 = 0;
        boolean[] used2 = new boolean[tableSize];
        for (String url : urls) {
            int idx = hashDJB2(url, tableSize);
            if (used2[idx]) collisions2++;
            used2[idx] = true;
            System.out.println("   " + url + " → index " + idx);
        }
        System.out.println("   Collisions: " + collisions2 + "\n");
        
        System.out.println("Key takeaway: Better hash functions reduce collisions");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nLinear Probing:");
        System.out.println("  Average case: O(1) lookup, insert");
        System.out.println("  Worst case: O(n) if table is full");
        System.out.println("  Problem: Clustering (collisions cause more collisions)\n");
        
        System.out.println("Chaining:");
        System.out.println("  Average case: O(1) lookup, insert");
        System.out.println("  Worst case: O(n) if all hash to same bucket");
        System.out.println("  Better: No clustering\n");
        
        System.out.println("Hash Function Quality:");
        System.out.println("  Load factor = entries / table_size");
        System.out.println("  Optimal: Load factor < 0.75");
        System.out.println("  Too high: More collisions");
        System.out.println("  Too low: Wasted memory\n");
        
        System.out.println("Real-world DNS:");
        System.out.println("  Queries per second: ~1 billion");
        System.out.println("  Without hashing: Impossible to handle");
        System.out.println("  With hashing: ~microseconds per lookup");
    }
    
    static void hashTableComparison() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ HASH TABLE vs OTHER DATA STRUCTURES                         ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝\n");
        
        System.out.println("Operation      | Hash Table | Binary Tree | Linear Array");
        System.out.println("---------------|-----------|-------------|-------------");
        System.out.println("Search (avg)   | O(1)       | O(log n)    | O(n)");
        System.out.println("Search (worst) | O(n)       | O(log n)    | O(n)");
        System.out.println("Insert (avg)   | O(1)       | O(log n)    | O(n)");
        System.out.println("Insert (worst) | O(n)       | O(log n)    | O(n)");
        System.out.println("Delete (avg)   | O(1)       | O(log n)    | O(n)");
        System.out.println("Range query    | Difficult  | O(log n+k)  | O(k)");
        System.out.println("Order          | None       | Sorted      | Depends");
        
        System.out.println("\nHash Table winner for:");
        System.out.println("- Cache lookups (DNS, ARP, memcached)");
        System.out.println("- Fast key-value access");
        System.out.println("- Distributed systems");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ HASHING ALGORITHM - JAVA COMPREHENSIVE DEMONSTRATION         ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nHashing: Convert search → computation");
        System.out.println("Time complexity: O(1) average case");
        System.out.println("Essential for: DNS, ARP, caching, load balancing\n");
        
        dnsCacheDemo();
        arpTableDemo();
        loadBalancingDemo();
        consistentHashingDemo();
        collisionAnalysis();
        complexityAnalysis();
        hashTableComparison();
        
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY PRINCIPLES                                               ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. HASH FUNCTION DESIGN:");
        System.out.println("   - Fast to compute");
        System.out.println("   - Uniform distribution");
        System.out.println("   - Minimize collisions\n");
        
        System.out.println("2. COLLISION HANDLING:");
        System.out.println("   - Chaining: Linked lists per bucket");
        System.out.println("   - Probing: Find next empty slot");
        System.out.println("   - Double hashing: Use second hash function\n");
        
        System.out.println("3. LOAD FACTOR:");
        System.out.println("   - Monitor entries/table_size ratio");
        System.out.println("   - Resize table when load > threshold");
        System.out.println("   - Maintain O(1) average performance\n");
        
        System.out.println("4. APPLICATIONS:");
        System.out.println("   - DNS (domain → IP)");
        System.out.println("   - ARP (IP → MAC)");
        System.out.println("   - Routing tables");
        System.out.println("   - Caches (memcached, Redis)");
        System.out.println("   - Load balancing (consistent hashing)");
    }
}
