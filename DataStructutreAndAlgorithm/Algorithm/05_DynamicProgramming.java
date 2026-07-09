/*
================================================================================
DYNAMIC PROGRAMMING - JAVA VERSION
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
- Time: O(n²) for Fibonacci, O(n) for optimized versions
- Space: O(n) for tabulation

KEY INSIGHT:
Optimal substructure: Optimal solution contains optimal solutions to subproblems.
DP trades space for time.
================================================================================
*/

import java.util.*;

public class DynamicProgramming {
    
    static class FibonacciMemo {
        private Map<Integer, Long> cache = new HashMap<>();
        private long functionCalls = 0;
        
        long compute(int n) {
            functionCalls++;
            
            if (n <= 1) return n;
            
            if (cache.containsKey(n)) {
                return cache.get(n);
            }
            
            long result = compute(n - 1) + compute(n - 2);
            cache.put(n, result);
            return result;
        }
        
        long getFunctionCalls() {
            return functionCalls;
        }
    }
    
    /*
    Fibonacci using tabulation (bottom-up DP)
    */
    static long fibonacciTabulation(int n) {
        if (n <= 1) return n;
        
        long[] dp = new long[n + 1];
        dp[0] = 0;
        dp[1] = 1;
        
        for (int i = 2; i <= n; i++) {
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        
        return dp[n];
    }
    
    /*
    0/1 KNAPSACK PROBLEM: Network Resource Allocation
    */
    static class KnapsackSolver {
        int capacity;
        int[] weights;  // Bandwidth requirements
        int[] values;   // Priority/importance
        int[][] dp;
        int n;
        
        KnapsackSolver(int capacity, int[] weights, int[] values) {
            this.capacity = capacity;
            this.weights = weights;
            this.values = values;
            this.n = weights.length;
            this.dp = new int[n + 1][capacity + 1];
        }
        
        void solve() {
            System.out.println("\n=== 0/1 KNAPSACK - NETWORK RESOURCE ALLOCATION ===");
            System.out.println("Capacity (bandwidth): " + capacity + " Mbps");
            System.out.println("Items (network services):");
            
            for (int i = 0; i < n; i++) {
                System.out.println("  Service " + (i + 1) + ": bandwidth=" + weights[i] + 
                                 " Mbps, priority=" + values[i]);
            }
            
            // Fill DP table
            for (int i = 1; i <= n; i++) {
                for (int w = 0; w <= capacity; w++) {
                    // Option 1: Don't take item i-1
                    dp[i][w] = dp[i - 1][w];
                    
                    // Option 2: Take item i-1 (if it fits)
                    if (weights[i - 1] <= w) {
                        int valueWithItem = values[i - 1] + dp[i - 1][w - weights[i - 1]];
                        if (valueWithItem > dp[i][w]) {
                            dp[i][w] = valueWithItem;
                        }
                    }
                }
            }
            
            System.out.println("\n=== RESULTS ===");
            System.out.println("Maximum priority: " + dp[n][capacity]);
            
            // Backtrack to find selected items
            System.out.println("\nSelected services:");
            int w = capacity;
            int totalBandwidth = 0;
            int totalPriority = 0;
            
            for (int i = n; i > 0; i--) {
                if (dp[i][w] != dp[i - 1][w]) {
                    System.out.println("  Service " + i + ": bandwidth=" + weights[i - 1] + 
                                     " Mbps, priority=" + values[i - 1] + " ✓");
                    totalBandwidth += weights[i - 1];
                    totalPriority += values[i - 1];
                    w -= weights[i - 1];
                }
            }
            
            System.out.println("\nTotal bandwidth used: " + totalBandwidth + "/" + capacity + " Mbps");
            System.out.println("Total priority achieved: " + totalPriority);
        }
    }
    
    /*
    Coin Change Problem: Minimum coins to make amount
    Network context: Minimum hops to reach distance
    */
    static int coinChange(int[] coins, int amount) {
        int[] dp = new int[amount + 1];
        
        for (int i = 0; i <= amount; i++) {
            dp[i] = amount + 1;  // Initialize to impossible value
        }
        dp[0] = 0;  // Base case: 0 coins for amount 0
        
        for (int i = 1; i <= amount; i++) {
            for (int coin : coins) {
                if (coin <= i) {
                    dp[i] = Math.min(dp[i], dp[i - coin] + 1);
                }
            }
        }
        
        return (dp[amount] > amount) ? -1 : dp[amount];
    }
    
    /*
    Longest Common Subsequence: Pattern matching
    Network context: Deep Packet Inspection
    */
    static int longestCommonSubsequence(String str1, String str2) {
        int len1 = str1.length();
        int len2 = str2.length();
        
        int[][] dp = new int[len1 + 1][len2 + 1];
        
        for (int i = 1; i <= len1; i++) {
            for (int j = 1; j <= len2; j++) {
                if (str1.charAt(i - 1) == str2.charAt(j - 1)) {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                } else {
                    dp[i][j] = Math.max(dp[i - 1][j], dp[i][j - 1]);
                }
            }
        }
        
        return dp[len1][len2];
    }
    
    /*
    Edit Distance (Levenshtein Distance)
    Network context: Protocol verification
    */
    static int editDistance(String str1, String str2) {
        int len1 = str1.length();
        int len2 = str2.length();
        
        int[][] dp = new int[len1 + 1][len2 + 1];
        
        // Initialize base cases
        for (int i = 0; i <= len1; i++) {
            dp[i][0] = i;
        }
        for (int j = 0; j <= len2; j++) {
            dp[0][j] = j;
        }
        
        // Fill DP table
        for (int i = 1; i <= len1; i++) {
            for (int j = 1; j <= len2; j++) {
                if (str1.charAt(i - 1) == str2.charAt(j - 1)) {
                    dp[i][j] = dp[i - 1][j - 1];
                } else {
                    dp[i][j] = 1 + Math.min(
                        Math.min(dp[i - 1][j], dp[i][j - 1]),
                        dp[i - 1][j - 1]
                    );
                }
            }
        }
        
        return dp[len1][len2];
    }
    
    /*
    Max Subarray Sum (Kadane's Algorithm variant)
    Network context: Maximum throughput in time window
    */
    static int maxSubarraySum(int[] arr) {
        int maxCurrent = arr[0];
        int maxGlobal = arr[0];
        
        for (int i = 1; i < arr.length; i++) {
            maxCurrent = Math.max(arr[i], maxCurrent + arr[i]);
            maxGlobal = Math.max(maxGlobal, maxCurrent);
        }
        
        return maxGlobal;
    }
    
    // ============ DEMONSTRATIONS ============
    
    static void fibonacciDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: FIBONACCI - MEMOIZATION vs TABULATION              ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int n = 40;
        
        // Memoization
        FibonacciMemo memo = new FibonacciMemo();
        long resultMemo = memo.compute(n);
        System.out.println("\nMemoization (top-down):");
        System.out.println("  Result: " + resultMemo);
        System.out.println("  Function calls: " + memo.getFunctionCalls());
        
        // Tabulation
        long resultTab = fibonacciTabulation(n);
        System.out.println("\nTabulation (bottom-up):");
        System.out.println("  Result: " + resultTab);
        System.out.println("  Time: O(n) instead of O(2^n)");
        System.out.println("  Space: O(n) for DP table");
    }
    
