import java.util.*;

/**
 * DNS ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class DNS {
    
    static class DNSRecord {
        String domain;
        String ipAddress;
        int ttl;
        long createdAt;
        
        DNSRecord(String domain, String ip) {
            this.domain = domain;
            this.ipAddress = ip;
            this.ttl = 300;
            this.createdAt = System.currentTimeMillis();
        }
    }
    
    static class CachedEntry {
        String domain;
        String ipAddress;
        long cachedAt;
        
        CachedEntry(String domain, String ip) {
            this.domain = domain;
            this.ipAddress = ip;
            this.cachedAt = System.currentTimeMillis();
        }
    }
    
    static class DNSServer {
        List<DNSRecord> records;
        Map<String, CachedEntry> cache;
        int lookups;
        int cacheHits;
        int cacheMisses;
        static final int TTL = 300;  // seconds
        
        DNSServer() {
            this.records = new ArrayList<>();
            this.cache = new HashMap<>();
            this.lookups = 0;
            this.cacheHits = 0;
            this.cacheMisses = 0;
            System.out.println("DNS Server created");
        }
        
        void addRecord(String domain, String ip) {
            records.add(new DNSRecord(domain, ip));
            System.out.println("✓ DNS record added: " + domain + " → " + ip);
        }
        
        String lookup(String domain) {
            lookups++;
            long now = System.currentTimeMillis();
            
            // Check cache
            CachedEntry cached = cache.get(domain);
            if (cached != null) {
                long ageSeconds = (now - cached.cachedAt) / 1000;
                if (ageSeconds < TTL) {
                    System.out.println("[Cache HIT] " + domain + " → " + 
                                     cached.ipAddress + " (age: " + ageSeconds + " s)");
                    cacheHits++;
                    return cached.ipAddress;
                } else {
                    System.out.println("[Cache EXPIRED] " + domain + 
                                     " (age: " + ageSeconds + " s > TTL: " + TTL + " s)");
                }
            }
            
            // Cache miss
            System.out.println("[Cache MISS] " + domain + " - querying authoritative");
            cacheMisses++;
            
            for (DNSRecord rec : records) {
                if (rec.domain.equals(domain)) {
                    cache.put(domain, new CachedEntry(domain, rec.ipAddress));
                    System.out.println("[Result] " + domain + " → " + rec.ipAddress);
                    return rec.ipAddress;
                }
            }
            
            System.out.println("[Not Found] " + domain);
            return null;
        }
        
        void hierarchicalLookup(String domain) {
            System.out.println("\n[Hierarchical Resolution] " + domain);
            System.out.println("  1. Query Root Nameserver");
            
            int dotIdx = domain.lastIndexOf('.');
            if (dotIdx > 0) {
                String tld = domain.substring(dotIdx);
                System.out.println("  2. Query TLD Nameserver (" + tld + ")");
            }
            
            System.out.println("  3. Query Authoritative Nameserver");
            lookup(domain);
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ DNS STATISTICS                                               ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nDNS Performance:");
            System.out.println("  Total lookups: " + lookups);
            System.out.println("  Cache hits: " + cacheHits);
            System.out.println("  Cache misses: " + cacheMisses);
            
            if (lookups > 0) {
                System.out.println("  Hit rate: " + 
                                 String.format("%.1f%%", (float)cacheHits / lookups * 100));
            }
            
            System.out.println("\nDNS Records:");
            System.out.println(String.format("  %-30s %-15s %-10s",
                                           "Domain", "IP Address", "TTL"));
            System.out.println(String.format("  %-30s %-15s %-10s",
                                           "------", "----------", "---"));
            
            for (DNSRecord rec : records) {
                System.out.println(String.format("  %-30s %-15s %-10d",
                                                rec.domain, rec.ipAddress, rec.ttl));
            }
            
            System.out.println("\nCached Entries:");
            System.out.println(String.format("  %-30s %-15s", "Domain", "IP Address"));
            System.out.println(String.format("  %-30s %-15s", "------", "----------"));
            
            long now = System.currentTimeMillis();
            for (Map.Entry<String, CachedEntry> e : cache.entrySet()) {
                CachedEntry entry = e.getValue();
                long ageSeconds = (now - entry.cachedAt) / 1000;
                if (ageSeconds < TTL) {
                    System.out.println(String.format("  %-30s %-15s (age: %d s)",
                                                    entry.domain, entry.ipAddress, ageSeconds));
                }
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DNS ARCHITECTURE - COMPREHENSIVE DEMONSTRATION               ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        DNSServer dns = new DNSServer();
        
        System.out.println("\n1. Add DNS Records:");
        System.out.println("───────────────────");
        
        dns.addRecord("google.com", "142.250.185.46");
        dns.addRecord("github.com", "140.82.113.4");
        dns.addRecord("stackoverflow.com", "151.101.1.140");
        dns.addRecord("youtube.com", "142.250.80.46");
        
        System.out.println("\n2. Perform DNS Lookups:");
        System.out.println("──────────────────────");
        
        dns.lookup("google.com");
        dns.lookup("github.com");
        dns.lookup("google.com");  // Cache hit
        dns.lookup("stackoverflow.com");
        dns.lookup("youtube.com");
        dns.lookup("google.com");  // Cache hit again
        
        System.out.println("\n3. Hierarchical Resolution:");
        System.out.println("───────────────────────────");
        
        dns.hierarchicalLookup("example.org");
        dns.hierarchicalLookup("test.co.uk");
        
        dns.printStats();
    }
    
    public static void main(String[] args) {
        demo();
        
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY PATTERNS                                                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Hierarchical Structure: Root → TLD → Authoritative");
        System.out.println("2. Recursive Resolution: Resolver handles recursion");
        System.out.println("3. Caching: Reduce load on authoritative");
        System.out.println("4. TTL (Time To Live): Cache invalidation");
        System.out.println("5. Longest Prefix Match: Find specific record");
    }
}
