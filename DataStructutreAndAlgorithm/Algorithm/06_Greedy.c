/*
================================================================================
GREEDY ALGORITHMS
================================================================================

STORY:
------
A traveler enters a city with three main roads.

One road is short but expensive.
One road is long but cheap.
One road is medium but risky.

Without thinking deeply, he always picks what seems best right now.

Most of the time, it works fine.

But sometimes, the short road leads to a dead end.

That's the risk of Greedy: Do best locally, hope it's best globally.

NETWORK APPLICATION:
- Huffman coding for data compression
- Activity selection in scheduling
- Fractional knapsack (resource allocation)
- Interval scheduling (task scheduling)
- Load balancing (least connection)
- Network bandwidth allocation
- Job scheduling

COMPLEXITY:
- Time: Usually O(n log n) or O(n²)
- Space: O(n)

KEY INSIGHT:
Greedy works when optimal substructure AND greedy choice property hold.
Not all problems have these properties.
Must verify greedy is optimal for specific problem.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 100
#define MAX_TASKS 100

// Activity Selection Problem
typedef struct {
    int start;
    int end;
    char name[50];
} Activity;

typedef struct {
    Activity activities[MAX_ITEMS];
    int count;
} ActivityList;

int compare_activities(const void* a, const void* b) {
    Activity* act_a = (Activity*)a;
    Activity* act_b = (Activity*)b;
    return act_a->end - act_b->end;  // Sort by end time
}

/*
ACTIVITY SELECTION: Select maximum non-overlapping activities
Network context: Schedule network maintenance windows
*/
void activity_selection(ActivityList* list) {
    printf("\n=== GREEDY: ACTIVITY SELECTION ===\n");
    printf("Select maximum non-overlapping activities:\n");
    
    for (int i = 0; i < list->count; i++) {
        printf("  %s: [%d, %d]\n", list->activities[i].name,
               list->activities[i].start, list->activities[i].end);
    }
    
    // Sort by end time
    qsort(list->activities, list->count, sizeof(Activity), compare_activities);
    
    printf("\nAfter sorting by end time:\n");
    for (int i = 0; i < list->count; i++) {
        printf("  %s: [%d, %d]\n", list->activities[i].name,
               list->activities[i].start, list->activities[i].end);
    }
    
    printf("\nGreedy selection:\n");
    printf("  Selected: %s (%d-%d)\n", list->activities[0].name,
           list->activities[0].start, list->activities[0].end);
    
    int last_end = list->activities[0].end;
    int count = 1;
    
    for (int i = 1; i < list->count; i++) {
        if (list->activities[i].start >= last_end) {
            printf("  Selected: %s (%d-%d)\n", list->activities[i].name,
                   list->activities[i].start, list->activities[i].end);
            last_end = list->activities[i].end;
            count++;
        }
    }
    
    printf("\nTotal selected: %d activities\n", count);
}

// Huffman Coding
typedef struct {
    char data;
    int freq;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

typedef struct {
    HuffmanNode* nodes[MAX_ITEMS];
    int size;
} MinHeap;

void min_heap_push(MinHeap* heap, HuffmanNode* node) {
    heap->nodes[heap->size] = node;
    heap->size++;
    
    // Bubble up
    int idx = heap->size - 1;
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->nodes[idx]->freq < heap->nodes[parent]->freq) {
            HuffmanNode* temp = heap->nodes[idx];
            heap->nodes[idx] = heap->nodes[parent];
            heap->nodes[parent] = temp;
            idx = parent;
        } else {
            break;
        }
    }
}

HuffmanNode* min_heap_pop(MinHeap* heap) {
    if (heap->size == 0) return NULL;
    
    HuffmanNode* result = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    
    // Bubble down
    int idx = 0;
    while (2 * idx + 1 < heap->size) {
        int smallest = idx;
        if (heap->nodes[2 * idx + 1]->freq < heap->nodes[smallest]->freq) {
            smallest = 2 * idx + 1;
        }
        if (2 * idx + 2 < heap->size && 
            heap->nodes[2 * idx + 2]->freq < heap->nodes[smallest]->freq) {
            smallest = 2 * idx + 2;
        }
        if (smallest != idx) {
            HuffmanNode* temp = heap->nodes[idx];
            heap->nodes[idx] = heap->nodes[smallest];
            heap->nodes[smallest] = temp;
            idx = smallest;
        } else {
            break;
        }
    }
    
    return result;
}

