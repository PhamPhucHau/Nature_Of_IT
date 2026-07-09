/*
================================================================================
DYNAMIC PROGRAMMING
================================================================================

STORY:
------
A boy climbs stairs. To reach step 100, he needs to climb step 99 or step 98.
To reach step 99, he needs to reach step 98 or 97.

He notices: He's calculating the same steps over and over.

So he writes down every result.

Next time he needs to climb step 50, he doesn't recalculate—he just looks in his notebook.

That's Dynamic Programming: Don't solve the same problem twice.

NETWORK APPLICATION:
- Bandwidth allocation (max throughput with constraints)
- Packet scheduling optimization
- Network resource planning
- Load balancing decisions
- Caching strategies (LRU, LFU)
- Routing optimization with constraints

COMPLEXITY:
- Time: O(n²) for Fibonacci, O(n) for Knapsack variants
- Space: O(n) for tabulation, O(log n) for space-optimized versions

KEY INSIGHT:
Optimal substructure: Optimal solution contains optimal solutions to subproblems.
Overlapping subproblems: Same subproblems computed multiple times.

DP trades space for time.
================================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 50
#define MAX_W 1000

// Fibonacci with memoization (top-down DP)
typedef struct {
    long long cache[MAX_N];
    int computed[MAX_N];
    long long function_calls;
} FibonacciMemo;

FibonacciMemo fib_memo_init() {
    FibonacciMemo m;
    memset(m.cache, 0, sizeof(m.cache));
    memset(m.computed, 0, sizeof(m.computed));
    m.function_calls = 0;
    return m;
}

long long fibonacci_memo(int n, FibonacciMemo* m) {
    m->function_calls++;
    
    if (n <= 1) return n;
    
    if (m->computed[n]) {
        return m->cache[n];
    }
    
    m->cache[n] = fibonacci_memo(n - 1, m) + fibonacci_memo(n - 2, m);
    m->computed[n] = 1;
    return m->cache[n];
}

// Fibonacci with tabulation (bottom-up DP)
long long fibonacci_tab(int n) {
    if (n <= 1) return n;
    
    long long dp[MAX_N];
    dp[0] = 0;
    dp[1] = 1;
    
    for (int i = 2; i <= n; i++) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }
    
    return dp[n];
}

/*
KNAPSACK PROBLEM: 0/1 Knapsack with DP
Classic resource allocation problem
*/
typedef struct {
    int capacity;
    int* weights;
    int* values;
    int n_items;
    int** dp;
    int** items_taken;
} KnapsackState;

KnapsackState knapsack_init(int capacity, int n_items) {
    KnapsackState ks;
    ks.capacity = capacity;
    ks.n_items = n_items;
    ks.weights = malloc(n_items * sizeof(int));
    ks.values = malloc(n_items * sizeof(int));
    
    ks.dp = malloc((n_items + 1) * sizeof(int*));
    ks.items_taken = malloc((n_items + 1) * sizeof(int*));
    
    for (int i = 0; i <= n_items; i++) {
        ks.dp[i] = malloc((capacity + 1) * sizeof(int));
        ks.items_taken[i] = malloc((capacity + 1) * sizeof(int));
        memset(ks.dp[i], 0, (capacity + 1) * sizeof(int));
        memset(ks.items_taken[i], 0, (capacity + 1) * sizeof(int));
    }
    
    return ks;
}

