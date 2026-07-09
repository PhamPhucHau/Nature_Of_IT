/*
================================================================================
DIJKSTRA'S SHORTEST PATH ALGORITHM - JAVA VERSION
================================================================================

STORY:
------
A merchant needs to find the cheapest route from city A to city Z. He doesn't know
the full map, but he knows the cost to each neighboring city.

His strategy: Always move to the nearest unvisited city that he's already sure about.
Then expand outward. This is Dijkstra's algorithm.

NETWORK APPLICATION:
- OSPF (Open Shortest Path First) routing protocol
- GPS navigation systems
- Network QoS (Quality of Service) optimization
- SDN (Software Defined Networks) path computation
- Telecommunications network optimization

COMPLEXITY:
- Time: O((V + E) log V) with min-heap
- Space: O(V)
- V = vertices, E = edges

KEY INSIGHT:
Once we determine that a node has the shortest path,
we never need to reconsider it.
================================================================================
*/

import java.util.*;

public class Dijkstra {
    static final int INF = Integer.MAX_VALUE;
    static final int MAX_NODES = 100;
    
    static class NodeDistance implements Comparable<NodeDistance> {
        int node;
        int distance;
        
        NodeDistance(int node, int distance) {
            this.node = node;
            this.distance = distance;
        }
        
        // For PriorityQueue ordering
        @Override
        public int compareTo(NodeDistance other) {
            return Integer.compare(this.distance, other.distance);
        }
    }
    
    static class DijkstraResult {
        int[] distance;      // Shortest distance from source
        boolean[] visited;   // Track visited nodes
        int[] parent;        // For path reconstruction
        int nodeCount;
        
        DijkstraResult(int nodeCount) {
            this.nodeCount = nodeCount;
            this.distance = new int[nodeCount];
            this.visited = new boolean[nodeCount];
            this.parent = new int[nodeCount];
            
            // Initialize
            Arrays.fill(distance, INF);
            Arrays.fill(visited, false);
            Arrays.fill(parent, -1);
        }
    }
    
    /*
    DIJKSTRA ALGORITHM IMPLEMENTATION
    Finds shortest path from source to all other nodes
    */
    static void dijkstra(int[][] graph, int nodeCount, int source) {
        DijkstraResult result = new DijkstraResult(nodeCount);
        result.distance[source] = 0;
        
        // Min-heap priority queue
        PriorityQueue<NodeDistance> pq = new PriorityQueue<>();
        pq.add(new NodeDistance(source, 0));
        
        System.out.println("\n=== DIJKSTRA'S SHORTEST PATH ALGORITHM ===");
        System.out.println("Source node: " + source + "\n");
        System.out.println("Step-by-step execution:");
        System.out.println("--------------------------------------");
        
        int step = 1;
        
        // Main Dijkstra loop
        while (!pq.isEmpty()) {
            NodeDistance current = pq.poll();
            int u = current.node;
            
            // Skip if already visited
            if (result.visited[u]) {
                continue;
            }
            
            result.visited[u] = true;
            
            System.out.println("Step " + step + ": Visit node " + u + 
                             " (distance=" + result.distance[u] + ")");
            step++;
            
            // Check all neighbors
            for (int v = 0; v < nodeCount; v++) {
                if (graph[u][v] != 0 && !result.visited[v]) {
                    int newDistance = result.distance[u] + graph[u][v];
                    
                    // Relaxation step: if we found a shorter path
                    if (newDistance < result.distance[v]) {
                        result.distance[v] = newDistance;
                        result.parent[v] = u;
                        pq.add(new NodeDistance(v, newDistance));
                        System.out.println("  → Found better path to " + v + 
                                         ": " + newDistance + " (via " + u + ")");
                    }
                }
            }
        }
        
        // Print results
        printResults(result);
    }
    
    /*
    Print shortest distances and paths
    */
    static void printResults(DijkstraResult result) {
        System.out.println("\n=== RESULTS ===");
        System.out.println("Shortest distances from node " + 0 + ":");
        System.out.println("--------------------------------------");
        
        for (int i = 0; i < result.nodeCount; i++) {
            if (result.distance[i] == INF) {
                System.out.println("Node " + i + ": UNREACHABLE");
            } else {
                System.out.println("Node " + i + ": " + result.distance[i]);
            }
        }
        
        // Print paths
        System.out.println("\n=== PATHS ===");
        System.out.println("Path reconstruction:");
        System.out.println("--------------------------------------");
        
        for (int i = 0; i < result.nodeCount; i++) {
            if (i == 0) continue;
            
            if (result.distance[i] == INF) {
                System.out.println("Path to " + i + ": UNREACHABLE");
            } else {
                System.out.print("Path to " + i + " (distance=" + result.distance[i] + "): ");
                
                // Reconstruct path
                List<Integer> path = new ArrayList<>();
                int temp = i;
                
                while (temp != -1) {
                    path.add(temp);
                    temp = result.parent[temp];
                }
                
                // Print in reverse order
                Collections.reverse(path);
                for (int j = 0; j < path.size(); j++) {
                    System.out.print(path.get(j));
                    if (j < path.size() - 1) System.out.print(" → ");
                }
                System.out.println();
            }
        }
    }
    
