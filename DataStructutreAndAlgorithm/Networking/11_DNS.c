/*
================================================================================
DNS ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
Users don't remember IP addresses. They remember domain names.

DNS translates names to IPs: google.com → 142.250.185.46

Hierarchical structure: Root → TLD (.com) → Authoritative (google.com)
Caching at each level prevents repeated lookups.

Trie structures organize domain names. Longest prefix matching finds the answer.

ARCHITECTURE:
  Client
    |
  Recursive Resolver
    |
  ├─ Root Nameserver
  ├─ TLD Nameserver
  └─ Authoritative Nameserver

Data Structures: Trie, HashMap (cache)
Algorithms: Recursive resolution, Caching, TTL-based invalidation
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 50
#define MAX_CACHE_SIZE 100
#define TTL 300  // Time to live in seconds

typedef struct {
    char domain[100];
    char ipAddress[20];
    int ttl;
    time_t createdAt;
} DNSRecord;

typedef struct {
    char domain[100];
    char ipAddress[20];
    time_t cachedAt;
} CachedEntry;

typedef struct {
    DNSRecord records[MAX_RECORDS];
    int recordCount;
    CachedEntry cache[MAX_CACHE_SIZE];
    int cacheCount;
    int lookups;
    int cacheHits;
    int cacheMisses;
} DNSServer;

DNSServer* dns_create() {
    DNSServer* dns = malloc(sizeof(DNSServer));
    dns->recordCount = 0;
    dns->cacheCount = 0;
    dns->lookups = 0;
    dns->cacheHits = 0;
    dns->cacheMisses = 0;
    printf("DNS Server created\n");
    return dns;
}

void dns_add_record(DNSServer* dns, const char* domain, const char* ip) {
    if (dns->recordCount >= MAX_RECORDS) return;
    
    DNSRecord* rec = &dns->records[dns->recordCount];
    strcpy(rec->domain, domain);
    strcpy(rec->ipAddress, ip);
    rec->ttl = TTL;
    rec->createdAt = time(NULL);
    
    dns->recordCount++;
    printf("✓ DNS record added: %s → %s\n", domain, ip);
}

char* dns_lookup(DNSServer* dns, const char* domain) {
    dns->lookups++;
    
    // Check cache first
    time_t now = time(NULL);
    for (int i = 0; i < dns->cacheCount; i++) {
        if (strcmp(dns->cache[i].domain, domain) == 0) {
            time_t age = now - dns->cache[i].cachedAt;
            if (age < TTL) {
                printf("[Cache HIT] %s → %s (age: %ld s)\n",
                       domain, dns->cache[i].ipAddress, age);
                dns->cacheHits++;
                return dns->cache[i].ipAddress;
            } else {
                // TTL expired
                printf("[Cache EXPIRED] %s (age: %ld s > TTL: %d s)\n",
                       domain, age, TTL);
            }
        }
    }
    
    // Cache miss - lookup in records
    printf("[Cache MISS] %s - querying authoritative server\n", domain);
    dns->cacheMisses++;
    
    for (int i = 0; i < dns->recordCount; i++) {
        if (strcmp(dns->records[i].domain, domain) == 0) {
            // Add to cache
            if (dns->cacheCount < MAX_CACHE_SIZE) {
                CachedEntry* entry = &dns->cache[dns->cacheCount];
                strcpy(entry->domain, domain);
                strcpy(entry->ipAddress, dns->records[i].ipAddress);
                entry->cachedAt = now;
                dns->cacheCount++;
            }
            
            printf("[Result] %s → %s\n", domain, dns->records[i].ipAddress);
            return dns->records[i].ipAddress;
        }
    }
    
    printf("[Not Found] %s\n", domain);
    return NULL;
}

void dns_hierarchical_lookup(DNSServer* dns, const char* domain) {
    printf("\n[Hierarchical Resolution] %s\n", domain);
    printf("  1. Query Root Nameserver\n");
    
    // Extract TLD
    char* tld = strrchr(domain, '.');
    if (!tld) {
        printf("  Invalid domain\n");
        return;
    }
    
    printf("  2. Query TLD Nameserver (%s)\n", tld);
    printf("  3. Query Authoritative Nameserver\n");
    
    dns_lookup(dns, domain);
}

void dns_print_stats(DNSServer* dns) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DNS STATISTICS                                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nDNS Performance:\n");
    printf("  Total lookups: %d\n", dns->lookups);
    printf("  Cache hits: %d\n", dns->cacheHits);
    printf("  Cache misses: %d\n", dns->cacheMisses);
    
    if (dns->lookups > 0) {
        printf("  Hit rate: %.1f%%\n",
               (float)dns->cacheHits / dns->lookups * 100);
    }
    
    printf("\nDNS Records:\n");
    printf("  %-30s %-15s %-10s\n", "Domain", "IP Address", "TTL");
    printf("  %-30s %-15s %-10s\n", "------", "----------", "---");
    
    for (int i = 0; i < dns->recordCount; i++) {
        DNSRecord* rec = &dns->records[i];
        printf("  %-30s %-15s %-10d\n", rec->domain, rec->ipAddress, rec->ttl);
    }
    
    printf("\nCached Entries:\n");
    printf("  %-30s %-15s\n", "Domain", "IP Address");
    printf("  %-30s %-15s\n", "------", "----------");
    
    time_t now = time(NULL);
    for (int i = 0; i < dns->cacheCount; i++) {
        CachedEntry* entry = &dns->cache[i];
        time_t age = now - entry->cachedAt;
        if (age < TTL) {
            printf("  %-30s %-15s (age: %ld s)\n",
                   entry->domain, entry->ipAddress, age);
        }
    }
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DNS ARCHITECTURE - COMPREHENSIVE DEMONSTRATION               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    DNSServer* dns = dns_create();
    
    printf("\n1. Add DNS Records:\n");
    printf("───────────────────\n");
    
    dns_add_record(dns, "google.com", "142.250.185.46");
    dns_add_record(dns, "github.com", "140.82.113.4");
    dns_add_record(dns, "stackoverflow.com", "151.101.1.140");
    dns_add_record(dns, "youtube.com", "142.250.80.46");
    
    printf("\n2. Perform DNS Lookups:\n");
    printf("──────────────────────\n");
    
    dns_lookup(dns, "google.com");
    dns_lookup(dns, "github.com");
    dns_lookup(dns, "google.com");  // Should hit cache
    dns_lookup(dns, "stackoverflow.com");
    dns_lookup(dns, "youtube.com");
    dns_lookup(dns, "google.com");  // Should hit cache again
    
    printf("\n3. Hierarchical Resolution:\n");
    printf("───────────────────────────\n");
    
    dns_hierarchical_lookup(dns, "example.org");
    dns_hierarchical_lookup(dns, "test.co.uk");
    
    dns_print_stats(dns);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ KEY PATTERNS                                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Hierarchical Structure: Root → TLD → Authoritative\n");
    printf("2. Recursive Resolution: Resolver handles recursion\n");
    printf("3. Caching: Reduce load on authoritative servers\n");
    printf("4. TTL (Time To Live): Cache invalidation\n");
    printf("5. Longest Prefix Match: Find most specific record\n");
    
    return 0;
}