    static void knapsackDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: 0/1 KNAPSACK - NETWORK RESOURCE ALLOCATION         ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int capacity = 100;  // 100 Mbps
        int[] weights = {20, 30, 25, 40, 35};      // Bandwidth
        int[] values = {80, 100, 90, 120, 110};    // Priority
        
        KnapsackSolver solver = new KnapsackSolver(capacity, weights, values);
        solver.solve();
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Problem: Allocate limited bandwidth to services");
        System.out.println("- Each service has bandwidth requirement and priority");
        System.out.println("- Goal: Maximize priority within bandwidth constraint");
        System.out.println("- Solution: Use DP to find optimal service combination");
    }
    
    static void coinChangeDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: COIN CHANGE - MINIMUM HOPS IN ROUTING              ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[] coins = {1, 5, 10, 25};  // Available hop costs
        
        System.out.print("\nAvailable hop costs: ");
        for (int coin : coins) {
            System.out.print(coin + " ");
        }
        System.out.println("\n");
        
        System.out.println("Find minimum hops to reach different distances:");
        
        for (int amount = 10; amount <= 100; amount += 10) {
            int minHops = coinChange(coins, amount);
            System.out.println("  Distance " + amount + ": " + minHops + " hops");
        }
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Coins = Available link costs");
        System.out.println("- Amount = Total distance to cover");
        System.out.println("- Solution = Minimum number of hops");
    }
    
    static void lcsDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 4: LONGEST COMMON SUBSEQUENCE - PATTERN MATCHING      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        String[] patterns = {"AGGTAB", "TCGATAG", "HTTPHEAD"};
        String[] texts = {"GXTXAYB", "GATGAT", "HTTPPOST"};
        
        for (int i = 0; i < patterns.length; i++) {
            int length = longestCommonSubsequence(patterns[i], texts[i]);
            System.out.println("\nLCS(\"" + patterns[i] + "\", \"" + texts[i] + "\") = " + length);
        }
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Deep Packet Inspection (DPI): Find pattern in network traffic");
        System.out.println("- Virus/malware detection: Match signatures");
        System.out.println("- Network protocol analysis: Find common fields");
    }
    
    static void editDistanceDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 5: EDIT DISTANCE - PROTOCOL VERIFICATION              ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nCalculating edit distance (Levenshtein distance):");
        
        String[][] pairs = {
            {"HTTP/1.1", "HTTP/1.0"},
            {"HELLO", "HALLO"},
            {"intention", "execution"}
        };
        
        for (String[] pair : pairs) {
            int dist = editDistance(pair[0], pair[1]);
            System.out.println("  \"" + pair[0] + "\" ↔ \"" + pair[1] + "\": " + dist + " edits");
        }
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Protocol version compatibility checking");
        System.out.println("- Command parsing with tolerance");
        System.out.println("- Spam detection with fuzzy matching");
    }
    
    static void throughputDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 6: MAX THROUGHPUT - PEAK BANDWIDTH DETECTION          ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[] throughput = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        
        System.out.print("\nThroughput changes over time: ");
        for (int t : throughput) {
            System.out.print(t + " ");
        }
        System.out.println();
        
        int maxSum = maxSubarraySum(throughput);
        System.out.println("Maximum cumulative throughput: " + maxSum);
        
        System.out.println("\nNETWORK CONTEXT:");
        System.out.println("- Find best time window for throughput measurement");
        System.out.println("- Network traffic optimization");
        System.out.println("- QoS peak detection");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nFibonacci:");
        System.out.println("  Naive recursion: O(2^n) ❌ Too slow");
        System.out.println("  Memoization: O(n) ✓ Much better");
        System.out.println("  Tabulation: O(n) ✓ Best\n");
        
        System.out.println("0/1 Knapsack:");
        System.out.println("  Time: O(n * capacity)");
        System.out.println("  Space: O(n * capacity)");
        System.out.println("  Useful for: Resource allocation with constraints\n");
        
        System.out.println("Coin Change:");
        System.out.println("  Time: O(amount * coin_count)");
        System.out.println("  Space: O(amount)");
        System.out.println("  Useful for: Minimum cost path finding\n");
        
        System.out.println("LCS:");
        System.out.println("  Time: O(m * n) where m, n are string lengths");
        System.out.println("  Space: O(m * n)");
        System.out.println("  Useful for: Pattern matching in packets\n");
        
        System.out.println("Edit Distance:");
        System.out.println("  Time: O(m * n)");
        System.out.println("  Space: O(m * n)");
        System.out.println("  Useful for: Protocol version compatibility\n");
        
        System.out.println("Max Subarray (Kadane):");
        System.out.println("  Time: O(n)");
        System.out.println("  Space: O(1)");
        System.out.println("  Useful for: Peak detection");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ DYNAMIC PROGRAMMING - JAVA COMPREHENSIVE DEMONSTRATION       ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nDynamic Programming Core Principle:");
        System.out.println("'Don't solve the same problem twice'");
        System.out.println("Trade space (memory) for time (computation)\n");
        
        fibonacciDemo();
        knapsackDemo();
        coinChangeDemo();
        lcsDemo();
        editDistanceDemo();
        throughputDemo();
        complexityAnalysis();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. DP solves problems with optimal substructure");
        System.out.println("2. Memoization: Remember results of subproblems");
        System.out.println("3. Tabulation: Build solution bottom-up");
        System.out.println("4. Essential for bandwidth/resource allocation");
        System.out.println("5. Critical for high-performance network systems");
    }
}