/*
HUFFMAN CODING: Compress data using optimal variable-length codes
Network context: Compress network packets
*/
void huffman_coding_demo() {
    printf("\n\n=== GREEDY: HUFFMAN CODING ===\n");
    printf("Build optimal prefix-free binary codes for compression\n\n");
    
    // Frequency table for ASCII characters
    struct {
        char c;
        int freq;
    } chars[] = {
        {'a', 5}, {'b', 9}, {'c', 12}, {'d', 13}, {'e', 16}, {'f', 45}
    };
    int char_count = 6;
    
    printf("Character frequencies:\n");
    int total = 0;
    for (int i = 0; i < char_count; i++) {
        printf("  '%c': %d occurrences\n", chars[i].c, chars[i].freq);
        total += chars[i].freq;
    }
    
    MinHeap heap;
    heap.size = 0;
    
    // Create leaf nodes and add to heap
    for (int i = 0; i < char_count; i++) {
        HuffmanNode* node = malloc(sizeof(HuffmanNode));
        node->data = chars[i].c;
        node->freq = chars[i].freq;
        node->left = NULL;
        node->right = NULL;
        min_heap_push(&heap, node);
    }
    
    // Build Huffman tree
    printf("\nBuilding Huffman tree (greedy selection):\n");
    while (heap.size > 1) {
        HuffmanNode* left = min_heap_pop(&heap);
        HuffmanNode* right = min_heap_pop(&heap);
        
        HuffmanNode* parent = malloc(sizeof(HuffmanNode));
        parent->freq = left->freq + right->freq;
        parent->left = left;
        parent->right = right;
        parent->data = '\0';
        
        printf("  Merge: freq(%d) + freq(%d) = %d\n",
               left->freq, right->freq, parent->freq);
        
        min_heap_push(&heap, parent);
    }
    
    printf("\nHuffman coding is optimal variable-length code\n");
    printf("Compression benefit: Frequently used chars get shorter codes\n");
    printf("Example: 'f'(45) → 0 or 1, 'a'(5) → 1110 or 1111\n");
}

// Fractional Knapsack (can take partial items)
typedef struct {
    char name[50];
    int weight;
    int value;
    double ratio;  // Value per weight
} Item;

int compare_items_by_ratio(const void* a, const void* b) {
    Item* item_a = (Item*)a;
    Item* item_b = (Item*)b;
    return (item_a->ratio < item_b->ratio) ? 1 : -1;  // Descending
}

/*
FRACTIONAL KNAPSACK: Greedy packing with partial items
Network context: Bandwidth allocation with fractional sharing
*/
void fractional_knapsack() {
    printf("\n\n=== GREEDY: FRACTIONAL KNAPSACK ===\n");
    printf("Pack items to maximize value with partial items allowed\n");
    printf("(Network context: Fractional bandwidth sharing)\n\n");
    
    Item items[] = {
        {"Service A", 60, 300, 300.0/60},   // value/weight = 5.0
        {"Service B", 100, 160, 160.0/100}, // value/weight = 1.6
        {"Service C", 120, 120, 120.0/120}  // value/weight = 1.0
    };
    int item_count = 3;
    int capacity = 150;  // Max bandwidth
    
    printf("Items (services):\n");
    for (int i = 0; i < item_count; i++) {
        printf("  %s: weight=%d, value=%d, ratio=%.2f\n",
               items[i].name, items[i].weight, items[i].value, items[i].ratio);
    }
    printf("\nCapacity: %d Mbps\n", capacity);
    
    // Sort by value-to-weight ratio
    qsort(items, item_count, sizeof(Item), compare_items_by_ratio);
    
    printf("\nAfter sorting by value/weight ratio (descending):\n");
    for (int i = 0; i < item_count; i++) {
        printf("  %s: ratio=%.2f\n", items[i].name, items[i].ratio);
    }
    
    printf("\nGreedy selection:\n");
    double total_value = 0;
    int remaining = capacity;
    
    for (int i = 0; i < item_count; i++) {
        if (remaining >= items[i].weight) {
            // Take fully
            printf("  Take %s fully: weight=%d, value=%d\n",
                   items[i].name, items[i].weight, items[i].value);
            total_value += items[i].value;
            remaining -= items[i].weight;
        } else if (remaining > 0) {
            // Take partially
            double fraction = (double)remaining / items[i].weight;
            double partial_value = fraction * items[i].value;
            printf("  Take %s partially (%.2f): weight=%d, partial_value=%.2f\n",
                   items[i].name, fraction, remaining, partial_value);
            total_value += partial_value;
            remaining = 0;
            break;
        }
    }
    
    printf("\nTotal value: %.2f\n", total_value);
    printf("Capacity used: %d/%d Mbps\n", capacity - remaining, capacity);
}

// Job Sequencing
typedef struct {
    int id;
    int deadline;
    int profit;
} Job;

int compare_jobs_by_profit(const void* a, const void* b) {
    Job* job_a = (Job*)a;
    Job* job_b = (Job*)b;
    return job_b->profit - job_a->profit;  // Descending
}

