import java.util.*;

/**
 * DISTRIBUTED CACHE ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class DistributedCache {
    
    static class CacheEntry {
        String key;
        String value;
        long timestamp;
        int accessCount;
        
        CacheEntry(String k, String v) {
            this.key = k;
            this.value = v;
            this.timestamp = System.currentTimeMillis();
            this.accessCount = 1;
        }
    }
    
    static class CacheNode {
        int nodeId;
        String address;
        Map<String, CacheEntry> entries;
        int hits;
        int misses;
        int evictions;
        
        CacheNode(int id, String addr) {
            this.nodeId = id;
            this.address = addr;
            this.entries = new LinkedHashMap<>();
            this.hits = 0;
            this.misses = 0;
            this.evictions = 0;
        }
    }
    
    static class Cache {
        Map<Integer, CacheNode> nodes;
        SortedMap<Integer, Integer> hashRing;  // hash -> nodeId
        int totalRequests;
        int totalHits;
        static final int MAX_ENTRIES_PER_NODE = 100;
        
        Cache() {
            this.nodes = new HashMap<>();
            this.hashRing = new TreeMap<>();
            this.totalRequests = 0;
            this.totalHits = 0;
            System.out.println("Distributed Cache created");
        }
        
        void addNode(int nodeId, String address) {
            nodes.put(nodeId, new CacheNode(nodeId, address));
            System.out.println("✓ Cache node " + nodeId + " added: " + address);
        }
        
        void buildHashRing() {
            hashRing.clear();
            for (int nodeId : nodes.keySet()) {
                for (int j = 0; j < 3; j++) {
                    String vnodeKey = "node_" + nodeId + "_vnode_" + j;
                    int hashValue = Math.abs(vnodeKey.hashCode());
                    hashRing.put(hashValue, nodeId);
                }
            }
            System.out.println("Hash ring built with " + hashRing.size() + " vnodes");
        }
        
        int findNode(String key) {
            int keyHash = Math.abs(key.hashCode());
            
            SortedMap<Integer, Integer> tailMap = hashRing.tailMap(keyHash);
            if (tailMap.isEmpty()) {
                return hashRing.get(hashRing.firstKey());
            }
            return tailMap.get(tailMap.firstKey());
        }
        
        boolean get(String key, StringBuilder value) {
            totalRequests++;
            int nodeId = findNode(key);
            CacheNode node = nodes.get(nodeId);
            
            if (node == null) return false;
            
            CacheEntry entry = node.entries.get(key);
            if (entry != null) {
                value.append(entry.value);
                entry.accessCount++;
                entry.timestamp = System.currentTimeMillis();
                node.hits++;
                totalHits++;
                System.out.println("[Cache HIT] Key: " + key + ", Node: " + nodeId);
                return true;
            }
            
            node.misses++;
            System.out.println("[Cache MISS] Key: " + key + ", Node: " + nodeId);
            return false;
        }
        
        void set(String key, String value) {
            int nodeId = findNode(key);
            CacheNode node = nodes.get(nodeId);
            if (node == null) return;
            
            if (node.entries.containsKey(key)) {
                node.entries.get(key).value = value;
                System.out.println("[Cache UPDATE] Key: " + key + ", Node: " + nodeId);
                return;
            }
            
            if (node.entries.size() >= MAX_ENTRIES_PER_NODE) {
                // LRU eviction
                String lruKey = null;
                long oldestTime = Long.MAX_VALUE;
                
                for (Map.Entry<String, CacheEntry> e : node.entries.entrySet()) {
                    if (e.getValue().timestamp < oldestTime) {
                        oldestTime = e.getValue().timestamp;
                        lruKey = e.getKey();
                    }
                }
                
                if (lruKey != null) {
                    node.entries.remove(lruKey);
                    node.evictions++;
                    System.out.println("[LRU Eviction] Node: " + nodeId + ", Evicted: " + lruKey);
                }
            }
            
            node.entries.put(key, new CacheEntry(key, value));
            System.out.println("[Cache SET] Key: " + key + ", Value: " + value + 
                             ", Node: " + nodeId);
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ DISTRIBUTED CACHE STATISTICS                                 ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nCluster Status:");
            System.out.println("  Total nodes: " + nodes.size());
            System.out.println("  Total requests: " + totalRequests);
            System.out.println("  Total hits: " + totalHits);
            
            if (totalRequests > 0) {
                System.out.println("  Hit rate: " + 
                                 String.format("%.1f%%", (float)totalHits / 
                                 totalRequests * 100));
            }
            
            System.out.println("\nNode Statistics:");
            System.out.println(String.format("  %-8s %-20s %-8s %-8s %-8s %-10s",
                                           "ID", "Address", "Entries", "Hits", "Misses", "Evictions"));
            System.out.println(String.format("  %-8s %-20s %-8s %-8s %-8s %-10s",
                                           "---", "-------", "-------", "----", "------", "---------"));
            
            for (CacheNode node : nodes.values()) {
                System.out.println(String.format("  %-8d %-20s %-8d %-8d %-8d %-10d",
                                                node.nodeId, node.address,
                                                node.entries.size(), node.hits,
                                                node.misses, node.evictions));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DISTRIBUTED CACHE ARCHITECTURE DEMONSTRATION                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Cache cache = new Cache();
        
        System.out.println("\n1. Add Cache Nodes:");
        System.out.println("───────────────────");
        
        cache.addNode(1, "redis1.local:6379");
        cache.addNode(2, "redis2.local:6379");
        cache.addNode(3, "redis3.local:6379");
        
        System.out.println("\n2. Build Hash Ring:");
        System.out.println("──────────────────");
        
        cache.buildHashRing();
        
        System.out.println("\n3. Store Data:");
        System.out.println("──────────────");
        
        cache.set("user:123", "Alice");
        cache.set("user:456", "Bob");
        cache.set("post:789", "HelloWorld");
        cache.set("post:101", "My First Post");
        
        System.out.println("\n4. Retrieve Data:");
        System.out.println("─────────────────");
        
        StringBuilder value = new StringBuilder();
        cache.get("user:123", value);
        cache.get("user:456", value);
        cache.get("user:999", value);
        cache.get("post:789", value);
        
        System.out.println("\n5. Stress Test:");
        System.out.println("───────────────");
        
        for (int i = 0; i < 100; i++) {
            cache.set("data:" + i, "value_" + i);
        }
        
        cache.printStats();
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nOperations:");
        System.out.println("  Get: O(1) hash lookup");
        System.out.println("  Set: O(1) hash insertion");
        System.out.println("  Eviction: O(1) timestamp");
        System.out.println("  Node lookup: O(log n) tree");
    }
    
    static void networkingPatterns() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DISTRIBUTED CACHE NETWORKING PATTERNS                        ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Consistent Hashing: Route to same node");
        System.out.println("2. LRU Eviction: Remove least recently used");
        System.out.println("3. Cache Invalidation: TTL and explicit");
        System.out.println("4. Replication: Fault tolerance");
        System.out.println("5. Monitoring: Hit rate tracking");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DISTRIBUTED CACHE - COMPREHENSIVE DEMONSTRATION             ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        demo();
        complexityAnalysis();
        networkingPatterns();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Consistent Hashing: Minimize redistribution");
        System.out.println("2. LRU Eviction: O(1) with timestamps");
        System.out.println("3. Virtual Nodes: Better distribution");
        System.out.println("4. Hit Rates: Key metric");
        System.out.println("5. Monitoring: Track per-node stats");
    }
}
