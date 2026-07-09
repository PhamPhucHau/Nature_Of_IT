/*
================================================================================
GRAPH DATA STRUCTURE - JAVA VERSION
================================================================================

STORY:
------
Cities are connected by roads.

To navigate, you need to know:
- Which cities are connected?
- What are the distances?
- What's the shortest route?

A Graph answers these questions.

Each city is a node. Each road is an edge.

In networking: nodes are routers, edges are links.

NETWORK APPLICATION:
- Network topology representation
- Router connectivity mapping
- Link state databases
- Network discovery (OSPF, ISIS)
- Social network analysis
- Content delivery networks (CDN)
- Peer-to-peer networks

COMPLEXITY:
- Adjacency Matrix: O(V²) space, O(1) edge lookup
- Adjacency List: O(V+E) space, O(degree) edge lookup
- BFS/DFS: O(V+E) time

KEY INSIGHT:
Graphs are the most general data structure.
Most complex networks can be modeled as graphs.
Choice of representation (matrix vs list) matters for performance.
================================================================================
*/

import java.util.*;

public class GraphDS {
    
    /*
    UNDIRECTED GRAPH using Adjacency List
    */
    static class Graph {
        private int vertices;
        private Map<Integer, List<Integer>> adjacencyList;
        private Map<Integer, Map<Integer, Integer>> weights;  // For weighted edges
        
        Graph(int vertices) {
            this.vertices = vertices;
            this.adjacencyList = new HashMap<>();
            this.weights = new HashMap<>();
            
            for (int i = 0; i < vertices; i++) {
                adjacencyList.put(i, new ArrayList<>());
                weights.put(i, new HashMap<>());
            }
        }
        
        // Add edge (undirected)
        void addEdge(int u, int v, int weight) {
            adjacencyList.get(u).add(v);
            adjacencyList.get(v).add(u);
            weights.get(u).put(v, weight);
            weights.get(v).put(u, weight);
        }
        
        void addEdge(int u, int v) {
            addEdge(u, v, 1);  // Default weight
        }
        
        // Get neighbors
        List<Integer> getNeighbors(int node) {
            return adjacencyList.get(node);
        }
        
        int getWeight(int u, int v) {
            return weights.get(u).getOrDefault(v, 0);
        }
        
        int getVertices() {
            return vertices;
        }
    }
    
    /*
    BREADTH-FIRST SEARCH (BFS)
    Explore nodes layer by layer
    */
    static void bfs(Graph graph, int start) {
        System.out.println("\n=== BFS (Breadth-First Search) ===");
        System.out.println("Starting from node: " + start + "\n");
        
        boolean[] visited = new boolean[graph.getVertices()];
        Queue<Integer> queue = new LinkedList<>();
        
        queue.add(start);
        visited[start] = true;
        
        int level = 0;
        int levelSize = 1;
        int nextLevelSize = 0;
        
        System.out.println("Traversal order:");
        System.out.print("Level 0: " + start);
        
        while (!queue.isEmpty()) {
            int node = queue.poll();
            levelSize--;
            
            for (Integer neighbor : graph.getNeighbors(node)) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.add(neighbor);
                    nextLevelSize++;
                    System.out.print(" → " + neighbor);
                }
            }
            
