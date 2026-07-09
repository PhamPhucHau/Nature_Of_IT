import java.util.Arrays;
import java.util.Random;

public class SortingDemo {
    
    /* Quick Sort */
    static int partition(int[] arr, int low, int high) {
        int pivot = arr[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (arr[j] < pivot) {
                i++;
                int temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
        int temp = arr[i + 1];
        arr[i + 1] = arr[high];
        arr[high] = temp;
        return i + 1;
    }
    
    static void quickSort(int[] arr) {
        if (arr != null && arr.length > 0) {
            quickSort(arr, 0, arr.length - 1);
        }
    }
    
    static void quickSort(int[] arr, int low, int high) {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi - 1);
            quickSort(arr, pi + 1, high);
        }
    }
    
    /* Merge Sort */
    static void merge(int[] arr, int left, int mid, int right) {
        int n1 = mid - left + 1;
        int n2 = right - mid;
        
        int[] L = new int[n1];
        int[] R = new int[n2];
        
        System.arraycopy(arr, left, L, 0, n1);
        System.arraycopy(arr, mid + 1, R, 0, n2);
        
        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            if (L[i] <= R[j]) arr[k++] = L[i++];
            else arr[k++] = R[j++];
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];
    }
    
    static void mergeSort(int[] arr) {
        if (arr != null && arr.length > 0) {
            mergeSort(arr, 0, arr.length - 1);
        }
    }
    
    static void mergeSort(int[] arr, int left, int right) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(arr, left, mid);
            mergeSort(arr, mid + 1, right);
            merge(arr, left, mid, right);
        }
    }
    
    static boolean isSorted(int[] arr) {
        for (int i = 1; i < arr.length; i++) {
            if (arr[i] < arr[i - 1]) return false;
        }
        return true;
    }
    
    public static void main(String[] args) {
        System.out.println("===== SORTING ALGORITHMS =====\n");
        
        int[] arr1 = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
        int[] arr2 = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
        
        System.out.println("Original: " + Arrays.toString(arr1) + "\n");
        
        /* Quick Sort */
        System.out.println("--- QUICK SORT ---");
        long start = System.nanoTime();
        quickSort(arr1);
        long quickTime = System.nanoTime() - start;
        System.out.println("Sorted: " + Arrays.toString(arr1));
        System.out.println("Valid: " + (isSorted(arr1) ? "YES" : "NO"));
        System.out.println("Time: " + String.format("%.3f ms\n", quickTime / 1_000_000.0));
        
        /* Merge Sort */
        System.out.println("--- MERGE SORT ---");
        start = System.nanoTime();
        mergeSort(arr2);
        long mergeTime = System.nanoTime() - start;
        System.out.println("Sorted: " + Arrays.toString(arr2));
        System.out.println("Valid: " + (isSorted(arr2) ? "YES" : "NO"));
        System.out.println("Time: " + String.format("%.3f ms\n", mergeTime / 1_000_000.0));
        
        /* Edge cases */
        System.out.println("--- EDGE CASES ---");
        
        int[] sorted = {1, 2, 3, 4, 5};
        System.out.println("Already sorted: " + Arrays.toString(sorted));
        quickSort(sorted);
        System.out.println("After sort: " + Arrays.toString(sorted) + "\n");
        
        int[] reverse = {5, 4, 3, 2, 1};
        System.out.println("Reverse sorted: " + Arrays.toString(reverse));
        mergeSort(reverse);
        System.out.println("After sort: " + Arrays.toString(reverse) + "\n");
        
        int[] dups = {3, 1, 4, 1, 5, 9, 2, 6, 5};
        System.out.println("With duplicates: " + Arrays.toString(dups));
        quickSort(dups);
        System.out.println("After sort: " + Arrays.toString(dups) + "\n");
        
        /* Performance test */
        System.out.println("--- PERFORMANCE TEST ---");
        int[] sizes = {10000, 100000, 1000000};
        
        for (int size : sizes) {
            Random rand = new Random(42);
            int[] quickArr = new int[size];
            int[] mergeArr = new int[size];
            
            for (int i = 0; i < size; i++) {
                quickArr[i] = rand.nextInt(10000);
                mergeArr[i] = quickArr[i];
            }
            
            start = System.nanoTime();
            quickSort(quickArr);
            quickTime = System.nanoTime() - start;
            
            start = System.nanoTime();
            mergeSort(mergeArr);
            mergeTime = System.nanoTime() - start;
            
            System.out.println("Size: " + size);
            System.out.println("  Quick Sort: " + String.format("%.3f ms", quickTime / 1_000_000.0));
            System.out.println("  Merge Sort: " + String.format("%.3f ms\n", mergeTime / 1_000_000.0));
        }
        
        /* Network Application: QoS Scheduling */
        System.out.println("--- NETWORK APPLICATION: QoS Scheduling ---");
        String[] packets = {"VoIP(priority=5)", "BulkData(priority=1)", 
                           "Video(priority=4)", "Email(priority=2)"};
        int[] priorities = {5, 1, 4, 2};
        
        System.out.println("Before sorting:");
        for (int i = 0; i < packets.length; i++) {
            System.out.println("  " + packets[i]);
        }
        
        /* Simple sort by priority */
        for (int i = 0; i < priorities.length; i++) {
            for (int j = i + 1; j < priorities.length; j++) {
                if (priorities[j] > priorities[i]) {
                    int tempP = priorities[i];
                    priorities[i] = priorities[j];
                    priorities[j] = tempP;
                    
                    String tempPk = packets[i];
                    packets[i] = packets[j];
                    packets[j] = tempPk;
                }
            }
        }
        
        System.out.println("\nAfter sorting by priority (high to low):");
        for (int i = 0; i < packets.length; i++) {
            System.out.println("  " + packets[i]);
        }
        System.out.println("\nHigh priority packets processed first!\n");
        
        System.out.println("=== SUMMARY ===");
        System.out.println("Quick Sort:  O(n log n) avg, O(n²) worst, O(log n) space");
        System.out.println("Merge Sort:  O(n log n) always, O(n) space");
        System.out.println("Network App: QoS priority scheduling, routing metrics");
    }
}