void knapsack_solve(KnapsackState* ks) {
    printf("\n=== 0/1 KNAPSACK PROBLEM - NETWORK RESOURCE ALLOCATION ===\n");
    printf("Capacity (bandwidth): %d Mbps\n", ks->capacity);
    printf("Items (network services):\n");
    
    for (int i = 0; i < ks->n_items; i++) {
        printf("  Service %d: bandwidth=%d Mbps, priority=%d\n", 
               i + 1, ks->weights[i], ks->values[i]);
    }
    
    // DP table filling
    for (int i = 1; i <= ks->n_items; i++) {
        for (int w = 0; w <= ks->capacity; w++) {
            // Option 1: Don't take item i-1
            ks->dp[i][w] = ks->dp[i - 1][w];
            
            // Option 2: Take item i-1 (if it fits)
            if (ks->weights[i - 1] <= w) {
                int value_with_item = ks->values[i - 1] + ks->dp[i - 1][w - ks->weights[i - 1]];
                if (value_with_item > ks->dp[i][w]) {
                    ks->dp[i][w] = value_with_item;
                    ks->items_taken[i][w] = 1;
                }
            }
        }
    }
    
    printf("\n=== RESULTS ===\n");
    printf("Maximum priority: %d\n", ks->dp[ks->n_items][ks->capacity]);
    
    // Backtrack to find which items were selected
    printf("\nSelected services:\n");
    int w = ks->capacity;
    int total_bandwidth = 0;
    int total_priority = 0;
    
    for (int i = ks->n_items; i > 0; i--) {
        if (ks->dp[i][w] != ks->dp[i - 1][w]) {
            printf("  Service %d: bandwidth=%d Mbps, priority=%d ✓\n",
                   i, ks->weights[i - 1], ks->values[i - 1]);
            total_bandwidth += ks->weights[i - 1];
            total_priority += ks->values[i - 1];
            w -= ks->weights[i - 1];
        }
    }
    
    printf("\nTotal bandwidth used: %d/%d Mbps\n", total_bandwidth, ks->capacity);
    printf("Total priority achieved: %d\n", total_priority);
}

/*
COIN CHANGE PROBLEM: Minimum coins to make amount
Similar to packet scheduling with minimum hops
*/
int coin_change(int* coins, int coin_count, int amount) {
    int* dp = malloc((amount + 1) * sizeof(int));
    
    for (int i = 0; i <= amount; i++) {
        dp[i] = amount + 1;  // Initialize to impossible value
    }
    dp[0] = 0;  // Base case: 0 coins needed to make 0
    
    for (int i = 1; i <= amount; i++) {
        for (int j = 0; j < coin_count; j++) {
            if (coins[j] <= i) {
                dp[i] = (dp[i - coins[j]] + 1 < dp[i]) ? dp[i - coins[j]] + 1 : dp[i];
            }
        }
    }
    
    int result = (dp[amount] > amount) ? -1 : dp[amount];
    free(dp);
    return result;
}

/*
LONGEST COMMON SUBSEQUENCE: For pattern matching in packets
*/
int lcs_length(const char* str1, const char* str2) {
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    
    int* dp = malloc((len1 + 1) * (len2 + 1) * sizeof(int));
    
    for (int i = 0; i <= len1; i++) {
        for (int j = 0; j <= len2; j++) {
            if (i == 0 || j == 0) {
                dp[i * (len2 + 1) + j] = 0;
            } else if (str1[i - 1] == str2[j - 1]) {
                dp[i * (len2 + 1) + j] = dp[(i - 1) * (len2 + 1) + (j - 1)] + 1;
            } else {
                int a = dp[(i - 1) * (len2 + 1) + j];
                int b = dp[i * (len2 + 1) + (j - 1)];
                dp[i * (len2 + 1) + j] = (a > b) ? a : b;
            }
        }
    }
    
    int result = dp[len1 * (len2 + 1) + len2];
    free(dp);
    return result;
}

void fibonacci_demo() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 1: FIBONACCI - MEMOIZATION vs TABULATION              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int n = 40;
    
    // Naive approach (for comparison, we skip due to exponential time)
    printf("\nCalculating F(%d)...\n", n);
    
    // Memoization
    FibonacciMemo m = fib_memo_init();
    long long result_memo = fibonacci_memo(n, &m);
    printf("\nMemoization (top-down):\n");
    printf("  Result: %lld\n", result_memo);
    printf("  Function calls: %lld\n", m.function_calls);
    
    // Tabulation
    long long result_tab = fibonacci_tab(n);
    printf("\nTabulation (bottom-up):\n");
    printf("  Result: %lld\n", result_tab);
    printf("  Time complexity: O(n) instead of O(2^n)\n");
    printf("  Space: O(n) for DP table\n");
}

