#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Quick Sort */
int partition(int arr[], int low, int high) {
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

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

/* Merge Sort */
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));
    
    memcpy(L, &arr[left], n1 * sizeof(int));
    memcpy(R, &arr[mid + 1], n2 * sizeof(int));
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    
    free(L);
    free(R);
}

void mergeSort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void printArray(int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

int isSorted(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i] < arr[i - 1]) return 0;
    }
    return 1;
}

int main() {
    printf("===== SORTING ALGORITHMS =====\n\n");
    
    int arr1[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
    int arr2[] = {64, 34, 25, 12, 22, 11, 90, 88, 45, 50};
    int n = sizeof(arr1) / sizeof(arr1[0]);
    
    printf("Original: ");
    printArray(arr1, n);
    printf("\n");
    
    /* Quick Sort */
    printf("--- QUICK SORT ---\n");
    clock_t start = clock();
    quickSort(arr1, 0, n - 1);
    clock_t end = clock();
    printf("Sorted: ");
    printArray(arr1, n);
    printf("Valid: %s\n", isSorted(arr1, n) ? "YES" : "NO");
    printf("Time: %.3f ms\n\n", ((double)(end - start)) / CLOCKS_PER_SEC * 1000);
    
    /* Merge Sort */
    printf("--- MERGE SORT ---\n");
    start = clock();
    mergeSort(arr2, 0, n - 1);
    end = clock();
    printf("Sorted: ");
    printArray(arr2, n);
    printf("Valid: %s\n", isSorted(arr2, n) ? "YES" : "NO");
    printf("Time: %.3f ms\n\n", ((double)(end - start)) / CLOCKS_PER_SEC * 1000);
    
    /* Edge cases */
    printf("--- EDGE CASES ---\n");
    
    int sorted[] = {1, 2, 3, 4, 5};
    int n2 = 5;
    printf("Already sorted: ");
    printArray(sorted, n2);
    quickSort(sorted, 0, n2 - 1);
    printf("After sort: ");
    printArray(sorted, n2);
    printf("\n");
    
    int reverse[] = {5, 4, 3, 2, 1};
    printf("Reverse sorted: ");
    printArray(reverse, n2);
    mergeSort(reverse, 0, n2 - 1);
    printf("After sort: ");
    printArray(reverse, n2);
    printf("\n");
    
    /* Performance test */
    printf("--- PERFORMANCE TEST ---\n");
    int size = 50000;
    int *perfArr1 = (int *)malloc(size * sizeof(int));
    int *perfArr2 = (int *)malloc(size * sizeof(int));
    
    srand(42);
    for (int i = 0; i < size; i++) {
        perfArr1[i] = rand() % 10000;
        perfArr2[i] = perfArr1[i];
    }
    
    start = clock();
    quickSort(perfArr1, 0, size - 1);
    end = clock();
    double quickTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    start = clock();
    mergeSort(perfArr2, 0, size - 1);
    end = clock();
    double mergeTime = ((double)(end - start)) / CLOCKS_PER_SEC * 1000;
    
    printf("Sorting %d elements:\n", size);
    printf("  Quick Sort: %.3f ms\n", quickTime);
    printf("  Merge Sort: %.3f ms\n\n", mergeTime);
    
    free(perfArr1);
    free(perfArr2);
    
    printf("=== SUMMARY ===\n");
    printf("Quick Sort:  O(n log n) avg, O(n²) worst, O(log n) space\n");
    printf("Merge Sort:  O(n log n) always, O(n) space\n");
    printf("Network App: QoS priority, routing metrics\n");
    
    return 0;
}
