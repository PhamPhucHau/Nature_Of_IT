/**
 * DATA STRUCTURE: Linked List (Danh sách liên kết)
 * 
 * JAVA VERSION
 * 
 * STORY:
 * Linked List là dòng ghi chú dán nhau với keo.
 * Mỗi ghi chú có:
 * - Nội dung (data)
 * - Con trỏ tới ghi chú tiếp theo (next)
 * 
 * Ưu điểm:
 * - Thêm/Xóa ở đầu: O(1)
 * - Không cần biết kích thước trước
 * 
 * Nhược điểm:
 * - Truy cập random: O(n)
 * - Dùng thêm bộ nhớ cho con trỏ
 * - Cache locality kém
 */

import java.util.LinkedList;
import java.util.List;

public class LinkedListDemo {
    
    /**
     * Ví dụ 1: Sử dụng Java LinkedList cơ bản
     */
    public static void basicUsage() {
        System.out.println("=== BASIC LINKED LIST USAGE ===\n");
        
        List<Integer> list = new LinkedList<>();
        
        // Thêm phần tử vào cuối
        System.out.println("--- Thêm vào cuối ---");
        list.add(10);
        list.add(20);
        list.add(30);
        System.out.println("List: " + list + "\n");
        
        // Thêm vào đầu (LinkedList có addFirst)
        System.out.println("--- Thêm vào đầu ---");
        ((LinkedList<Integer>) list).addFirst(5);
        System.out.println("List: " + list + "\n");
        
        // Truy cập
        System.out.println("--- Truy cập ---");
        System.out.println("Element tại index 0: " + list.get(0));
        System.out.println("Element tại index 2: " + list.get(2) + "\n");
        
        // Xóa đầu (O(1))
        System.out.println("--- Xóa đầu (O(1)) ---");
        int removed = ((LinkedList<Integer>) list).removeFirst();
        System.out.println("Xóa: " + removed);
        System.out.println("List: " + list + "\n");
    }
    
    /**
     * Ví dụ 2: Tạo Custom LinkedList để hiểu cơ chế
     */
    static class CustomLinkedList<T> {
        private Node<T> head;
        private int size;
        
        static class Node<T> {
            T data;
            Node<T> next;
            
            Node(T data) {
                this.data = data;
                this.next = null;
            }
        }
        
        public CustomLinkedList() {
            this.head = null;
            this.size = 0;
        }
        
        /**
         * Thêm vào đầu - O(1)
         */
        public void addFirst(T data) {
            Node<T> newNode = new Node<>(data);
            newNode.next = head;
            head = newNode;
            size++;
        }
        
        /**
         * Thêm vào cuối - O(n)
         */
        public void addLast(T data) {
            Node<T> newNode = new Node<>(data);
            
            if (head == null) {
                head = newNode;
            } else {
                Node<T> current = head;
                while (current.next != null) {
                    current = current.next;
                }
                current.next = newNode;
            }
            size++;
        }
        
        /**
         * Lấy phần tử tại index - O(n)
         */
        public T get(int index) {
            if (index < 0 || index >= size) {
                throw new IndexOutOfBoundsException();
            }
            
            Node<T> current = head;
            for (int i = 0; i < index; i++) {
                current = current.next;
            }
            return current.data;
        }
        
        /**
         * Xóa phần tử đầu - O(1)
         */
        public T removeFirst() {
            if (head == null) {
                throw new RuntimeException("List is empty");
            }
            
            T data = head.data;
            head = head.next;
            size--;
            return data;
        }
        
        /**
         * Xóa tại vị trí - O(n)
         */
        public T remove(int index) {
            if (index < 0 || index >= size) {
                throw new IndexOutOfBoundsException();
            }
            
            T data;
            if (index == 0) {
                data = head.data;
                head = head.next;
            } else {
                Node<T> prev = head;
                for (int i = 0; i < index - 1; i++) {
                    prev = prev.next;
                }
                data = prev.next.data;
                prev.next = prev.next.next;
            }
            size--;
            return data;
        }
        
        public int size() {
            return size;
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder("[");
            Node<T> current = head;
            while (current != null) {
                sb.append(current.data);
                if (current.next != null) sb.append(" → ");
                current = current.next;
            }
            sb.append("]");
            return sb.toString();
        }
    }
    
    /**
     * Ví dụ 3: Demo Custom LinkedList
     */
    public static void customLinkedListDemo() {
        System.out.println("\n=== CUSTOM LINKED LIST DEMO ===\n");
        
        CustomLinkedList<Integer> list = new CustomLinkedList<>();
        
        System.out.println("--- Thêm vào cuối ---");
        for (int i = 1; i <= 3; i++) {
            list.addLast(i * 10);
            System.out.println(list);
        }
        
        System.out.println("\n--- Thêm vào đầu (O(1)) ---");
        list.addFirst(5);
        System.out.println(list);
        
        System.out.println("\n--- Xóa đầu (O(1)) ---");
        int removed = list.removeFirst();
        System.out.println("Xóa: " + removed);
        System.out.println(list);
    }
    