void knapsack_demo() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 2: 0/1 KNAPSACK - NETWORK RESOURCE ALLOCATION         ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    KnapsackState ks = knapsack_init(100, 5);  // 100 Mbps capacity, 5 services
    
    // Set bandwidth and priority
    int weights[] = {20, 30, 25, 40, 35};      // Bandwidth requirements (Mbps)
    int values[] = {80, 100, 90, 120, 110};    // Priority/importance
    
    for (int i = 0; i < ks.n_items; i++) {
        ks.weights[i] = weights[i];
        ks.values[i] = values[i];
    }
    
    knapsack_solve(&ks);
    
    printf("\nNETWORK CONTEXT:\n");
    printf("- Problem: Allocate limited bandwidth to services\n");
    printf("- Each service has bandwidth requirement and priority\n");
    printf("- Goal: Maximize priority within bandwidth constraint\n");
    printf("- Solution: Use DP to find optimal service combination\n");
}

void coin_change_demo() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 3: COIN CHANGE - MINIMUM HOPS IN ROUTING              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    int coins[] = {1, 5, 10, 25};  // Available hop costs
    int coin_count = 4;
    
    printf("\nAvailable hop costs: ");
    for (int i = 0; i < coin_count; i++) {
        printf("%d ", coins[i]);
    }
    printf("\n");
    
    printf("\nFind minimum hops to reach different distances:\n");
    
    for (int amount = 10; amount <= 100; amount += 10) {
        int min_hops = coin_change(coins, coin_count, amount);
        printf("  Distance %d: %d hops\n", amount, min_hops);
    }
    
    printf("\nNETWORK CONTEXT:\n");
    printf("- Coins = Available link costs (1, 5, 10, 25)\n");
    printf("- Amount = Total distance to cover\n");
    printf("- Solution = Minimum number of hops\n");
}

void lcs_demo() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ EXAMPLE 4: LONGEST COMMON SUBSEQUENCE - PATTERN MATCHING      ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    const char* patterns[] = {
        "AGGTAB",
        "TCGATAG",
        "HTTPHEAD"
    };
    
    const char* texts[] = {
        "GXTXAYB",
        "GATGAT",
        "HTTPPOST"
    };
    
    for (int i = 0; i < 3; i++) {
        int length = lcs_length(patterns[i], texts[i]);
        printf("\nLCS('%s', '%s') = %d\n", patterns[i], texts[i], length);
    }
    
    printf("\nNETWORK CONTEXT:\n");
    printf("- Deep Packet Inspection (DPI): Find pattern in network traffic\n");
    printf("- Virus/malware detection: Match signatures\n");
    printf("- Network protocol analysis: Find common fields\n");
}

void complexity_analysis() {
    printf("\n\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ COMPLEXITY ANALYSIS                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nFibonacci:\n");
    printf("  Naive recursion: O(2^n) ❌ Too slow\n");
    printf("  Memoization: O(n) ✓ Much better\n");
    printf("  Tabulation: O(n) ✓ Best\n\n");
    
    printf("0/1 Knapsack:\n");
    printf("  Time: O(n * capacity)\n");
    printf("  Space: O(n * capacity)\n");
    printf("  Useful for: Resource allocation with constraints\n\n");
    
    printf("Coin Change:\n");
    printf("  Time: O(amount * coin_count)\n");
    printf("  Space: O(amount)\n");
    printf("  Useful for: Minimum cost path finding\n\n");
    
    printf("LCS:\n");
    printf("  Time: O(m * n) where m, n are string lengths\n");
    printf("  Space: O(m * n)\n");
    printf("  Useful for: Pattern matching in packets\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║ DYNAMIC PROGRAMMING - COMPREHENSIVE DEMONSTRATION             ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nDynamic Programming Core Principle:\n");
    printf("'Don't solve the same problem twice'\n");
    printf("Trade space (memory) for time (computation)\n");
    
    fibonacci_demo();
    knapsack_demo();
    coin_change_demo();
    lcs_demo();
    complexity_analysis();
    
    printf("\n\nKEY TAKEAWAYS:\n");
    printf("1. DP solves problems with optimal substructure\n");
    printf("2. Memoization: Remember results of subproblems\n");
    printf("3. Tabulation: Build solution bottom-up\n");
    printf("4. Essential for bandwidth/resource allocation\n");
    printf("5. Critical for high-performance network systems\n");
    
    return 0;
}