    /*
    NETWORK APPLICATION EXAMPLE:
    ISP routing optimization - finding lowest latency paths
    */
    static void networkRoutingExample() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ NETWORK APPLICATION: ISP ROUTING OPTIMIZATION                 ║");
        System.out.println("║ Problem: Find lowest-latency route through ISP network        ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int nodes = 6;  // 6 routers: A(0), B(1), C(2), D(3), E(4), F(5)
        int[][] graph = {
            // A  B  C  D  E  F (distances in ms)
            { 0,  4,  2,  0,  0,  0},  // From A
            { 4,  0,  1,  5,  0,  0},  // From B
            { 2,  1,  0,  8,  10, 0},  // From C
            { 0,  5,  8,  0,  2,  6},  // From D
            { 0,  0,  10, 2,  0,  3},  // From E
            { 0,  0,  0,  6,  3,  0}   // From F
        };
        
        System.out.println("\nNetwork topology (latency in ms):");
        System.out.println("        A(0)");
        System.out.println("       / \\");
        System.out.println("      4   2");
        System.out.println("     /     \\");
        System.out.println("   B(1)--1--C(2)");
        System.out.println("    |       |");
        System.out.println("    5       8");
        System.out.println("    |       |");
        System.out.println("   D(3)-2--E(4)");
        System.out.println("    \\       /");
        System.out.println("     6     3");
        System.out.println("      \\   /");
        System.out.println("       F(5)");
        
        System.out.println("\nFinding optimal routes from Router A:");
        dijkstra(graph, nodes, 0);
        
        System.out.println("\n\nNETWORK INSIGHTS:");
        System.out.println("- Route to E: A→C→B→D→E = 2+1+5+2 = 10ms");
        System.out.println("- Route to F: A→C→B→D→F = 2+1+5+6 = 14ms");
        System.out.println("- Dijkstra ensures packets always take minimum-latency path");
        System.out.println("- Used by OSPF and IS-IS routing protocols");
    }
    
    /*
    Complexity analysis and comparison
    */
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nAlgorithm versions:");
        System.out.println("1. Array-based (find min with linear search):");
        System.out.println("   - Time: O(V²)");
        System.out.println("   - Space: O(V)");
        System.out.println("   - Good for: Dense graphs, small networks\n");
        
        System.out.println("2. Min-Heap based (this implementation):");
        System.out.println("   - Time: O((V+E) log V)");
        System.out.println("   - Space: O(V)");
        System.out.println("   - Good for: Sparse graphs, large networks\n");
        
        System.out.println("3. Fibonacci Heap (theoretical best):");
        System.out.println("   - Time: O(E + V log V)");
        System.out.println("   - Space: O(V)");
        System.out.println("   - Used in: Advanced networking research\n");
        
        System.out.println("For a network with 1000 routers:");
        System.out.println("- Array-based: ~1,000,000 operations");
        System.out.println("- Heap-based:  ~10,000 operations");
        System.out.println("- Fibonacci:   ~10,000 operations (slightly better constants)");
    }
    
    /*
    PRACTICAL USE CASE: QoS (Quality of Service) Routing
    */
    static void qosRoutingExample() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ PRACTICAL USE CASE: QoS ROUTING                               ║");
        System.out.println("║ Minimizing latency for VoIP traffic                           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nScenario: VoIP call routing");
        System.out.println("- Requirement: < 100ms latency");
        System.out.println("- Using Dijkstra to find minimum-latency path");
        System.out.println("\nWithout optimization:");
        System.out.println("  Random path: A→B→D→E = 4+5+2 = 11ms ✓ (within SLA)");
        System.out.println("\nWith Dijkstra optimization:");
        System.out.println("  Optimal path: A→C→E = 2+10 = 12ms ✓ (but uses different route)");
        System.out.println("  Best path: A→C→B→D→E = 2+1+5+2 = 10ms ✓ (guaranteed minimum)");
        System.out.println("\nBenefit: Consistent QoS, better call quality");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DIJKSTRA'S SHORTEST PATH ALGORITHM - JAVA IMPLEMENTATION      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        // Simple example
        int[][] simpleGraph = {
            // 0  1  2  3  4
            { 0,  7,  9,  0,  0},  // From 0
            { 7,  0,  10, 15, 0},  // From 1
            { 9,  10, 0,  11, 2},  // From 2
            { 0,  15, 11, 0,  6},  // From 3
            { 0,  0,  2,  6,  0}   // From 4
        };
        
        dijkstra(simpleGraph, 5, 0);
        
        // Network example
        networkRoutingExample();
        
        // QoS example
        qosRoutingExample();
        
        // Complexity analysis
        complexityAnalysis();
        
        System.out.println("\n\nKEY TAKEAWAYS:");
        System.out.println("1. Dijkstra finds shortest paths efficiently");
        System.out.println("2. Greedy approach works: always pick closest known node");
        System.out.println("3. Cannot handle negative weights");
        System.out.println("4. Foundation for routing protocols (OSPF)");
        System.out.println("5. Essential for network optimization and QoS");
    }
}
