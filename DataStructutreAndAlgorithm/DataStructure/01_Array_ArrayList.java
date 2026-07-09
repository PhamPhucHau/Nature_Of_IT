/**
 * DATA STRUCTURE: Array / Dynamic Array (ArrayList)
 * 
 * JAVA VERSION
 * 
 * STORY:
 * Trong Java, bạn không cần lo về việc cấp phát bộ nhớ thủ công.
 * ArrayList tự động giúp bạn quản lý việc resize.
 * 
 * So sánh:
 * - C:    Bạn tự quản lý (malloc, free, resize)
 * - Java: Ngôn ngữ quản lý (dù phía sau cũng giống C)
 * 
 * Ưu điểm:
 * - Lập trình nhanh, ít lỗi bộ nhớ
 * 
 * Nhược điểm:
 * - Có Garbage Collection overhead (nhưng thường nhanh hơn việc bạn quản lý)
 */

import java.util.ArrayList;
import java.util.List;

public class DynamicArrayDemo {
    
    /**
     * Ví dụ 1: Sử dụng ArrayList cơ bản
     */
    public static void basicUsage() {
        System.out.println("=== BASIC ARRAYLIST USAGE ===\n");
        
        // Tạo ArrayList
        List<Integer> arr = new ArrayList<>();
        System.out.println("✓ Tạo ArrayList xong\n");
        
        // Thêm phần tử
        System.out.println("--- THÊM PHẦN TỬ ---");
        arr.add(10);
        arr.add(20);
        arr.add(30);
        System.out.println("Array sau khi thêm: " + arr);
        System.out.println("Size: " + arr.size() + "\n");
        
        // Truy cập
        System.out.println("--- TRUY CẬP ---");
        System.out.println("Element tại index 0: " + arr.get(0));
        System.out.println("Element tại index 2: " + arr.get(2) + "\n");
        
        // Xóa
        System.out.println("--- XOÁ ---");
        arr.remove(1);  // Xóa element tại index 1 (giá trị 20)
        System.out.println("Array sau khi xóa index 1: " + arr + "\n");
        
        // Kiểm tra
        System.out.println("--- KIỂM TRA ---");
        System.out.println("Có chứa 30? " + arr.contains(30));
        System.out.println("Vị trí của 30? " + arr.indexOf(30) + "\n");
    }
    
    /**
     * Ví dụ 2: Tạo Custom ArrayList để hiểu rõ hơn cơ chế resize
     * (giống như ở C)
     */
    static class CustomArrayList<T> {
        private Object[] data;
        private int size;
        private static final int INITIAL_CAPACITY = 10;
        
        public CustomArrayList() {
            this.data = new Object[INITIAL_CAPACITY];
            this.size = 0;
        }
        
        /**
         * Thêm phần tử vào cuối
         * 
         * Nếu size == capacity, resize
         */
        public void add(T value) {
            // Kiểm tra nếu cần resize
            if (size == data.length) {
                resize();
            }
            
            // Thêm phần tử
            data[size] = value;
            size++;
        }
        
        /**
         * Resize: Tăng capacity gấp đôi
         * 
         * Mô phỏng:
         * Capacity: 10 → 20 → 40 → 80 → ...
         */
        private void resize() {
            System.out.println("  → Resize! Capacity cũ: " + data.length);
            
            int newCapacity = data.length * 2;
            Object[] newData = new Object[newCapacity];
            
            // Copy dữ liệu cũ
            System.arraycopy(data, 0, newData, 0, size);
            
            data = newData;
            System.out.println("  → Resize xong. Capacity mới: " + data.length);
        }
        
        /**
         * Lấy phần tử tại vị trí index
         */
        @SuppressWarnings("unchecked")
        public T get(int index) {
            if (index < 0 || index >= size) {
                throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
            }
            return (T) data[index];
        }
        
        /**
         * Xóa phần tử tại vị trí index
         */
        @SuppressWarnings("unchecked")
        public T remove(int index) {
            if (index < 0 || index >= size) {
                throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
            }
            
            T value = (T) data[index];
            
            // Dịch các phần tử sau về phía trước
            System.arraycopy(data, index + 1, data, index, size - index - 1);
            
            size--;
            return value;
        }
        
        public int getSize() {
            return size;
        }
        
