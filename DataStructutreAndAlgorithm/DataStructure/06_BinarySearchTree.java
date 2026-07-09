/*
================================================================================
BINARY SEARCH TREE (BST) - JAVA VERSION
================================================================================

STORY:
------
A librarian has 1 million books to organize.

She can't use an array: searching would take forever.

She can't just store in order: inserting a new book would require moving all others.

So she uses a tree: Split into left (smaller) and right (larger).

Every search cuts the problem in half.

This is a Binary Search Tree.

NETWORK APPLICATION:
- IP routing table lookups
- DNS hierarchical name resolution
- Network range queries (CIDR blocks)
- Packet classification
- Hierarchical resource allocation
- B-Trees in databases and file systems

COMPLEXITY:
- Search: O(log n) average, O(n) worst (unbalanced)
- Insert: O(log n) average, O(n) worst
- Delete: O(log n) average, O(n) worst
- Range Query: O(log n + k) where k = results

KEY INSIGHT:
BST maintains sorted order implicitly through structure.
Excellent for range queries unlike hash tables.
================================================================================
*/

import java.util.*;

public class BinarySearchTreeDS {
    
    static class TreeNode {
        int key;
        String value;
        TreeNode left, right;
        int height;
        
        TreeNode(int key, String value) {
            this.key = key;
            this.value = value;
            this.height = 1;
        }
    }
    
    static class BST {
        TreeNode root;
        int comparisons;
        
        BST() {
            this.root = null;
            this.comparisons = 0;
        }
        
        int getHeight(TreeNode node) {
            return (node == null) ? 0 : node.height;
        }
        
        void updateHeight(TreeNode node) {
            if (node != null) {
                node.height = 1 + Math.max(getHeight(node.left), getHeight(node.right));
            }
        }
        
        // Insert - O(log n) average
        TreeNode insert(TreeNode node, int key, String value) {
            if (node == null) {
                return new TreeNode(key, value);
            }
            
            comparisons++;
            
            if (key < node.key) {
                node.left = insert(node.left, key, value);
            } else if (key > node.key) {
                node.right = insert(node.right, key, value);
            } else {
                // Key exists - update value
                node.value = value;
                return node;
            }
            
            updateHeight(node);
            return node;
        }
        
        public void insert(int key, String value) {
            root = insert(root, key, value);
        }
        
        // Search - O(log n) average
        TreeNode search(TreeNode node, int key) {
            if (node == null) {
                return null;
            }
            
            comparisons++;
            
            if (key == node.key) {
                return node;
            } else if (key < node.key) {
                return search(node.left, key);
            } else {
                return search(node.right, key);
            }
        }
        
        public TreeNode search(int key) {
            comparisons = 0;
            return search(root, key);
        }
        
        // In-order traversal (sorted)
        void inOrder(TreeNode node, List<Integer> result) {
            if (node == null) return;
            
            inOrder(node.left, result);
            result.add(node.key);
            inOrder(node.right, result);
        }
        
        public List<Integer> getInOrderTraversal() {
            List<Integer> result = new ArrayList<>();
            inOrder(root, result);
            return result;
        }
        
        // Range query: find all keys in [min, max]
        void rangeQuery(TreeNode node, int min, int max, List<Integer> result) {
            if (node == null) return;
            
            if (node.key > min) {
                rangeQuery(node.left, min, max, result);
            }
            
            if (node.key >= min && node.key <= max) {
                result.add(node.key);
            }
            
            if (node.key < max) {
                rangeQuery(node.right, min, max, result);
            }
        }
        
        public List<Integer> rangeQuery(int min, int max) {
            List<Integer> result = new ArrayList<>();
            rangeQuery(root, min, max, result);
            return result;
        }
    }
    
    static void basicBSTDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: BINARY SEARCH TREE BASICS                        ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BST tree = new BST();
        
        System.out.println("\nInserting values (50, 30, 70, 20, 40, 60, 80):");
        
        tree.insert(50, "router-50");
        tree.insert(30, "router-30");
        tree.insert(70, "router-70");
        tree.insert(20, "router-20");
        tree.insert(40, "router-40");
        tree.insert(60, "router-60");
        tree.insert(80, "router-80");
        
        System.out.println("\nTree structure (in-order traversal - sorted):");
        List<Integer> sorted = tree.getInOrderTraversal();
        for (Integer val : sorted) {
            System.out.print(val + " ");
        }
        System.out.println();
        
        System.out.println("\nSearching for values:");
        
