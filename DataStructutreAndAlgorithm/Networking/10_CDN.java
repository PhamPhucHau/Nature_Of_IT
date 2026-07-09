import java.util.*;

/**
 * CDN ARCHITECTURE - JAVA IMPLEMENTATION
 */

public class CDN {
    
    static class Location {
        double latitude;
        double longitude;
        
        Location(double lat, double lon) {
            this.latitude = lat;
            this.longitude = lon;
        }
    }
    
    static class EdgeServer {
        int edgeId;
        String name;
        Location location;
        int hits;
        int misses;
        Map<String, String> cache;
        
        EdgeServer(int id, String name, double lat, double lon) {
            this.edgeId = id;
            this.name = name;
            this.location = new Location(lat, lon);
            this.hits = 0;
            this.misses = 0;
            this.cache = new LinkedHashMap<>();
        }
    }
    
    static class CDNCluster {
        List<EdgeServer> edges;
        int totalRequests;
        int totalHits;
        Location origin;
        
        CDNCluster() {
            this.edges = new ArrayList<>();
            this.totalRequests = 0;
            this.totalHits = 0;
            this.origin = new Location(35.6762, 139.6503);  // Tokyo
            System.out.println("CDN created");
        }
        
        void addEdge(int id, String name, double lat, double lon) {
            edges.add(new EdgeServer(id, name, lat, lon));
            System.out.println("✓ Edge server " + id + " (" + name + 
                             ") added at (" + lat + ", " + lon + ")");
        }
        
        double calculateDistance(Location a, Location b) {
            double lat1 = Math.toRadians(a.latitude);
            double lon1 = Math.toRadians(a.longitude);
            double lat2 = Math.toRadians(b.latitude);
            double lon2 = Math.toRadians(b.longitude);
            
            double dlon = lon2 - lon1;
            double dlat = lat2 - lat1;
            
            double aVal = Math.sin(dlat / 2) * Math.sin(dlat / 2) +
                         Math.cos(lat1) * Math.cos(lat2) * 
                         Math.sin(dlon / 2) * Math.sin(dlon / 2);
            double c = 2 * Math.atan2(Math.sqrt(aVal), Math.sqrt(1 - aVal));
            
            return 6371 * c;  // km
        }
        
        int findNearestEdge(Location userLocation) {
            int nearest = 0;
            double minDistance = calculateDistance(userLocation, edges.get(0).location);
            
            for (int i = 1; i < edges.size(); i++) {
                double dist = calculateDistance(userLocation, edges.get(i).location);
                if (dist < minDistance) {
                    minDistance = dist;
                    nearest = i;
                }
            }
            
            return nearest;
        }
        
        boolean getContent(String url, Location userLocation) {
            totalRequests++;
            
            int edgeIdx = findNearestEdge(userLocation);
            EdgeServer edge = edges.get(edgeIdx);
            
            System.out.println("\n[Geo-route] User at (" + userLocation.latitude + 
                             ", " + userLocation.longitude + ") → " + edge.name);
            
            if (edge.cache.containsKey(url)) {
                System.out.println("  [Cache HIT] " + url);
                edge.hits++;
                totalHits++;
                return true;
            }
            
            System.out.println("  [Cache MISS] " + url + " - fetching from origin");
            edge.misses++;
            
            if (edge.cache.size() < 100) {
                edge.cache.put(url, "Content of " + url);
                System.out.println("  [Cached] " + url + " (cache size: " + 
                                 edge.cache.size() + " items)");
            }
            
            return false;
        }
        
        void printStats() {
            System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
            System.out.println("║ CDN STATISTICS                                               ║");
            System.out.println("╚════════════════════════════════════════════════════════════════╝");
            
            System.out.println("\nCDN Status:");
            System.out.println("  Total edge servers: " + edges.size());
            System.out.println("  Total requests: " + totalRequests);
            System.out.println("  Total hits: " + totalHits);
            
            if (totalRequests > 0) {
                System.out.println("  Cache hit rate: " + 
                                 String.format("%.1f%%", (float)totalHits / 
                                 totalRequests * 100));
            }
            
            System.out.println("\nEdge Server Performance:");
            System.out.println(String.format("  %-15s %-15s %-12s %-12s",
                                           "Name", "Location", "Cache", "Hits"));
            System.out.println(String.format("  %-15s %-15s %-12s %-12s",
                                           "----", "--------", "-----", "----"));
            
            for (EdgeServer edge : edges) {
                System.out.println(String.format("  %-15s (%.2f,%.2f)   %-12d %-12d",
                                                edge.name, edge.location.latitude,
                                                edge.location.longitude,
                                                edge.cache.size(), edge.hits));
            }
        }
    }
    
    static void demo() {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ CDN ARCHITECTURE - COMPREHENSIVE DEMONSTRATION               ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        CDNCluster cdn = new CDNCluster();
        
        System.out.println("\n1. Add Edge Servers:");
        System.out.println("────────────────────");
        
        cdn.addEdge(1, "US-East", 40.7128, -74.0060);
        cdn.addEdge(2, "US-West", 37.7749, -122.4194);
        cdn.addEdge(3, "EU", 48.8566, 2.3522);
        cdn.addEdge(4, "Asia", 1.3521, 103.8198);
        
        System.out.println("\n2. Request Content:");
        System.out.println("──────────────────");
        
        Location userNYC = new Location(40.7128, -74.0060);
        Location userSF = new Location(37.7749, -122.4194);
        Location userTokyo = new Location(35.6762, 139.6503);
        
        cdn.getContent("video.mp4", userNYC);
        cdn.getContent("image.jpg", userSF);
        cdn.getContent("index.html", userTokyo);
        cdn.getContent("video.mp4", userNYC);
        cdn.getContent("image.jpg", userSF);
        
        cdn.printStats();
    }
    
    public static void main(String[] args) {
        demo();
        
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ KEY PATTERNS                                                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Geo-routing: Route to nearest edge");
        System.out.println("2. Cache Management: LRU eviction");
        System.out.println("3. Content Replication: Automatic sync");
        System.out.println("4. Bandwidth Optimization: Reduce origin");
        System.out.println("5. Hit Rate: Key efficiency metric");
    }
}