        public int getCapacity() {
            return data.length;
        }
        
        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder("[");
            for (int i = 0; i < size; i++) {
                sb.append(data[i]);
                if (i < size - 1) sb.append(", ");
            }
            sb.append("]");
            return sb.toString() + " (Size: " + size + ", Capacity: " + data.length + ")";
        }
    }
    
    /**
     * Ví dụ 3: Demo Custom ArrayList
     */
    public static void customArrayListDemo() {
        System.out.println("\n=== CUSTOM ARRAYLIST DEMO ===\n");
        
        CustomArrayList<Integer> arr = new CustomArrayList<>();
        
        System.out.println("--- THÊM PHẦN TỬ ---");
        for (int i = 1; i <= 15; i++) {
            System.out.print("Thêm " + (i * 10) + ": ");
            arr.add(i * 10);
            System.out.println(arr);
        }
        
        System.out.println("\n--- TRUY CẬP ---");
        System.out.println("Element tại index 0: " + arr.get(0));
        System.out.println("Element tại index 5: " + arr.get(5));
        
        System.out.println("\n--- XOÁ ---");
        System.out.println("Xóa index 2 (giá trị " + arr.get(2) + ")");
        arr.remove(2);
        System.out.println("Array sau khi xóa: " + arr);
    }
    
    /**
     * Ví dụ 4: So sánh độ hiệu năng (ArrayList vs Array)
     */
    public static void performanceComparison() {
        System.out.println("\n=== PERFORMANCE COMPARISON ===\n");
        
        int n = 1_000_000;
        
        // Thử nghiệm 1: ArrayList
        System.out.println("--- ArrayList (với resize) ---");
        long start = System.nanoTime();
        List<Integer> arrayList = new ArrayList<>();
        for (int i = 0; i < n; i++) {
            arrayList.add(i);
        }
        long end = System.nanoTime();
        System.out.println("Thời gian thêm " + n + " phần tử: " + (end - start) / 1_000_000.0 + " ms");
        
        // Thử nghiệm 2: Array cố định (phải biết kích thước trước)
        System.out.println("\n--- Array cố định ---");
        start = System.nanoTime();
        int[] array = new int[n];
        for (int i = 0; i < n; i++) {
            array[i] = i;
        }
        end = System.nanoTime();
        System.out.println("Thời gian gán " + n + " phần tử: " + (end - start) / 1_000_000.0 + " ms");
    }
    
    /**
     * Ví dụ 5: Ứng dụng thực tế - Lưu packet buffer
     */
    static class PacketBuffer {
        private List<String> packets = new ArrayList<>();
        
        public void receivePacket(String packet) {
            packets.add(packet);
        }
        
        public String getPacket(int index) {
            return packets.get(index);
        }
        
        public int getPacketCount() {
            return packets.size();
        }
        
        public void printAllPackets() {
            System.out.println("Total packets: " + packets.size());
            for (int i = 0; i < packets.size(); i++) {
                System.out.println("  Packet " + (i + 1) + ": " + packets.get(i));
            }
        }
    }
    
    public static void networkApplicationDemo() {
        System.out.println("\n=== NETWORK APPLICATION DEMO ===\n");
        
        PacketBuffer buffer = new PacketBuffer();
        
        System.out.println("--- Nhận packet ---");
        buffer.receivePacket("HTTP GET request");
        buffer.receivePacket("TCP SYN");
        buffer.receivePacket("UDP DNS query");
        buffer.receivePacket("TCP ACK");
        
        buffer.printAllPackets();
        
        System.out.println("\n--- Truy cập packet cụ thể ---");
        System.out.println("Packet thứ 2: " + buffer.getPacket(1));
    }
    
    /**
     * ============================================================
     * TÓM TẮT ĐỘ PHỨC TẠP
     * ============================================================
     * 
     * | Phép toán   | Độ phức tạp | Ghi chú                    |
     * |-------------|-------------|---------------------------|
     * | Get         | O(1)        | Truy cập trực tiếp         |
     * | Add (cuối)  | O(1)*       | * Amortized, resize O(n)   |
     * | Remove (k)  | O(n)        | Cần dịch các phần tử sau    |
     * | Search      | O(n)        | Phải kiểm tra từng phần tử  |
     * | Insert      | O(n)        | Cần dịch các phần tử sau    |
     * 
     * ============================================================
     */
    
    public static void main(String[] args) {
        basicUsage();
        customArrayListDemo();
        performanceComparison();
        networkApplicationDemo();
    }
}

/*
 * CHI PHÍ RESIZE:
 * 
 * Nếu tăng capacity từng cái một:
 *     1 + 2 + 3 + ... + n = n(n+1)/2 = O(n²)
 * 
 * Nếu tăng gấp đôi:
 *     1 + 2 + 4 + 8 + ... + n = 2n - 1 = O(n)
 *     Amortized: O(n) / n = O(1) per add
 * 
 * Đó là lý do ArrayList tăng gấp 1.5 hoặc 2 lần, không tăng từng cái một.
 */
