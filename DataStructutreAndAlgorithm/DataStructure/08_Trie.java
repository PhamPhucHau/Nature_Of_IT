/*
================================================================================
TRIE (PREFIX TREE) - JAVA VERSION
================================================================================

STORY:
------
A postal worker has 10 million addresses.

Instead of organizing by country, then city, then street, then house...

They organize by first letter, then second letter, then third...

This is a prefix tree (Trie).

Each node represents a letter. Paths from root to node spell words.

NETWORK APPLICATION:
- IP address routing (CIDR prefix matching)
- DNS name resolution
- Auto-complete in search engines
- Longest prefix matching in packet forwarding
- Spell checking and dictionary lookup
- IP blocking/filtering rules

COMPLEXITY:
- Insert: O(m) where m = key length
- Search: O(m)
- Delete: O(m)
- Prefix search: O(m + k) where k = results

KEY INSIGHT:
Trie is perfect for prefix-based queries that hash tables cannot handle.
================================================================================
*/

import java.util.*;

public class TrieDS {
    
    static class TrieNode {
        Map<Character, TrieNode> children;
        boolean isEndOfWord;
        String value;
        
        TrieNode() {
            this.children = new HashMap<>();
            this.isEndOfWord = false;
            this.value = "";
        }
    }
    
    static class Trie {
        TrieNode root;
        int nodeCount;
        int wordCount;
        
        Trie() {
            this.root = new TrieNode();
            this.nodeCount = 1;
            this.wordCount = 0;
        }
        
        // Insert - O(m) where m is key length
        void insert(String key, String value) {
            TrieNode node = root;
            
            for (char c : key.toCharArray()) {
                if (!node.children.containsKey(c)) {
                    node.children.put(c, new TrieNode());
                    nodeCount++;
                }
                node = node.children.get(c);
            }
            
            if (!node.isEndOfWord) {
                wordCount++;
            }
            
            node.isEndOfWord = true;
            node.value = value;
        }
        
        void insert(String key) {
            insert(key, "");
        }
        
        // Search - O(m)
        String search(String key) {
            TrieNode node = root;
            
            for (char c : key.toCharArray()) {
                if (!node.children.containsKey(c)) {
                    return null;
                }
                node = node.children.get(c);
            }
            
            return node.isEndOfWord ? node.value : null;
        }
        
        // Check if word exists exactly
        boolean exists(String key) {
            return search(key) != null;
        }
        
        // Prefix search - find all words with given prefix
        List<String> prefixSearch(String prefix) {
            List<String> results = new ArrayList<>();
            TrieNode node = root;
            
            // Navigate to prefix
            for (char c : prefix.toCharArray()) {
                if (!node.children.containsKey(c)) {
                    return results;  // No matches
                }
                node = node.children.get(c);
            }
            
            // DFS from prefix node
            dfsForPrefix(node, prefix, results);
            return results;
        }
        
        private void dfsForPrefix(TrieNode node, String current, List<String> results) {
            if (node.isEndOfWord) {
                results.add(current);
            }
            
            for (char c : node.children.keySet()) {
                dfsForPrefix(node.children.get(c), current + c, results);
            }
        }
        
        // Longest prefix match - essential for IP routing
        String longestPrefixMatch(String key) {
            TrieNode node = root;
            String bestMatch = null;
            String bestValue = null;
            
            for (int i = 0; i < key.length(); i++) {
                char c = key.charAt(i);
                
                if (!node.children.containsKey(c)) {
                    break;
                }
                
                node = node.children.get(c);
                
                if (node.isEndOfWord) {
                    bestMatch = key.substring(0, i + 1);
                    bestValue = node.value;
                }
            }
            
            return bestValue;
        }
    }
    
