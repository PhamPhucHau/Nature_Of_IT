import java.util.Arrays;

public class BinarySearchDemo {
    
    public static int binarySearch(int[] arr, int target) {
        int left = 0, right = arr.length - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (arr[mid] == target) return mid;
            else if (arr[mid] < target) left = mid + 1;
            else right = mid - 1;
        }
        return -1;
    }
    
    public static int findFirst(int[] arr, int target) {
        int left = 0, right = arr.length - 1, result = -1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (arr[mid] == target) {
                result = mid;
                right = mid - 1;
            } else if (arr[mid] < target) left = mid + 1;
            else right = mid - 1;
        }
        return result;
    }
    
    public static void main(String[] args) {
        System.out.println("===== BINARY SEARCH =====\n");
        
        int[] arr = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
        System.out.println("Array: " + Arrays.toString(arr) + "\n");
        
        System.out.println("--- Test: Find 7 ---");
        int idx = binarySearch(arr, 7);
        System.out.println("Result: Index " + idx + "\n");
        
        System.out.println("--- Test: Find 6 (not found) ---");
        idx = binarySearch(arr, 6);
        System.out.println("Result: Index " + idx + "\n");
        
        System.out.println("--- Test: Duplicates ---");
        int[] arr2 = {1, 2, 2, 2, 3, 4, 4, 5};
        System.out.println(Arrays.toString(arr2));
        System.out.println("First 2: " + findFirst(arr2, 2) + ", Last 2 would need findLast\n");
        
        System.out.println("=== SUMMARY ===");
        System.out.println("Time: O(log n)");
        System.out.println("Space: O(1)");
        System.out.println("Network: DNS lookup, routing table");
    }
}