    /**
     * Ví dụ 4: So sánh ArrayList vs LinkedList
     */
    public static void performanceComparison() {
        System.out.println("\n=== PERFORMANCE: ARRAYLIST vs LINKEDLIST ===\n");
        
        int n = 100_000;
        
        // Test 1: Add at end
        System.out.println("--- Add at end (ArrayList vs LinkedList) ---");
        
        long start = System.nanoTime();
        List<Integer> arrayList = new java.util.ArrayList<>();
        for (int i = 0; i < n; i++) {
            arrayList.add(i);
        }
        long arrayListTime = System.nanoTime() - start;
        System.out.println("ArrayList:  " + arrayListTime / 1_000_000.0 + " ms");
        
        start = System.nanoTime();
        List<Integer> linkedList = new LinkedList<>();
        for (int i = 0; i < n; i++) {
            linkedList.add(i);
        }
        long linkedListTime = System.nanoTime() - start;
        System.out.println("LinkedList: " + linkedListTime / 1_000_000.0 + " ms\n");
        
        // Test 2: Remove at front
        System.out.println("--- Remove at front (ArrayList vs LinkedList) ---");
        
        start = System.nanoTime();
        for (int i = 0; i < n / 100; i++) {
            arrayList.remove(0);
        }
        arrayListTime = System.nanoTime() - start;
        System.out.println("ArrayList:  " + arrayListTime / 1_000_000.0 + " ms");
        
        start = System.nanoTime();
        LinkedList<Integer> linked = (LinkedList<Integer>) linkedList;
        for (int i = 0; i < n / 100; i++) {
            linked.removeFirst();
        }
        linkedListTime = System.nanoTime() - start;
        System.out.println("LinkedList: " + linkedListTime / 1_000_000.0 + " ms\n");
    }
    
    /**
     * Ví dụ 5: Ứng dụng - TCP Connection Queue
     */
    static class TCPConnectionQueue {
        private LinkedList<String> connections = new LinkedList<>();
        
        public void newConnection(String clientIP) {
            connections.add(clientIP);
            System.out.println("  + New connection from: " + clientIP);
        }
        
        public String processConnection() {
            if (connections.isEmpty()) {
                return null;
            }
            String client = connections.removeFirst();  // O(1)
            System.out.println("  - Processing: " + client);
            return client;
        }
        
        public int getPendingCount() {
            return connections.size();
        }
    }
    
    public static void networkApplicationDemo() {
        System.out.println("\n=== NETWORK APPLICATION DEMO ===\n");
        
        TCPConnectionQueue queue = new TCPConnectionQueue();
        
        System.out.println("--- Incoming connections ---");
        queue.newConnection("192.168.1.1");
        queue.newConnection("192.168.1.2");
        queue.newConnection("192.168.1.3");
        System.out.println("Pending: " + queue.getPendingCount() + "\n");
        
        System.out.println("--- Processing connections ---");
        queue.processConnection();
        queue.processConnection();
        System.out.println("Remaining: " + queue.getPendingCount() + "\n");
    }
    
    /**
     * ============================================================
     * TÓM TẮT ĐỘ PHỨC TẠP
     * ============================================================
     * 
     * | Phép toán      | ArrayList | LinkedList |
     * |----------------|-----------|------------|
     * | get(index)     | O(1)      | O(n)       |
     * | add(last)      | O(1)*     | O(1)       |
     * | add(first)     | O(n)      | O(1) ← ✓   |
     * | remove(index)  | O(n)      | O(n)       |
     * | remove(first)  | O(n)      | O(1) ← ✓   |
     * | space          | Compact   | +overhead  |
     * 
     * * ArrayList is O(1) amortized (considering resize cost)
     */
    
    public static void main(String[] args) {
        basicUsage();
        customLinkedListDemo();
        performanceComparison();
        networkApplicationDemo();
    }
}

/*
 * ============================================================
 * KHI NÀO DÙNG?
 * ============================================================
 * 
 * ArrayList:
 * - Cần truy cập random nhiều
 * - Thêm/xóa chủ yếu ở cuối
 * - Muốn cache locality tốt
 * 
 * LinkedList:
 * - Thêm/xóa ở đầu nhiều
 * - Không cần truy cập random
 * - Kích thước thay đổi nhiều
 * - Network queue: connection, packet queue
 * 
 * ============================================================
 */
