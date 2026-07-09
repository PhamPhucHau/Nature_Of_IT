/*
================================================================================
HEAP / PRIORITY QUEUE DATA STRUCTURE - JAVA VERSION
================================================================================

STORY:
------
A hospital emergency room has patients with different priorities.

Critical patients should be treated first.

A regular queue wouldn't work: they'd be treated in arrival order.

So they use a priority queue: Most urgent patient → treated first.

Under the hood, this is a Heap.

A Heap is a tree that maintains priority order efficiently.

NETWORK APPLICATION:
- QoS (Quality of Service) scheduling
- Bandwidth allocation (priority to VoIP over FTP)
- Process scheduling in operating systems
- Event-driven networking (next event first)
- Dijkstra's algorithm (shortest path)
- Huffman coding (tree building)

COMPLEXITY:
- Insert: O(log n)
- Extract min/max: O(log n)
- Peek: O(1)
- Space: O(n)

KEY INSIGHT:
Heap is not fully sorted (unlike BST), just partially ordered.
This is the sweet spot: O(log n) insert while maintaining priority.
Perfect for systems that need fast priority-based access.
================================================================================
*/

import java.util.*;

public class HeapDS {
    
    /*
    MIN HEAP IMPLEMENTATION
    Parent is smaller than children
    */
    static class MinHeap {
        private List<Integer> heap;
        private int comparisons;
        private int swaps;
        
        MinHeap() {
            this.heap = new ArrayList<>();
            this.comparisons = 0;
            this.swaps = 0;
        }
        
        // Get parent index
        private int getParent(int i) {
            return (i - 1) / 2;
        }
        
        // Get left child index
        private int getLeftChild(int i) {
            return 2 * i + 1;
        }
        
        // Get right child index
        private int getRightChild(int i) {
            return 2 * i + 2;
        }
        
        // Swap two elements
        private void swap(int i, int j) {
            Integer temp = heap.get(i);
            heap.set(i, heap.get(j));
            heap.set(j, temp);
            swaps++;
        }
        
        // Insert element - O(log n)
        public void insert(int value) {
            heap.add(value);
            bubbleUp(heap.size() - 1);
        }
        
        // Bubble up: fix heap property by moving up
        private void bubbleUp(int idx) {
            while (idx > 0) {
                int parentIdx = getParent(idx);
                comparisons++;
                
                if (heap.get(idx) < heap.get(parentIdx)) {
                    swap(idx, parentIdx);
                    idx = parentIdx;
                } else {
                    break;
                }
            }
        }
        
        // Extract minimum - O(log n)
        public int extractMin() {
            if (heap.isEmpty()) {
                throw new NoSuchElementException("Heap is empty");
            }
            
            int min = heap.get(0);
            
            if (heap.size() == 1) {
                heap.remove(0);
            } else {
                heap.set(0, heap.remove(heap.size() - 1));
                bubbleDown(0);
            }
            
            return min;
        }
        
        // Bubble down: fix heap property by moving down
        private void bubbleDown(int idx) {
            while (true) {
                int smallest = idx;
                int leftChild = getLeftChild(idx);
                int rightChild = getRightChild(idx);
                
                if (leftChild < heap.size()) {
                    comparisons++;
                    if (heap.get(leftChild) < heap.get(smallest)) {
                        smallest = leftChild;
                    }
                }
                
                if (rightChild < heap.size()) {
                    comparisons++;
                    if (heap.get(rightChild) < heap.get(smallest)) {
                        smallest = rightChild;
                    }
                }
                
                if (smallest != idx) {
                    swap(idx, smallest);
                    idx = smallest;
                } else {
                    break;
                }
            }
        }
        
        // Peek minimum - O(1)
        public int peek() {
            if (heap.isEmpty()) {
                throw new NoSuchElementException("Heap is empty");
            }
            return heap.get(0);
        }
        
        public boolean isEmpty() {
            return heap.isEmpty();
        }
        
        public int size() {
            return heap.size();
        }
        
        public int getComparisons() {
            return comparisons;
        }
        
        public int getSwaps() {
            return swaps;
        }
    }
    
