/**
 * DATA STRUCTURES: Stack & Queue
 * 
 * JAVA VERSION
 * 
 * STORY:
 * Stack: LIFO - Last In, First Out (chồng đĩa)
 * Queue: FIFO - First In, First Out (hàng chờ)
 * 
 * JAVA COLLECTIONS:
 * - Stack<T>: Legacy, mở rộng Vector
 * - Deque<T>: Interface, tốt hơn (double-ended queue)
 * - Queue<T>: Interface
 */

import java.util.*;

public class StackQueueDemo {
    
    /**
     * ============================================================
     * PHẦN 1: STACK
     * ============================================================
     */
    
    public static void stackBasics() {
        System.out.println("=== STACK BASICS ===\n");
        
        // Sử dụng Deque thay vì Stack (hiệu năng tốt hơn)
        Deque<Integer> stack = new ArrayDeque<>();
        
        System.out.println("--- Push ---");
        stack.push(10);
        stack.push(20);
        stack.push(30);
        System.out.println("Stack: " + stack + "\n");
        
        System.out.println("--- Peek (không xóa) ---");
        System.out.println("Top: " + stack.peek());
        System.out.println("Stack: " + stack + "\n");
        
        System.out.println("--- Pop (xóa và trả về) ---");
        int top = stack.pop();
        System.out.println("Popped: " + top);
        System.out.println("Stack: " + stack + "\n");
    }
    
    /**
     * Custom Stack implementation để hiểu cơ chế
     */
    static class CustomStack<T> {
        private Node<T> top;
        private int size;
        
        static class Node<T> {
            T data;
            Node<T> next;
            
            Node(T data) {
                this.data = data;
            }
        }
        
        public CustomStack() {
            this.top = null;
            this.size = 0;
        }
        
        /**
         * Push: Thêm vào đầu - O(1)
         */
        public void push(T data) {
            Node<T> newNode = new Node<>(data);
            newNode.next = top;
            top = newNode;
            size++;
        }
        
        /**
         * Pop: Lấy từ đầu - O(1)
         */
        public T pop() {
            if (isEmpty()) {
                throw new RuntimeException("Stack is empty");
            }
            T data = top.data;
            top = top.next;
            size--;
            return data;
        }
        
        /**
         * Peek: Nhìn mà không xóa - O(1)
         */
        public T peek() {
            if (isEmpty()) {
                throw new RuntimeException("Stack is empty");
            }
            return top.data;
        }
        
        public boolean isEmpty() {
            return size == 0;
        }
        
        public int size() {
            return size;
        }
    }
    
    /**
     * ============================================================
     * PHẦN 2: QUEUE
     * ============================================================
     */
    
    public static void queueBasics() {
        System.out.println("\n=== QUEUE BASICS ===\n");
        
        Queue<Integer> queue = new LinkedList<>();
        
        System.out.println("--- Enqueue (add) ---");
        queue.add(10);
        queue.add(20);
        queue.add(30);
        System.out.println("Queue: " + queue + "\n");
        
        System.out.println("--- Peek (không xóa) ---");
        System.out.println("Front: " + queue.peek());
        System.out.println("Queue: " + queue + "\n");
        
        System.out.println("--- Dequeue (poll) ---");
        int front = queue.poll();
        System.out.println("Removed: " + front);
        System.out.println("Queue: " + queue + "\n");
    }
    
    /**
     * Custom Queue implementation
     */
    static class CustomQueue<T> {
        private Node<T> front;
        private Node<T> rear;
        private int size;
        
        static class Node<T> {
            T data;
            Node<T> next;
            
            Node(T data) {
                this.data = data;
            }
        }
        
        public CustomQueue() {
            this.front = null;
            this.rear = null;
            this.size = 0;
        }
        