    static void basicTrieDemo() {
        System.out.println("\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 1: BASIC TRIE - Dictionary/Search                   ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Trie trie = new Trie();
        
        System.out.println("\nInserting words:");
        
        String[] words = {"apple", "application", "apply", "apricot", "banana"};
        String[] defs = {"fruit", "software", "use", "fruit", "yellow"};
        
        for (int i = 0; i < words.length; i++) {
            trie.insert(words[i], defs[i]);
            System.out.println("  '" + words[i] + "' → " + defs[i]);
        }
        
        System.out.println("\nTrie statistics:");
        System.out.println("  Total nodes: " + trie.nodeCount);
        System.out.println("  Total words: " + trie.wordCount);
        
        System.out.println("\nSearching:");
        
        String[] searches = {"apple", "app", "banana", "orange"};
        for (String word : searches) {
            String result = trie.search(word);
            if (result != null) {
                System.out.println("  '" + word + "' found: " + result + " ✓");
            } else {
                System.out.println("  '" + word + "' not found ✗");
            }
        }
        
        System.out.println("\nPrefix matches for 'app':");
        List<String> matches = trie.prefixSearch("app");
        for (String match : matches) {
            System.out.println("  " + match);
        }
    }
    
    static void ipRoutingDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 2: IP ROUTING TABLE (Longest Prefix Match)           ║");
        System.out.println("║ (Real network application)                                   ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Trie routingTable = new Trie();
        
        System.out.println("\nBuilding routing table (CIDR blocks):");
        
        routingTable.insert("10", "Gateway A");
        routingTable.insert("10.0", "Gateway B");
        routingTable.insert("10.0.0", "Gateway C");
        routingTable.insert("192.168", "Gateway D");
        routingTable.insert("172.16", "Gateway E");
        
        System.out.println("  10 → Gateway A");
        System.out.println("  10.0 → Gateway B");
        System.out.println("  10.0.0 → Gateway C");
        System.out.println("  192.168 → Gateway D");
        System.out.println("  172.16 → Gateway E");
        
        System.out.println("\nLongest prefix matching (routing lookups):");
        
        String[] ipAddresses = {"10.0.0.1", "10.0.5.1", "192.168.1.1", "172.16.0.1"};
        
        for (String ip : ipAddresses) {
            String gateway = routingTable.longestPrefixMatch(ip);
            if (gateway != null) {
                System.out.println("  IP " + ip + " → " + gateway);
            } else {
                System.out.println("  IP " + ip + " no match");
            }
        }
        
        System.out.println("\nLongest prefix match is critical for routing:");
        System.out.println("- More specific routes take precedence");
        System.out.println("- 10.0.0.0/24 is more specific than 10.0.0.0/16");
        System.out.println("- Trie efficiently finds longest match");
    }
    
    static void autocompleteDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 3: AUTO-COMPLETE (Prefix Completion)                 ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Trie dict = new Trie();
        
        System.out.println("\nDictionary words:");
        
        String[] words = {
            "network", "networking", "networked",
            "internet", "intercom", "intermediate",
            "packet", "packing", "packaged"
        };
        
        for (String word : words) {
            dict.insert(word);
            System.out.println("  " + word);
        }
        
        System.out.println("\nAuto-complete suggestions:");
        
        String[] prefixes = {"net", "inter", "pack"};
        
        for (String prefix : prefixes) {
            System.out.println("\nUser typed: '" + prefix + "'");
            List<String> suggestions = dict.prefixSearch(prefix);
            System.out.print("  Suggestions:");
            for (String suggestion : suggestions) {
                System.out.print(" " + suggestion);
            }
            System.out.println();
        }
    }
    
    static void dnsDemo() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ EXAMPLE 4: DNS NAME RESOLUTION                              ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        Trie dnsCache = new Trie();
        
        System.out.println("\nDNS records (hierarchical):");
        
        dnsCache.insert("com", "TLD - Commercial");
        dnsCache.insert("com.google", "142.250.80.46");
        dnsCache.insert("com.facebook", "31.13.64.35");
        dnsCache.insert("org", "TLD - Organization");
        dnsCache.insert("org.wikipedia", "103.102.166.224");
        
        System.out.println("  com → TLD");
        System.out.println("  com.google → 142.250.80.46");
        System.out.println("  com.facebook → 31.13.64.35");
        System.out.println("  org → TLD");
        System.out.println("  org.wikipedia → 103.102.166.224");
        
        System.out.println("\nDNS lookups:");
        
        String[] domains = {"com", "com.google", "com.microsoft", "org.apache"};
        
        for (String domain : domains) {
            String result = dnsCache.search(domain);
            if (result != null && !result.isEmpty()) {
                System.out.println("  " + domain + " → " + result);
            } else {
                System.out.println("  " + domain + " not in cache");
            }
        }
        
        System.out.println("\nBenefit: Hierarchical DNS uses Trie-like structure");
    }
    
    static void complexityAnalysis() {
        System.out.println("\n\n╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ COMPLEXITY ANALYSIS                                            ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nTrie Operations (m = key length, k = result count):");
        System.out.println("- Insert: O(m)");
        System.out.println("- Search: O(m)");
        System.out.println("- Delete: O(m)");
        System.out.println("- Prefix search: O(m + k)");
        System.out.println("- Longest prefix match: O(m)");
        System.out.println("- Space: O(alphabet_size * total_chars)\n");
        
        System.out.println("Comparison with other structures:");
        System.out.println("Operation      | Hash Table | BST        | Trie");
        System.out.println("---------------|------------|------------|------");
        System.out.println("Search         | O(1)       | O(log n)   | O(m)");
        System.out.println("Prefix search  | Impossible | O(log n+k) | O(m+k)");
        System.out.println("Longest prefix | Impossible | Difficult  | O(m) ✓");
        System.out.println("\nTrie winner for: Prefix-based operations!");
    }
    
    public static void main(String[] args) {
        System.out.println("╔════════════════════════════════════════════════════════════════╗");
        System.out.println("║ TRIE (PREFIX TREE) - JAVA COMPREHENSIVE DEMONSTRATION       ║");
        System.out.println("╚════════════════════════════════════════════════════════════════╝");
        
        System.out.println("\nTrie: Tree optimized for prefix-based operations");
        System.out.println("Key insight: Trade space for fast prefix matching");
        System.out.println("Essential for: IP routing, auto-complete, spell checking\n");
        
        basicTrieDemo();
        ipRoutingDemo();
        autocompleteDemo();
        dnsDemo();
        complexityAnalysis();
        
        System.out.println("\nKEY TAKEAWAYS:");
        System.out.println("1. Trie: O(m) operations where m = key length");
        System.out.println("2. Perfect for prefix queries (hash tables can't do this!)");
        System.out.println("3. Longest prefix match: Essential for IP routing");
        System.out.println("4. Hierarchical nature: Perfect for DNS");
        System.out.println("5. Used in: DNS, IP routing, auto-complete");
    }
}
