/*
================================================================================
DIJKSTRA'S SHORTEST PATH ALGORITHM
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
- Time: O((V + E) log V) with min-heap, O(V²) with array
- Space: O(V)
- V = vertices (routers/nodes), E = edges (links)

KEY INSIGHT:
Once we determine that a node has the shortest path to destination,
we never need to reconsider it.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 100
#define INF INT_MAX

typedef struct {
    int distance[MAX_NODES];  // Shortest distance from source
    int visited[MAX_NODES];   // Track visited nodes
    int parent[MAX_NODES];    // For path reconstruction
    int node_count;
} DijkstraState;

typedef struct {
    int node;
    int distance;
} NodeDist;

// Simple min-heap for priority queue
typedef struct {
    NodeDist heap[MAX_NODES];
    int size;
} MinHeap;

void heap_push(MinHeap* h, int node, int dist) {
    h->heap[h->size].node = node;
    h->heap[h->size].distance = dist;
    h->size++;
    
    // Bubble up
    int idx = h->size - 1;
    while (idx > 0 && h->heap[idx].distance < h->heap[(idx - 1) / 2].distance) {
        NodeDist temp = h->heap[idx];
        h->heap[idx] = h->heap[(idx - 1) / 2];
        h->heap[(idx - 1) / 2] = temp;
        idx = (idx - 1) / 2;
    }
}

NodeDist heap_pop(MinHeap* h) {
    NodeDist result = h->heap[0];
    h->heap[0] = h->heap[h->size - 1];
    h->size--;
    
    // Bubble down
    int idx = 0;
    while (2 * idx + 1 < h->size) {
        int smallest = idx;
        if (h->heap[2 * idx + 1].distance < h->heap[smallest].distance) {
            smallest = 2 * idx + 1;
        }
        if (2 * idx + 2 < h->size && h->heap[2 * idx + 2].distance < h->heap[smallest].distance) {
            smallest = 2 * idx + 2;
        }
        if (smallest != idx) {
            NodeDist temp = h->heap[idx];
            h->heap[idx] = h->heap[smallest];
            h->heap[smallest] = temp;
            idx = smallest;
        } else {
            break;
        }
    }
    return result;
}

/*
DIJKSTRA ALGORITHM IMPLEMENTATION
This finds the shortest path from source to all other nodes
*/
void dijkstra(int graph[MAX_NODES][MAX_NODES], int node_count, int source) {
    DijkstraState state;
    state.node_count = node_count;
    
    // Initialize distances and visited array
    for (int i = 0; i < node_count; i++) {
        state.distance[i] = INF;
        state.visited[i] = 0;
        state.parent[i] = -1;
    }
    
    state.distance[source] = 0;
    
    // Min heap priority queue
    MinHeap heap;
    heap.size = 0;
    heap_push(&heap, source, 0);
    
    printf("\n=== DIJKSTRA'S SHORTEST PATH ALGORITHM ===\n");
    printf("Source node: %d\n\n", source);
    printf("Step-by-step execution:\n");
    printf("--------------------------------------\n");
    
    int step = 1;
    
    // Main Dijkstra loop
    while (heap.size > 0) {
        NodeDist current = heap_pop(&heap);
        int u = current.node;
        
        // Skip if already visited
        if (state.visited[u]) {
            continue;
        }
        
        state.visited[u] = 1;
        
        printf("Step %d: Visit node %d (distance=%d)\n", step++, u, state.distance[u]);
        
        // Check all neighbors
        for (int v = 0; v < node_count; v++) {
            if (graph[u][v] != 0 && !state.visited[v]) {
                int new_distance = state.distance[u] + graph[u][v];
                
                // Relaxation step: if we found a shorter path
                if (new_distance < state.distance[v]) {
                    state.distance[v] = new_distance;
                    state.parent[v] = u;
                    heap_push(&heap, v, new_distance);
                    printf("  → Found better path to %d: %d (via %d)\n", 
                           v, new_distance, u);
                }
            }
        }
    }
    
    // Print results
    printf("\n=== RESULTS ===\n");
    printf("Shortest distances from node %d:\n", source);
    printf("--------------------------------------\n");
    for (int i = 0; i < node_count; i++) {
        if (state.distance[i] == INF) {
            printf("Node %d: UNREACHABLE\n", i);
        } else {
            printf("Node %d: %d\n", i, state.distance[i]);
        }
    }
    
    // Print paths
    printf("\n=== PATHS ===\n");
    printf("Path reconstruction from source %d:\n", source);
    printf("--------------------------------------\n");
    
    for (int i = 0; i < node_count; i++) {
        if (i == source) continue;
        
        if (state.distance[i] == INF) {
            printf("Path to %d: UNREACHABLE\n", i);
        } else {
            printf("Path to %d (distance=%d): ", i, state.distance[i]);
            
            // Reconstruct path
            int temp = i;
            int path[MAX_NODES];
            int path_len = 0;
            
            while (temp != -1) {
                path[path_len++] = temp;
                temp = state.parent[temp];
            }
            
            // Print in reverse order
            for (int j = path_len - 1; j >= 0; j--) {
                printf("%d", path[j]);
                if (j > 0) printf(" → ");
            }
            printf("\n");
        }
    }
}