            if (levelSize == 0 && !queue.isEmpty()) {
                level++;
                levelSize = nextLevelSize;
                nextLevelSize = 0;
                System.out.print("\nLevel " + level + ": ");
            }
        }
        System.out.println();
    }
    
    /*
    DEPTH-FIRST SEARCH (DFS)
    Explore as deep as possible before backtracking
    */
    static void dfs(Graph graph, int start) {
        System.out.println("\n=== DFS (Depth-First Search) ===");
        System.out.println("Starting from node: " + start + "\n");
        
        boolean[] visited = new boolean[graph.getVertices()];
        List<Integer> order = new ArrayList<>();
        
        dfsHelper(graph, start, visited, order);
        
        System.out.println("Traversal order:");
        for (int i = 0; i < order.size(); i++) {
            System.out.print(order.get(i));
            if (i < order.size() - 1) System.out.print(" → ");
        }
        System.out.println();
    }
    
    static void dfsHelper(Graph graph, int node, boolean[] visited, List<Integer> order) {
        visited[node] = true;
        order.add(node);
        
        for (Integer neighbor : graph.getNeighbors(node)) {
            if (!visited[neighbor]) {
                dfsHelper(graph, neighbor, visited, order);
            }
        }
    }
    
    /*
    CONNECTED COMPONENTS
    Find separate clusters in graph
    */
    static void findConnectedComponents(Graph graph) {
        System.out.println("\n=== Connected Components ===");
        
        boolean[] visited = new boolean[graph.getVertices()];
        int componentCount = 0;
        
        for (int i = 0; i < graph.getVertices(); i++) {
            if (!visited[i]) {
                componentCount++;
                System.out.print("Component " + componentCount + ": ");
                List<Integer> component = new ArrayList<>();
                dfsComponent(graph, i, visited, component);
                
                for (int node : component) {
                    System.out.print(node + " ");
                }
                System.out.println();
            }
        }
        
        System.out.println("Total components: " + componentCount);
    }
    
    static void dfsComponent(Graph graph, int node, boolean[] visited, List<Integer> component) {
        visited[node] = true;
        component.add(node);
        
        for (Integer neighbor : graph.getNeighbors(node)) {
            if (!visited[neighbor]) {
                dfsComponent(graph, neighbor, visited, component);
            }
        }
    }
    
    /*
    NETWORK TOPOLOGY: Routers connected by links
    */
    static void networkTopologyDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: NETWORK TOPOLOGY (Router Connectivity)            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Graph network = new Graph(6);  // 6 routers
        
        System.out.println("\nBuilding network topology:");
        System.out.println("Router 0 ↔ Router 1 (10 ms)");
        System.out.println("Router 1 ↔ Router 2 (15 ms)");
        System.out.println("Router 2 ↔ Router 3 (20 ms)");
        System.out.println("Router 3 ↔ Router 4 (10 ms)");
        System.out.println("Router 4 ↔ Router 5 (15 ms)");
        System.out.println("Router 5 ↔ Router 0 (25 ms)\n");
        
        network.addEdge(0, 1, 10);
        network.addEdge(1, 2, 15);
        network.addEdge(2, 3, 20);
        network.addEdge(3, 4, 10);
        network.addEdge(4, 5, 15);
        network.addEdge(5, 0, 25);
        
        bfs(network, 0);
        dfs(network, 0);
        
        System.out.println("\nRouter neighbors:");
        for (int i = 0; i < 6; i++) {
            System.out.print("Router " + i + " connected to: ");
            for (Integer neighbor : network.getNeighbors(i)) {
                System.out.print(neighbor + " ");
            }
            System.out.println();
        }
    }
    
    /*
    GRAPH PROPERTIES
    */
    static void graphPropertiesDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: GRAPH PROPERTIES                                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Graph g = new Graph(5);
        
        g.addEdge(0, 1);
        g.addEdge(0, 2);
        g.addEdge(1, 2);
        g.addEdge(1, 3);
        g.addEdge(3, 4);
        
        System.out.println("\nGraph edges: (0-1), (0-2), (1-2), (1-3), (3-4)");
        System.out.println("\nDegree of each node:");
        for (int i = 0; i < 5; i++) {
            System.out.println("  Node " + i + ": degree " + g.getNeighbors(i).size());
        }
        
        int totalEdges = 0;
        for (int i = 0; i < 5; i++) {
            totalEdges += g.getNeighbors(i).size();
        }
        totalEdges /= 2;  // Each edge counted twice
        System.out.println("\nTotal edges: " + totalEdges);
        System.out.println("Total nodes: " + g.getVertices());
        
        findConnectedComponents(g);
    }
    
    /*
    DIRECTED GRAPH
    */
    static void directedGraphDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: DIRECTED GRAPH (Flow Networks)                  ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nDirected graph example: Packet flow");
        System.out.println("Router A → Router B (bandwidth: 100 Mbps)");
        System.out.println("Router B → Router C (bandwidth: 50 Mbps)");
        System.out.println("Router C → Router D (bandwidth: 75 Mbps)");
        System.out.println("Router B → Router D (bandwidth: 150 Mbps)\n");
        
        System.out.println("In a directed graph:");
        System.out.println("- Edge (A, B) ≠ Edge (B, A)");
        System.out.println("- Useful for: packet routing, call flows, dependencies");
    }
    
    /*
    GRAPH REPRESENTATION COMPARISON
    */
    static void representationComparison() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ GRAPH REPRESENTATIONS                                       ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\n1. Adjacency Matrix:");
        System.out.println("   Space: O(V²)");
        System.out.println("   Edge lookup: O(1)");
        System.out.println("   Best for: Dense graphs, all-pairs operations\n");
        
        System.out.println("2. Adjacency List:");
        System.out.println("   Space: O(V + E)");
        System.out.println("   Edge lookup: O(degree)");
        System.out.println("   Best for: Sparse graphs, BFS/DFS\n");
        
        System.out.println("3. Edge List:");
        System.out.println("   Space: O(E)");
        System.out.println("   Edge lookup: O(E)");
        System.out.println("   Best for: Algorithm on all edges\n");
        
        System.out.println("Network topology usually uses Adjacency List (sparse)");
    }
    
    /*
    COMPLEXITY ANALYSIS
    */
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nGraph Algorithms:");
        System.out.println("- BFS: O(V + E)");
        System.out.println("- DFS: O(V + E)");
        System.out.println("- Dijkstra: O((V+E) log V)");
        System.out.println("- Floyd-Warshall: O(V³)");
        System.out.println("- Topological Sort: O(V + E)\n");
        
        System.out.println("Real network example (1000 routers):");
        System.out.println("- V = 1000, E ≈ 3000 (sparse)");
        System.out.println("- BFS: ~4000 operations");
        System.out.println("- DFS: ~4000 operations");
        System.out.println("- Floyd-Warshall: ~1 billion operations (too slow!)");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ GRAPH DATA STRUCTURE - JAVA COMPREHENSIVE DEMONSTRATION      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nGraph: Set of nodes connected by edges");
        System.out.println("Perfect model for: networks, social graphs, dependencies\n");
        
        networkTopologyDemo();
        graphPropertiesDemo();
        directedGraphDemo();
        representationComparison();
        complexityAnalysis();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Graphs model complex relationships");
        System.out.println("2. Adjacency list: preferred for sparse networks");
        System.out.println("3. BFS/DFS: O(V+E) traversal");
        System.out.println("4. Foundation for Dijkstra, routing algorithms");
        System.out.println("5. Essential for network topology representation");
    }
}