/*
JOB SEQUENCING: Schedule jobs to maximize profit
Network context: Task scheduling with deadlines
*/
void job_sequencing() {
    printf("\n\n=== GREEDY: JOB SEQUENCING ===\n");
    printf("Schedule jobs to maximize profit with deadlines\n\n");
    
    Job jobs[] = {
        {1, 4, 20},   // Job 1: deadline=4, profit=20
        {2, 1, 10},   // Job 2: deadline=1, profit=10
        {3, 1, 40},   // Job 3: deadline=1, profit=40
        {4, 1, 30},   // Job 4: deadline=1, profit=30
        {5, 3, 35},   // Job 5: deadline=3, profit=35
    };
    int job_count = 5;
    
    printf("Jobs:\n");
    for (int i = 0; i < job_count; i++) {
        printf("  Job %d: deadline=%d, profit=%d\n",
               jobs[i].id, jobs[i].deadline, jobs[i].profit);
    }
    
    // Sort by profit (descending)
    qsort(jobs, job_count, sizeof(Job), compare_jobs_by_profit);
    
    printf("\nAfter sorting by profit (descending):\n");
    for (int i = 0; i < job_count; i++) {
        printf("  Job %d: profit=%d\n", jobs[i].id, jobs[i].profit);
    }
    
    // Find maximum deadline
    int max_deadline = 0;
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].deadline > max_deadline) {
            max_deadline = jobs[i].deadline;
        }
    }
    
    // Schedule greedily
    int* schedule = (int*)malloc((max_deadline + 1) * sizeof(int));
    memset(schedule, 0, (max_deadline + 1) * sizeof(int));
    
    int total_profit = 0;
    printf("\nGreedy scheduling:\n");
    
    for (int i = 0; i < job_count; i++) {
        // Try to schedule at or before deadline
        for (int j = jobs[i].deadline; j > 0; j--) {
            if (schedule[j] == 0) {
                schedule[j] = jobs[i].id;
                printf("  Job %d scheduled at time %d (profit=%d)\n",
                       jobs[i].id, j, jobs[i].profit);
                total_profit += jobs[i].profit;
                break;
            }
        }
    }
    
    printf("\nTotal profit: %d\n", total_profit);
    
    printf("\nFinal schedule:\n");
    printf("Time: ");
    for (int i = 1; i <= max_deadline; i++) {
        printf("%d ", i);
    }
    printf("\nJob:  ");
    for (int i = 1; i <= max_deadline; i++) {
        printf("%d ", schedule[i]);
    }
    printf("\n");
    
    free(schedule);
}

void complexity_analysis() {
    printf("\n\n╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nActivity Selection:\n");
    printf("  Time: O(n log n) for sorting\n");
    printf("  Space: O(1) if sorted already\n");
    printf("  Property: Optimal if sorted by end time\n\n");
    
    printf("Huffman Coding:\n");
    printf("  Time: O(n log n) with priority queue\n");
    printf("  Space: O(n) for tree\n");
    printf("  Property: Optimal variable-length prefix code\n\n");
    
    printf("Fractional Knapsack:\n");
    printf("  Time: O(n log n) for sorting\n");
    printf("  Space: O(1)\n");
    printf("  Property: Greedy optimal for fractional items\n\n");
    
    printf("Job Sequencing:\n");
    printf("  Time: O(n² ) or O(n log n + n*deadline)\n");
    printf("  Space: O(deadline)\n");
    printf("  Property: Optimal for deadline scheduling\n\n");
    
    printf("Important: Greedy NOT always optimal!\n");
    printf("  0/1 Knapsack: Greedy FAILS (use DP instead)\n");
    printf("  TSP: Greedy gives ~2x optimal (NP-hard)\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ GREEDY ALGORITHMS - COMPREHENSIVE DEMONSTRATION               ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nGreedy Strategy: Pick locally best option\n");
    printf("Hope: It leads to globally optimal solution\n");
    printf("Reality: Works for some problems, not others\n");
    
    // Activity Selection Demo
    ActivityList list;
    list.count = 6;
    list.activities[0] = (Activity){1, 3, "Maintenance A"};
    list.activities[1] = (Activity){2, 5, "Maintenance B"};
    list.activities[2] = (Activity){4, 6, "Maintenance C"};
    list.activities[3] = (Activity){6, 7, "Maintenance D"};
    list.activities[4] = (Activity){5, 8, "Maintenance E"};
    list.activities[5] = (Activity){8, 9, "Maintenance F"};
    
    activity_selection(&list);
    
    // Huffman Coding Demo
    huffman_coding_demo();
    
    // Fractional Knapsack
    fractional_knapsack();
    
    // Job Sequencing
    job_sequencing();
    
    // Complexity Analysis
    complexity_analysis();
    
    printf("\nKEY TAKEAWAYS:\n");
    printf("1. Greedy: Fast, simple to implement\n");
    printf("2. Works when greedy choice property holds\n");
    printf("3. Must verify optimality for each problem\n");
    printf("4. Useful for scheduling and compression\n");
    printf("5. Essential for load balancing algorithms\n");
    
    return 0;
}