        int[] searchKeys = {40, 100, 60};
        for (int key : searchKeys) {
            TreeNode found = tree.search(key);
            if (found != null) {
                System.out.println("  Found " + key + " (" + found.value + 
                                 ") after " + tree.comparisons + " comparisons");
            } else {
                System.out.println("  Not found " + key + " after " + 
                                 tree.comparisons + " comparisons");
            }
        }
    }
    
    static void routingTableDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: IP ROUTING TABLE (Real network application)      ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BST routingTable = new BST();
        
        System.out.println("\nBuilding routing table (CIDR blocks):");
        
        routingTable.insert(10, "10.0.0.0/8");
        routingTable.insert(172, "172.16.0.0/12");
        routingTable.insert(192, "192.168.0.0/16");
        routingTable.insert(8, "8.8.8.8/32");
        routingTable.insert(20, "20.0.0.0/8");
        
        System.out.println("Routing entries in sorted order:");
        List<Integer> entries = routingTable.getInOrderTraversal();
        for (Integer entry : entries) {
            System.out.print(entry + " ");
        }
        System.out.println("\n");
        
        System.out.println("Routing table lookup:");
        int[] lookupIps = {10, 50, 192};
        for (int ip : lookupIps) {
            TreeNode route = routingTable.search(ip);
            if (route != null) {
                System.out.println("  IP " + ip + " → " + route.value + 
                                 " (found in " + routingTable.comparisons + " steps)");
            } else {
                System.out.println("  IP " + ip + " → NOT FOUND (checked in " + 
                                 routingTable.comparisons + " steps)");
            }
        }
    }
    
    static void rangeQueryDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: RANGE QUERIES (CIDR subnet matching)            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        BST tree = new BST();
        
        System.out.println("\nBuilding tree with network prefixes:");
        
        int[] prefixes = {10, 20, 30, 40, 50, 60, 70, 80, 90};
        for (int prefix : prefixes) {
            tree.insert(prefix, "");
        }
        
        System.out.print("Prefixes in order: ");
        List<Integer> inOrder = tree.getInOrderTraversal();
        for (Integer p : inOrder) {
            System.out.print(p + " ");
        }
        System.out.println();
        
        System.out.println("\nRange queries (find CIDR blocks in range):");
        
        int[][] ranges = {{25, 75}, {10, 50}, {60, 100}};
        
        for (int[] range : ranges) {
            int min = range[0];
            int max = range[1];
            List<Integer> result = tree.rangeQuery(min, max);
            
            System.out.print("  Range [" + min + ", " + max + "]: ");
            for (Integer val : result) {
                System.out.print(val + " ");
            }
            System.out.println("(" + result.size() + " results)");
        }
        
        System.out.println("\nNETWORK APPLICATION:");
        System.out.println("- BST perfect for range queries");
        System.out.println("- Hash table cannot do range queries");
        System.out.println("- Find all routes matching a prefix");
        System.out.println("- Example: Find all IPs in 192.168.0.0/16");
    }
    
    static void hierarchicalDNSDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 4: HIERARCHICAL DNS RESOLUTION                     ║");
        System.out.println("║ (Tree-based domain name hierarchy)                         ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nDNS uses tree structure (simplified):");
        System.out.println("                  .");
        System.out.println("               /  |  \\");
        System.out.println("             com org net");
        System.out.println("            / | \\");
        System.out.println("        google fb twitter");
        System.out.println("\nTree allows efficient hierarchical lookup");
        System.out.println("Each subtree can be cached separately");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nBinary Search Tree Performance:");
        System.out.println("Average case (balanced):");
        System.out.println("  - Search: O(log n)");
        System.out.println("  - Insert: O(log n)");
        System.out.println("  - Delete: O(log n)");
        System.out.println("  - Range query: O(log n + k) where k = results\n");
        
        System.out.println("Worst case (unbalanced/linear):");
        System.out.println("  - Search: O(n)");
        System.out.println("  - Insert: O(n)");
        System.out.println("  - Delete: O(n)");
        System.out.println("  - This happens when tree is skewed\n");
        
        System.out.println("Comparison with other structures:");
        System.out.println("Operation  | Hash Table | BST        | Array");
        System.out.println("-----------|------------|------------|-------");
        System.out.println("Search     | O(1)       | O(log n)   | O(n)");
        System.out.println("Insert     | O(1)       | O(log n)   | O(n)");
        System.out.println("Delete     | O(1)       | O(log n)   | O(n)");
        System.out.println("Range Q    | X (hard)   | O(log n+k) | O(n)");
        System.out.println("Ordered    | X          | O(n)       | O(1)");
        System.out.println("\nBST winner for: Ordered access + range queries!");
    }
    
    static void balancingExplanation() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ BALANCING STRATEGIES                                           ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nUnbalanced BST problem:");
        System.out.println("  Insert in order: 1, 2, 3, 4, 5");
        System.out.println("  Creates linear chain:");
        System.out.println("    1");
        System.out.println("     \\");
        System.out.println("      2");
        System.out.println("       \\");
        System.out.println("        3");
        System.out.println("  Performance: O(n) like linked list!\n");
        
        System.out.println("Solutions:");
        System.out.println("1. AVL Tree:");
        System.out.println("   - Keep balance factor ∈ [-1, 1]");
        System.out.println("   - Guaranteed O(log n) operations");
        System.out.println("   - Rotations to rebalance\n");
        
        System.out.println("2. Red-Black Tree:");
        System.out.println("   - Simpler balancing rules than AVL");
        System.out.println("   - Color nodes (red/black)");
        System.out.println("   - Slightly faster than AVL\n");
        
        System.out.println("3. B-Tree:");
        System.out.println("   - Multi-way tree (not binary)");
        System.out.println("   - Perfect for disk I/O");
        System.out.println("   - Used in databases (InnoDB, PostgreSQL)\n");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ BINARY SEARCH TREE - JAVA COMPREHENSIVE DEMONSTRATION       ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nBST: Ordered tree where left < parent < right");
        System.out.println("Key advantage: Range queries impossible in hash tables");
        System.out.println("Trade-off: O(log n) instead of O(1) for single lookups\n");
        
        basicBSTDemo();
        routingTableDemo();
        rangeQueryDemo();
        hierarchicalDNSDemo();
        complexityAnalysis();
        balancingExplanation();
        
        System.out.println("KEY TAKEAWAYS:");
        System.out.println("1. BST: O(log n) ops on average (balanced)");
        System.out.println("2. Maintains sorted order implicitly");
        System.out.println("3. Excellent for range queries");
        System.out.println("4. Balancing required for guaranteed performance");
        System.out.println("5. Perfect for IP routing and hierarchical data");
    }
}
