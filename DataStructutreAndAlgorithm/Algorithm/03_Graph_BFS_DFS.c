#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NODES 10

typedef struct {
    int adj[MAX_NODES][MAX_NODES];
    int numNodes;
} Graph;

typedef struct {
    int data[MAX_NODES];
    int front, rear;
} Queue;

Graph* createGraph(int n) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    g->numNodes = n;
    memset(g->adj, 0, sizeof(g->adj));
    return g;
}

void addEdge(Graph *g, int u, int v) {
    g->adj[u][v] = 1;
    g->adj[v][u] = 1;
}

Queue* createQueue() {
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->front = q->rear = -1;
    return q;
}

void enqueue(Queue *q, int val) {
    if (q->rear < MAX_NODES - 1) {
        if (q->front == -1) q->front = 0;
        q->data[++q->rear] = val;
    }
}

int dequeue(Queue *q) {
    if (q->front <= q->rear) {
        return q->data[q->front++];
    }
    return -1;
}

void BFS(Graph *g, int start) {
    int visited[MAX_NODES] = {0};
    Queue *q = createQueue();
    
    printf("BFS from node %d: ", start);
    visited[start] = 1;
    enqueue(q, start);
    
    while (q->front <= q->rear) {
        int node = dequeue(q);
        printf("%d ", node);
        
        for (int i = 0; i < g->numNodes; i++) {
            if (g->adj[node][i] && !visited[i]) {
                visited[i] = 1;
                enqueue(q, i);
            }
        }
    }
    printf("\n");
    free(q);
}

void DFSUtil(Graph *g, int node, int visited[]) {
    printf("%d ", node);
    visited[node] = 1;
    
    for (int i = 0; i < g->numNodes; i++) {
        if (g->adj[node][i] && !visited[i]) {
            DFSUtil(g, i, visited);
        }
    }
}

void DFS(Graph *g, int start) {
    int visited[MAX_NODES] = {0};
    printf("DFS from node %d: ", start);
    DFSUtil(g, start, visited);
    printf("\n");
}

int main() {
    printf("===== GRAPH: BFS & DFS =====\n\n");
    
    Graph *g = createGraph(5);
    
    printf("--- Building graph ---\n");
    addEdge(g, 0, 1);
    addEdge(g, 0, 2);
    addEdge(g, 1, 3);
    addEdge(g, 2, 3);
    addEdge(g, 3, 4);
    printf("Graph created with 5 nodes\n\n");
    
    printf("--- Graph Traversals ---\n");
    BFS(g, 0);
    DFS(g, 0);
    printf("\n");
    
    printf("=== SUMMARY ===\n");
    printf("BFS: O(V+E) - breadth first, level order\n");
    printf("DFS: O(V+E) - depth first, recursive\n");
    printf("Network: Network discovery, topology analysis\n");
    
    free(g);
    return 0;
}
