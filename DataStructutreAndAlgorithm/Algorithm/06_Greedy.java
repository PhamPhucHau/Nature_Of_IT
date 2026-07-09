/*
================================================================================
GREEDY ALGORITHMS - JAVA VERSION
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
================================================================================
*/

import java.util.*;

public class Greedy {
    
    static class Activity implements Comparable<Activity> {
        int id;
        int start;
        int end;
        String name;
        
        Activity(int id, int start, int end, String name) {
            this.id = id;
            this.start = start;
            this.end = end;
            this.name = name;
        }
        
        // Sort by end time
        @Override
        public int compareTo(Activity other) {
            return this.end - other.end;
        }
    }
    
    /*
    ACTIVITY SELECTION: Select maximum non-overlapping activities
    Network context: Schedule network maintenance windows
    */
    static void activitySelection(List<Activity> activities) {
        System.out.println("\n=== GREEDY: ACTIVITY SELECTION ===");
        System.out.println("Select maximum non-overlapping activities:\n");
        
        for (Activity act : activities) {
            System.out.println("  " + act.name + ": [" + act.start + ", " + act.end + "]");
        }
        
        // Sort by end time
        Collections.sort(activities);
        
        System.out.println("\nAfter sorting by end time:");
        for (Activity act : activities) {
            System.out.println("  " + act.name + ": [" + act.start + ", " + act.end + "]");
        }
        
        System.out.println("\nGreedy selection:");
        System.out.println("  Selected: " + activities.get(0).name + 
                         " (" + activities.get(0).start + "-" + activities.get(0).end + ")");
        
        int lastEnd = activities.get(0).end;
        int count = 1;
        
        for (int i = 1; i < activities.size(); i++) {
            Activity current = activities.get(i);
            if (current.start >= lastEnd) {
                System.out.println("  Selected: " + current.name + 
                                 " (" + current.start + "-" + current.end + ")");
                lastEnd = current.end;
                count++;
            }
        }
        
        System.out.println("\nTotal selected: " + count + " activities");
    }
    
    static class Item implements Comparable<Item> {
        String name;
        int weight;
        int value;
        double ratio;
        
        Item(String name, int weight, int value) {
            this.name = name;
            this.weight = weight;
            this.value = value;
            this.ratio = (double) value / weight;
        }
        
        // Sort by ratio (descending)
        @Override
        public int compareTo(Item other) {
            return Double.compare(other.ratio, this.ratio);  // Descending
        }
    }
    
    /*
    FRACTIONAL KNAPSACK: Greedy packing with partial items
    Network context: Bandwidth allocation with fractional sharing
    */
    static void fractionalKnapsack() {
        System.out.println("\n\n=== GREEDY: FRACTIONAL KNAPSACK ===");
        System.out.println("Pack items to maximize value with partial items allowed");
        System.out.println("(Network context: Fractional bandwidth sharing)\n");
        
        List<Item> items = new ArrayList<>();
        items.add(new Item("Service A", 60, 300));   // ratio = 5.0
        items.add(new Item("Service B", 100, 160));  // ratio = 1.6
        items.add(new Item("Service C", 120, 120));  // ratio = 1.0
        
        int capacity = 150;  // Max bandwidth
        
        System.out.println("Items (services):");
        for (Item item : items) {
            System.out.printf("  %s: weight=%d, value=%d, ratio=%.2f\n",
                            item.name, item.weight, item.value, item.ratio);
        }
        System.out.println("\nCapacity: " + capacity + " Mbps");
        
        // Sort by ratio
        Collections.sort(items);
        
        System.out.println("\nAfter sorting by value/weight ratio (descending):");
        for (Item item : items) {
            System.out.printf("  %s: ratio=%.2f\n", item.name, item.ratio);
        }
        
        System.out.println("\nGreedy selection:");
        double totalValue = 0;
        int remaining = capacity;
        
        for (Item item : items) {
            if (remaining >= item.weight) {
                // Take fully
                System.out.printf("  Take %s fully: weight=%d, value=%d\n",
                                item.name, item.weight, item.value);
                totalValue += item.value;
                remaining -= item.weight;
            } else if (remaining > 0) {
                // Take partially
                double fraction = (double) remaining / item.weight;
                double partialValue = fraction * item.value;
                System.out.printf("  Take %s partially (%.2f): weight=%d, partial_value=%.2f\n",
                                item.name, fraction, remaining, partialValue);
                totalValue += partialValue;
                remaining = 0;
                break;
            }
        }
        
        System.out.printf("\nTotal value: %.2f\n", totalValue);
        System.out.println("Capacity used: " + (capacity - remaining) + "/" + capacity + " Mbps");
    }
    