/*
NETWORK APPLICATION EXAMPLE:
Network routing - finding shortest path between routers
*/
void network_routing_example() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ NETWORK APPLICATION: ISP ROUTING OPTIMIZATION                 ║\n");
    printf("║ Problem: Find lowest-latency route through ISP network        ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int nodes = 6;  // 6 routers: A(0), B(1), C(2), D(3), E(4), F(5)
    int graph[6][6] = {
        // A  B  C  D  E  F (distances in ms)
        { 0,  4,  2,  0,  0,  0},  // From A
        { 4,  0,  1,  5,  0,  0},  // From B
        { 2,  1,  0,  8,  10, 0},  // From C
        { 0,  5,  8,  0,  2,  6},  // From D
        { 0,  0,  10, 2,  0,  3},  // From E
        { 0,  0,  0,  6,  3,  0}   // From F
    };
    
    printf("\nNetwork topology (latency in ms):\n");
    printf("        A(0)\n");
    printf("       / \\\n");
    printf("      4   2\n");
    printf("     /     \\\n");
    printf("   B(1)--1--C(2)\n");
    printf("    |       |\n");
    printf("    5       8\n");
    printf("    |       |\n");
    printf("   D(3)-2--E(4)\n");
    printf("    \\       /\n");
    printf("     6     3\n");
    printf("      \\   /\n");
    printf("       F(5)\n");
    
    printf("\nFinding optimal routes from Router A:\n");
    dijkstra(graph, nodes, 0);
    
    printf("\n\nNETWORK INSIGHTS:\n");
    printf("- Route to E: A→C→B→D→E = 2+1+5+2 = 10ms\n");
    printf("- Route to F: A→C→B→D→F = 2+1+5+6 = 14ms\n");
    printf("- Dijkstra ensures packets always take minimum-latency path\n");
    printf("- Used by OSPF and IS-IS routing protocols\n");
}

/*
COMPARISON: Array-based vs Heap-based complexity
*/
void complexity_analysis() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nAlgorithm versions:\n");
    printf("1. Array-based (find min with linear search):\n");
    printf("   - Time: O(V²)\n");
    printf("   - Space: O(V)\n");
    printf("   - Good for: Dense graphs, small networks\n\n");
    
    printf("2. Min-Heap based (this implementation):\n");
    printf("   - Time: O((V+E) log V)\n");
    printf("   - Space: O(V)\n");
    printf("   - Good for: Sparse graphs, large networks\n\n");
    
    printf("3. Fibonacci Heap (theoretical best):\n");
    printf("   - Time: O(E + V log V)\n");
    printf("   - Space: O(V)\n");
    printf("   - Used in: Advanced networking research\n\n");
    
    printf("For a network with 1000 routers:\n");
    printf("- Array-based: ~1,000,000 operations\n");
    printf("- Heap-based:  ~10,000 operations\n");
    printf("- Fibonacci:   ~10,000 operations (slightly better constants)\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DIJKSTRA'S SHORTEST PATH ALGORITHM DEMONSTRATION              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    // Simple example
    int simple_nodes = 5;
    int simple_graph[5][5] = {
        // 0  1  2  3  4
        { 0,  7,  9,  0,  0},  // From 0
        { 7,  0,  10, 15, 0},  // From 1
        { 9,  10, 0,  11, 2},  // From 2
        { 0,  15, 11, 0,  6},  // From 3
        { 0,  0,  2,  6,  0}   // From 4
    };
    
    dijkstra(simple_graph, simple_nodes, 0);
    
    // Network example
    network_routing_example();
    
    // Complexity analysis
    complexity_analysis();
    
    printf("\n\nKEY TAKEAWAYS:\n");
    printf("1. Dijkstra finds shortest paths efficiently\n");
    printf("2. Greedy approach works: always pick closest known node\n");
    printf("3. Cannot handle negative weights\n");
    printf("4. Foundation for routing protocols (OSPF)\n");
    printf("5. Essential for network optimization\n");
    
    return 0;
}