    /*
    HEAP SORT using min heap
    */
    static void heapSortDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: HEAP SORT                                          ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        int[] arr = {45, 23, 51, 12, 78, 34, 89, 1, 56, 99};
        
        System.out.println("\nOriginal array:");
        for (int num : arr) {
            System.out.print(num + " ");
        }
        System.out.println();
        
        MinHeap heap = new MinHeap();
        
        System.out.println("\nInserting elements into heap:");
        for (int num : arr) {
            heap.insert(num);
            System.out.print(num + " ");
        }
        System.out.println();
        
        System.out.println("\nExtracting in sorted order:");
        List<Integer> sorted = new ArrayList<>();
        while (!heap.isEmpty()) {
            int min = heap.extractMin();
            sorted.add(min);
            System.out.print(min + " ");
        }
        System.out.println();
        
        System.out.println("\nHeap sort statistics:");
        System.out.println("  Original array: [45, 23, 51, 12, 78, 34, 89, 1, 56, 99]");
        System.out.println("  Sorted array: " + sorted);
        System.out.println("  Comparisons: " + heap.getComparisons());
        System.out.println("  Swaps: " + heap.getSwaps());
        System.out.println("  Time complexity: O(n log n)");
    }
    
    /*
    NETWORK APPLICATION: QoS Packet Scheduling
    Priority = Service importance (higher is more important)
    */
    static class NetworkPacket implements Comparable<NetworkPacket> {
        int packetId;
        int priority;      // 1-5: Normal, Important, Critical, Emergency, Highest
        int size;          // bytes
        long timestamp;    // when packet arrived
        
        NetworkPacket(int id, int priority, int size) {
            this.packetId = id;
            this.priority = priority;
            this.size = size;
            this.timestamp = System.currentTimeMillis();
        }
        
        @Override
        public int compareTo(NetworkPacket other) {
            // Higher priority = lower value in min-heap (inverted)
            if (this.priority != other.priority) {
                return other.priority - this.priority;  // Higher priority first
            }
            // FIFO if same priority
            return (int)(this.timestamp - other.timestamp);
        }
        
        @Override
        public String toString() {
            return "P" + packetId + "(pri=" + priority + ",size=" + size + ")";
        }
    }
    
    static void qosSchedulingDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: QoS PACKET SCHEDULING                             ║");
        System.out.println("║ (Network Application: Priority-based packet transmission)    ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        PriorityQueue<NetworkPacket> queue = new PriorityQueue<>();
        
        System.out.println("\nPacket priorities:");
        System.out.println("  1-Normal, 2-Important, 3-Critical, 4-Emergency, 5-Highest\n");
        
        // Add packets with different priorities
        NetworkPacket[] packets = {
            new NetworkPacket(1, 2, 512),   // Important
            new NetworkPacket(2, 1, 1024),  // Normal (FTP)
            new NetworkPacket(3, 4, 128),   // Emergency (911 call)
            new NetworkPacket(4, 3, 256),   // Critical (VoIP)
            new NetworkPacket(5, 1, 2048),  // Normal (file download)
            new NetworkPacket(6, 5, 64),    // Highest priority
            new NetworkPacket(7, 2, 512),   // Important
        };
        
        System.out.println("Incoming packets (in arrival order):");
        for (NetworkPacket p : packets) {
            queue.add(p);
            System.out.println("  " + p);
        }
        
        System.out.println("\nTransmission order (by priority):");
        int bandwidth_used = 0;
        int packet_count = 0;
        
        while (!queue.isEmpty()) {
            NetworkPacket p = queue.poll();
            bandwidth_used += p.size;
            packet_count++;
            
            String priority_name = "";
            switch(p.priority) {
                case 1: priority_name = "Normal"; break;
                case 2: priority_name = "Important"; break;
                case 3: priority_name = "Critical"; break;
                case 4: priority_name = "Emergency"; break;
                case 5: priority_name = "Highest"; break;
            }
            
            System.out.println("  Send: " + p + " (" + priority_name + ")");
        }
        
        System.out.println("\nQoS Benefits:");
        System.out.println("- Critical packets transmitted first");
        System.out.println("- VoIP not delayed by FTP downloads");
        System.out.println("- Emergency calls get priority");
        System.out.println("- Total packets: " + packet_count);
        System.out.println("- Total bandwidth: " + bandwidth_used + " bytes");
    }
    
    /*
    DIJKSTRA'S ALGORITHM uses min-heap for efficiency
    */
    static void dijkstraWithHeapDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: DIJKSTRA'S ALGORITHM uses Heap                    ║");
        System.out.println("║ (Finding shortest path in network routing)                  ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nDijkstra's algorithm:");
        System.out.println("1. Initialize: source distance = 0, all others = infinity");
        System.out.println("2. Add source to min-heap");
        System.out.println("3. While heap not empty:");
        System.out.println("   - Extract node with min distance (O(log n))");
        System.out.println("   - For each neighbor, if path is shorter:");
        System.out.println("     - Update distance");
        System.out.println("     - Add to heap");
        System.out.println("4. Result: shortest path to all nodes\n");
        
        System.out.println("Why min-heap is essential:");
        System.out.println("- Without heap: O(V²) time");
        System.out.println("- With heap: O((V+E) log V) time");
        System.out.println("- For large networks: ~1000x faster!\n");
        
        System.out.println("Example: Network with 10,000 routers:");
        System.out.println("- Array-based: 100,000,000 operations");
        System.out.println("- Heap-based: 230,000 operations");
    }
    
    static void heapPropertiesDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ HEAP PROPERTIES AND INVARIANTS                               ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nComplete Binary Tree:");
        System.out.println("- All levels filled except possibly the last");
        System.out.println("- Last level filled from left to right");
        System.out.println("- Efficient array representation\n");
        
        System.out.println("Heap Property (Min-Heap):");
        System.out.println("- Parent ≤ Children");
        System.out.println("- Root is minimum element");
        System.out.println("- NOT fully sorted (unlike BST)\n");
        
        System.out.println("Array Representation:");
        System.out.println("- Index i: parent at (i-1)/2, children at 2i+1 and 2i+2");
        System.out.println("- Efficient: No pointers, cache-friendly\n");
        
        System.out.println("Tree structure example:");
        System.out.println("        1");
        System.out.println("       / \\");
        System.out.println("      2   3");
        System.out.println("     / \\ /");
        System.out.println("    4  5 6");
        System.out.println("\nArray: [1, 2, 3, 4, 5, 6]");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nHeap Operations:");
        System.out.println("- Insert: O(log n) - bubble up at most height");
        System.out.println("- Extract min: O(log n) - bubble down at most height");
        System.out.println("- Peek min: O(1) - just return root");
        System.out.println("- Heapify (build from array): O(n)");
        System.out.println("- Heap sort: O(n log n)\n");
        
        System.out.println("Comparison with other structures:");
        System.out.println("Operation | Heap    | BST        | Sorted Array");
        System.out.println("----------|---------|------------|-------------");
        System.out.println("Min       | O(1)    | O(log n)   | O(1)");
        System.out.println("Insert    | O(log n)| O(log n)   | O(n)");
        System.out.println("Delete    | O(log n)| O(log n)   | O(n)");
        System.out.println("Sort      | O(nlogn)| O(nlogn)   | O(1)");
        System.out.println("\nHeap is optimal for priority queue operations!");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ HEAP / PRIORITY QUEUE - JAVA COMPREHENSIVE DEMONSTRATION     ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nHeap: Partially ordered tree data structure");
        System.out.println("Key insight: Efficient priority-based access without full sort");
        System.out.println("Time: O(log n) insert/remove vs O(n) for array\n");
        
        heapSortDemo();
        qosSchedulingDemo();
        dijkstraWithHeapDemo();
        heapPropertiesDemo();
        complexityAnalysis();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Heap: O(log n) insert/extract, O(1) peek");
        System.out.println("2. Perfect for priority queues");
        System.out.println("3. Array-based: Efficient memory use");
        System.out.println("4. Essential for Dijkstra's algorithm");
        System.out.println("5. Critical for network QoS systems");
    }
}