    static class Job implements Comparable<Job> {
        int id;
        int deadline;
        int profit;
        
        Job(int id, int deadline, int profit) {
            this.id = id;
            this.deadline = deadline;
            this.profit = profit;
        }
        
        // Sort by profit (descending)
        @Override
        public int compareTo(Job other) {
            return other.profit - this.profit;  // Descending
        }
    }
    
    /*
    JOB SEQUENCING: Schedule jobs to maximize profit with deadlines
    Network context: Task scheduling with deadlines
    */
    static void jobSequencing() {
        System.out.println("\n\n=== GREEDY: JOB SEQUENCING ===");
        System.out.println("Schedule jobs to maximize profit with deadlines\n");
        
        List<Job> jobs = new ArrayList<>();
        jobs.add(new Job(1, 4, 20));
        jobs.add(new Job(2, 1, 10));
        jobs.add(new Job(3, 1, 40));
        jobs.add(new Job(4, 1, 30));
        jobs.add(new Job(5, 3, 35));
        
        System.out.println("Jobs:");
        for (Job job : jobs) {
            System.out.println("  Job " + job.id + ": deadline=" + job.deadline + 
                             ", profit=" + job.profit);
        }
        
        // Sort by profit (descending)
        Collections.sort(jobs);
        
        System.out.println("\nAfter sorting by profit (descending):");
        for (Job job : jobs) {
            System.out.println("  Job " + job.id + ": profit=" + job.profit);
        }
        
        // Find maximum deadline
        int maxDeadline = 0;
        for (Job job : jobs) {
            maxDeadline = Math.max(maxDeadline, job.deadline);
        }
        
        // Schedule greedily
        int[] schedule = new int[maxDeadline + 1];
        int totalProfit = 0;
        
        System.out.println("\nGreedy scheduling:");
        
        for (Job job : jobs) {
            // Try to schedule at or before deadline
            for (int j = job.deadline; j > 0; j--) {
                if (schedule[j] == 0) {
                    schedule[j] = job.id;
                    System.out.println("  Job " + job.id + " scheduled at time " + j + 
                                     " (profit=" + job.profit + ")");
                    totalProfit += job.profit;
                    break;
                }
            }
        }
        
        System.out.println("\nTotal profit: " + totalProfit);
        
        System.out.println("\nFinal schedule:");
        System.out.print("Time: ");
        for (int i = 1; i <= maxDeadline; i++) {
            System.out.print(i + " ");
        }
        System.out.println();
        System.out.print("Job:  ");
        for (int i = 1; i <= maxDeadline; i++) {
            System.out.print(schedule[i] + " ");
        }
        System.out.println();
    }
    
    /*
    HUFFMAN CODING: Compress data using optimal variable-length codes
    */
    static class HuffmanNode implements Comparable<HuffmanNode> {
        char data;
        int freq;
        HuffmanNode left, right;
        
        HuffmanNode(char data, int freq) {
            this.data = data;
            this.freq = freq;
        }
        
        @Override
        public int compareTo(HuffmanNode other) {
            return this.freq - other.freq;
        }
    }
    