        /**
         * Enqueue: Thêm vào cuối - O(1)
         */
        public void enqueue(T data) {
            Node<T> newNode = new Node<>(data);
            
            if (rear == null) {
                front = rear = newNode;
            } else {
                rear.next = newNode;
                rear = newNode;
            }
            size++;
        }
        
        /**
         * Dequeue: Lấy từ đầu - O(1)
         */
        public T dequeue() {
            if (isEmpty()) {
                throw new RuntimeException("Queue is empty");
            }
            
            T data = front.data;
            front = front.next;
            
            if (front == null) {
                rear = null;
            }
            
            size--;
            return data;
        }
        
        /**
         * Peek: Nhìn phần tử đầu - O(1)
         */
        public T peek() {
            if (isEmpty()) {
                throw new RuntimeException("Queue is empty");
            }
            return front.data;
        }
        
        public boolean isEmpty() {
            return size == 0;
        }
        
        public int size() {
            return size;
        }
    }
    
    /**
     * ============================================================
     * PHẦN 3: ỨNG DỤNG - BALANCED PARENTHESES (STACK)
     * ============================================================
     * 
     * Bài toán: Kiểm tra xem dấu ngoặc có cân bằng không
     * Ví dụ: \"({[()]})\" → Valid
     *        \"({[}])\"   → Invalid\n     */\n    \n    public static boolean isBalanced(String str) {\n        Stack<Character> stack = new Stack<>();\n        \n        for (char c : str.toCharArray()) {\n            if (c == '(' || c == '{' || c == '[') {\n                stack.push(c);\n            } else if (c == ')' || c == '}' || c == ']') {\n                if (stack.isEmpty()) return false;\n                \n                char open = stack.pop();\n                if (!matches(open, c)) return false;\n            }\n        }\n        \n        return stack.isEmpty();\n    }\n    \n    private static boolean matches(char open, char close) {\n        return (open == '(' && close == ')') ||\n               (open == '{' && close == '}') ||\n               (open == '[' && close == ']');\n    }\n    \n    public static void balancedParenthesesDemo() {\n        System.out.println(\"=== BALANCED PARENTHESES (STACK APPLICATION) ===\\n\");\n        \n        String[] tests = {\n            \"({[()]})\",\n            \"({[})]\",\n            \"([{}])\",\n            \"(([]){})\",\n            \"(((\"\n        };\n        \n        for (String test : tests) {\n            boolean valid = isBalanced(test);\n            System.out.println(test + \" → \" + (valid ? \"✓ Valid\" : \"✗ Invalid\"));\n        }\n    }\n    \n    /**\n     * ============================================================\n     * PHẦN 4: ỨNG DỤNG - SIMPLE WEB SERVER QUEUE\n     * ============================================================\n     */\n    \n    static class HTTPRequest {\n        String method;\n        String path;\n        String clientIP;\n        \n        HTTPRequest(String method, String path, String clientIP) {\n            this.method = method;\n            this.path = path;\n            this.clientIP = clientIP;\n        }\n        \n        @Override\n        public String toString() {\n            return method + \" \" + path + \" from \" + clientIP;\n        }\n    }\n    \n    static class WebServer {\n        private Queue<HTTPRequest> requestQueue = new LinkedList<>();\n        \n        public void receiveRequest(HTTPRequest req) {\n            requestQueue.add(req);\n            System.out.println(\"  ← Received: \" + req);\n        }\n        \n        public void processRequest() {\n            if (requestQueue.isEmpty()) {\n                System.out.println(\"  No requests to process\");\n                return;\n            }\n            \n            HTTPRequest req = requestQueue.poll();\n            System.out.println(\"  → Processing: \" + req);\n        }\n        \n        public int getPendingRequests() {\n            return requestQueue.size();\n        }\n    }\n    \n    public static void webServerDemo() {\n        System.out.println(\"\\n=== WEB SERVER REQUEST QUEUE (QUEUE APPLICATION) ===\\n\");\n        \n        WebServer server = new WebServer();\n        \n        System.out.println(\"--- Incoming requests ---\");\n        server.receiveRequest(new HTTPRequest(\"GET\", \"/index.html\", \"192.168.1.1\"));\n        server.receiveRequest(new HTTPRequest(\"POST\", \"/api/users\", \"192.168.1.2\"));\n        server.receiveRequest(new HTTPRequest(\"GET\", \"/images/logo.png\", \"192.168.1.3\"));\n        System.out.println(\"Pending: \" + server.getPendingRequests() + \"\\n\");\n        \n        System.out.println(\"--- Processing requests (FIFO) ---\");\n        server.processRequest();\n        server.processRequest();\n        System.out.println(\"Remaining: \" + server.getPendingRequests() + \"\\n\");\n    }\n    \n    /**\n     * ============================================================\n     * PHẦN 5: PERFORMANCE COMPARISON\n     * ============================================================\n     */\n    \n    public static void performanceComparison() {\n        System.out.println(\"=== PERFORMANCE: STACK vs QUEUE ===\\n\");\n        \n        int n = 1_000_000;\n        \n        // Stack test\n        System.out.println(\"--- Stack (Push/Pop) ---\");\n        Deque<Integer> stack = new ArrayDeque<>();\n        long start = System.nanoTime();\n        for (int i = 0; i < n; i++) {\n            stack.push(i);\n        }\n        for (int i = 0; i < n; i++) {\n            stack.pop();\n        }\n        long stackTime = System.nanoTime() - start;\n        System.out.println(\"Time: \" + stackTime / 1_000_000.0 + \" ms\\n\");\n        \n        // Queue test\n        System.out.println(\"--- Queue (Add/Poll) ---\");\n        Queue<Integer> queue = new LinkedList<>();\n        start = System.nanoTime();\n        for (int i = 0; i < n; i++) {\n            queue.add(i);\n        }\n        for (int i = 0; i < n; i++) {\n            queue.poll();\n        }\n        long queueTime = System.nanoTime() - start;\n        System.out.println(\"Time: \" + queueTime / 1_000_000.0 + \" ms\\n\");\n        \n        System.out.println(\"Both O(1) operations, so performance similar\");\n    }\n    \n    /**\n     * ============================================================\n     * TÓM TẮT ĐỘ PHỨC TẠP\n     * ============================================================\n     * \n     * | Phép toán  | Stack | Queue |\n     * |------------|-------|-------|\n     * | push/enq   | O(1)  | O(1)  |\n     * | pop/deq    | O(1)  | O(1)  |\n     * | peek       | O(1)  | O(1)  |\n     * \n     * ============================================================\n     */\n    \n    public static void main(String[] args) {\n        stackBasics();\n        queueBasics();\n        balancedParenthesesDemo();\n        webServerDemo();\n        performanceComparison();\n        \n        System.out.println(\"\\n=== SUMMARY ===\\n\");\n        System.out.println(\"STACK (LIFO):\");\n        System.out.println(\"  Operations: push, pop, peek → all O(1)\");\n        System.out.println(\"  Applications: undo/redo, parsing, DFS, backtracking\\n\");\n        \n        System.out.println(\"QUEUE (FIFO):\");\n        System.out.println(\"  Operations: enqueue, dequeue, peek → all O(1)\");\n        System.out.println(\"  Applications: scheduling, BFS, packet queue\\n\");\n    }\n}\n\n/*\n * ============================================================\n * NETWORK APPLICATION EXAMPLES\n * ============================================================\n * \n * STACK:\n * - HTTP/HTTPS protocol parsing\n * - DFS routing algorithm\n * - Expression evaluation in parsers\n * \n * QUEUE:\n * - Packet buffer queue at NIC\n * - TCP SYN queue (listen backlog)\n * - Message queue (RabbitMQ, Kafka)\n * - Scheduler queue in OS kernel\n * - Load balancer request queue\n * \n * ============================================================\n */\n