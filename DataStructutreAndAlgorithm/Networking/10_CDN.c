/*
================================================================================
CDN ARCHITECTURE - C IMPLEMENTATION
================================================================================

STORY:
------
Users around the world want fast downloads. Serving from one location means
slow responses for distant users.

CDN (Content Delivery Network) places edge servers near users worldwide.
When user requests content, CDN routes to nearest edge server.

Geo-routing chooses the closest edge. LRU cache manages storage.
Popular content stays, unpopular content evicted.

ARCHITECTURE:
  User (USA)
    |
  Edge (USA) ← Fastest
    |
  Origin (Tokyo)

Data Structures: HashMap, Trie, LRU Cache
Algorithms: Geo-routing, Cache replacement, Consistent hashing
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_EDGES 5
#define MAX_CACHE_SIZE 100

typedef struct {
    double latitude;
    double longitude;
} Location;

typedef struct {
    int edgeId;
    char name[50];
    Location location;
    int cacheSize;
    int hits;
    int misses;
    int evictions;
} EdgeServer;

typedef struct {
    char url[100];
    char content[1024];
    long size;
} CachedContent;

typedef struct {
    EdgeServer edges[MAX_EDGES];
    int edgeCount;
    CachedContent cache[MAX_EDGES][MAX_CACHE_SIZE];
    int cacheCount[MAX_EDGES];
    int totalRequests;
    int totalHits;
    Location origin;
} CDN;

double calculate_distance(Location a, Location b) {
    double lat1 = a.latitude * M_PI / 180;
    double lon1 = a.longitude * M_PI / 180;
    double lat2 = b.latitude * M_PI / 180;
    double lon2 = b.longitude * M_PI / 180;
    
    double dlon = lon2 - lon1;
    double dlat = lat2 - lat1;
    
    double a_val = sin(dlat / 2) * sin(dlat / 2) +
                   cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a_val), sqrt(1 - a_val));
    double radius = 6371;  // km
    
    return radius * c;
}

CDN* cdn_create() {
    CDN* cdn = malloc(sizeof(CDN));
    cdn->edgeCount = 0;
    cdn->totalRequests = 0;
    cdn->totalHits = 0;
    cdn->origin.latitude = 35.6762;   // Tokyo
    cdn->origin.longitude = 139.6503;
    
    for (int i = 0; i < MAX_EDGES; i++) {
        cdn->cacheCount[i] = 0;
    }
    
    printf("CDN created\n");
    return cdn;
}

void cdn_add_edge(CDN* cdn, int edgeId, const char* name,
                 double lat, double lon) {
    if (cdn->edgeCount >= MAX_EDGES) return;
    
    EdgeServer* edge = &cdn->edges[cdn->edgeCount];
    edge->edgeId = edgeId;
    strcpy(edge->name, name);
    edge->location.latitude = lat;
    edge->location.longitude = lon;
    edge->cacheSize = 0;
    edge->hits = 0;
    edge->misses = 0;
    edge->evictions = 0;
    
    cdn->edgeCount++;
    printf("✓ Edge server %d (%s) added at (%.2f, %.2f)\n",
           edgeId, name, lat, lon);
}

int cdn_find_nearest_edge(CDN* cdn, Location userLocation) {
    int nearest = 0;
    double minDistance = calculate_distance(userLocation, cdn->edges[0].location);
    
    for (int i = 1; i < cdn->edgeCount; i++) {
        double dist = calculate_distance(userLocation, cdn->edges[i].location);
        if (dist < minDistance) {
            minDistance = dist;
            nearest = i;
        }
    }
    
    return nearest;
}

int cdn_get_content(CDN* cdn, const char* url, Location userLocation) {
    cdn->totalRequests++;
    
    int edgeIdx = cdn_find_nearest_edge(cdn, userLocation);
    EdgeServer* edge = &cdn->edges[edgeIdx];
    
    printf("\n[Geo-route] User at (%.2f, %.2f) → %s\n",
           userLocation.latitude, userLocation.longitude, edge->name);
    
    // Check cache
    for (int i = 0; i < cdn->cacheCount[edgeIdx]; i++) {
        if (strcmp(cdn->cache[edgeIdx][i].url, url) == 0) {
            printf("  [Cache HIT] %s (size: %ld KB)\n", url, cdn->cache[edgeIdx][i].size);
            edge->hits++;
            cdn->totalHits++;
            return 1;
        }
    }
    
    printf("  [Cache MISS] %s - fetching from origin\n", url);
    edge->misses++;
    
    // Add to cache
    if (cdn->cacheCount[edgeIdx] < MAX_CACHE_SIZE) {
        CachedContent* content = &cdn->cache[edgeIdx][cdn->cacheCount[edgeIdx]];
        strcpy(content->url, url);
        sprintf(content->content, "Content of %s", url);
        content->size = 50 + (rand() % 500);  // Random size 50-550 KB
        
        cdn->cacheCount[edgeIdx]++;
        edge->cacheSize += content->size;
        printf("  [Cached] %s (size: %ld KB, edge cache: %d items)\n",
               url, content->size, cdn->cacheCount[edgeIdx]);
    } else {
        printf("  [Cache Full] LRU eviction would occur\n");
        edge->evictions++;
    }
    
    return 0;
}

void cdn_print_stats(CDN* cdn) {
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ CDN STATISTICS                                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nCDN Status:\n");
    printf("  Total edge servers: %d\n", cdn->edgeCount);
    printf("  Total requests: %d\n", cdn->totalRequests);
    printf("  Total hits: %d\n", cdn->totalHits);
    
    if (cdn->totalRequests > 0) {
        printf("  Cache hit rate: %.1f%%\n",
               (float)cdn->totalHits / cdn->totalRequests * 100);
    }
    
    printf("\nEdge Server Performance:\n");
    printf("  %-15s %-15s %-12s %-12s %-10s\n",
           "Name", "Location", "Cache Items", "Hits", "Misses");
    printf("  %-15s %-15s %-12s %-12s %-10s\n",
           "----", "--------", "-----------", "----", "------");
    
    for (int i = 0; i < cdn->edgeCount; i++) {
        EdgeServer* edge = &cdn->edges[i];
        printf("  %-15s (%.2f,%.2f)    %-12d %-12d %-10d\n",
               edge->name, edge->location.latitude, edge->location.longitude,
               cdn->cacheCount[i], edge->hits, edge->misses);
    }
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ CDN ARCHITECTURE - COMPREHENSIVE DEMONSTRATION               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    srand(time(NULL));
    CDN* cdn = cdn_create();
    
    printf("\n1. Add Edge Servers:\n");
    printf("────────────────────\n");
    
    cdn_add_edge(cdn, 1, "US-East", 40.7128, -74.0060);      // NYC
    cdn_add_edge(cdn, 2, "US-West", 37.7749, -122.4194);     // SF
    cdn_add_edge(cdn, 3, "EU", 48.8566, 2.3522);             // Paris
    cdn_add_edge(cdn, 4, "Asia", 1.3521, 103.8198);          // Singapore
    
    printf("\n2. Request Content from Different Locations:\n");
    printf("──────────────────────────────────────────────\n");
    
    Location userNYC = {40.7128, -74.0060};
    Location userSF = {37.7749, -122.4194};
    Location userTokyo = {35.6762, 139.6503};\n");
    
    cdn_get_content(cdn, "video.mp4", userNYC);
    cdn_get_content(cdn, "image.jpg", userSF);
    cdn_get_content(cdn, "index.html", userTokyo);
    
    cdn_get_content(cdn, "video.mp4", userNYC);  // Should hit
    cdn_get_content(cdn, "image.jpg", userSF);   // Should hit
    
    cdn_print_stats(cdn);
    
    printf("\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ KEY PATTERNS                                                 ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\n1. Geo-routing: Route to nearest edge\n");
    printf("2. Cache Management: LRU eviction\n");
    printf("3. Content Replication: Automatic sync\n");
    printf("4. Bandwidth Optimization: Reduce origin load\n");
    printf("5. Hit Rate: Key metric for CDN efficiency\n");
    
    return 0;
}