    static void huffmanCoding() {
        System.out.println("\n\n=== GREEDY: HUFFMAN CODING ===");
        System.out.println("Build optimal prefix-free binary codes for compression\n");
        
        int[] charFreq = {5, 9, 12, 13, 16, 45};
        char[] chars = {'a', 'b', 'c', 'd', 'e', 'f'};
        
        System.out.println("Character frequencies:");
        int total = 0;
        for (int i = 0; i < chars.length; i++) {
            System.out.println("  '" + chars[i] + "': " + charFreq[i] + " occurrences");
            total += charFreq[i];
        }
        
        // Create priority queue
        PriorityQueue<HuffmanNode> pq = new PriorityQueue<>();
        
        for (int i = 0; i < chars.length; i++) {
            pq.add(new HuffmanNode(chars[i], charFreq[i]));
        }
        
        // Build Huffman tree
        System.out.println("\nBuilding Huffman tree (greedy selection):");
        while (pq.size() > 1) {
            HuffmanNode left = pq.poll();
            HuffmanNode right = pq.poll();
            
            HuffmanNode parent = new HuffmanNode('\0', left.freq + right.freq);
            parent.left = left;
            parent.right = right;
            
            System.out.println("  Merge: freq(" + left.freq + ") + freq(" + right.freq + 
                             ") = " + parent.freq);
            
            pq.add(parent);
        }
        
        System.out.println("\nHuffman coding is optimal variable-length code");
        System.out.println("Compression benefit: Frequently used chars get shorter codes");
        System.out.println("Example: 'f'(45) → 0 or 1, 'a'(5) → 1110 or 1111");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nActivity Selection:");
        System.out.println("  Time: O(n log n) for sorting");
        System.out.println("  Space: O(1) if sorted already");
        System.out.println("  Property: Optimal if sorted by end time\n");
        
        System.out.println("Huffman Coding:");
        System.out.println("  Time: O(n log n) with priority queue");
        System.out.println("  Space: O(n) for tree");
        System.out.println("  Property: Optimal variable-length prefix code\n");
        
        System.out.println("Fractional Knapsack:");
        System.out.println("  Time: O(n log n) for sorting");
        System.out.println("  Space: O(1)");
        System.out.println("  Property: Greedy optimal for fractional items\n");
        
        System.out.println("Job Sequencing:");
        System.out.println("  Time: O(n²) or O(n log n + n*deadline)");
        System.out.println("  Space: O(deadline)");
        System.out.println("  Property: Optimal for deadline scheduling\n");
        
        System.out.println("Important: Greedy NOT always optimal!");
        System.out.println("  0/1 Knapsack: Greedy FAILS (use DP instead)");
        System.out.println("  TSP: Greedy gives ~2x optimal (NP-hard)");
    }
    
    /*
    PRACTICAL EXAMPLE: Network Load Balancing with Greedy
    */
    static void loadBalancingExample() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ PRACTICAL: NETWORK LOAD BALANCING                             ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nScenario: Distribute incoming requests to servers");
        System.out.println("Greedy strategy: Always send to server with least connections\n");
        
        int[] serverConnections = {5, 8, 3, 6};
        String[] servers = {"Server-1", "Server-2", "Server-3", "Server-4"};
        
        System.out.println("Current server loads:");
        for (int i = 0; i < servers.length; i++) {
            System.out.println("  " + servers[i] + ": " + serverConnections[i] + " connections");
        }
        
        System.out.println("\nNew requests arrive (10 requests):");
        for (int req = 1; req <= 10; req++) {
            // Find server with min connections
            int minIdx = 0;
            for (int i = 1; i < serverConnections.length; i++) {
                if (serverConnections[i] < serverConnections[minIdx]) {
                    minIdx = i;
                }
            }
            
            serverConnections[minIdx]++;
            System.out.println("  Request " + req + " → " + servers[minIdx]);
        }
        
        System.out.println("\nFinal loads:");
        for (int i = 0; i < servers.length; i++) {
            System.out.println("  " + servers[i] + ": " + serverConnections[i] + " connections");
        }
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ GREEDY ALGORITHMS - JAVA COMPREHENSIVE DEMONSTRATION          ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nGreedy Strategy: Pick locally best option");
        System.out.println("Hope: It leads to globally optimal solution");
        System.out.println("Reality: Works for some problems, not others\n");
        
        // Activity Selection
        List<Activity> activities = new ArrayList<>();
        activities.add(new Activity(1, 1, 3, "Maintenance A"));
        activities.add(new Activity(2, 2, 5, "Maintenance B"));
        activities.add(new Activity(3, 4, 6, "Maintenance C"));
        activities.add(new Activity(4, 6, 7, "Maintenance D"));
        activities.add(new Activity(5, 5, 8, "Maintenance E"));
        activities.add(new Activity(6, 8, 9, "Maintenance F"));
        
        activitySelection(activities);
        
        // Huffman Coding
        huffmanCoding();
        
        // Fractional Knapsack
        fractionalKnapsack();
        
        // Job Sequencing
        jobSequencing();
        
        // Load Balancing
        loadBalancingExample();
        
        // Complexity Analysis
        complexityAnalysis();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Greedy: Fast, simple to implement");
        System.out.println("2. Works when greedy choice property holds");
        System.out.println("3. Must verify optimality for each problem");
        System.out.println("4. Useful for scheduling and compression");
        System.out.println("5. Essential for load balancing algorithms");
    }
}
