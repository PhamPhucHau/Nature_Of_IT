#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int binarySearch(int arr[], int n, int target) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid] == target) return mid;
        else if (arr[mid] < target) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

int findFirst(int arr[], int n, int target) {
    int left = 0, right = n - 1, result = -1;
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

void printArray(int arr[], int n) {
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");
}

int main() {
    printf("===== BINARY SEARCH =====\n\n");
    
    int arr[] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    printf("Array: ");
    printArray(arr, n);
    printf("\n");
    
    printf("--- Test: Find 7 ---\n");
    int idx = binarySearch(arr, n, 7);
    printf("Result: Index %d\n\n", idx);
    
    printf("--- Test: Find 6 (not found) ---\n");
    idx = binarySearch(arr, n, 6);
    printf("Result: Index %d\n\n", idx);
    
    printf("--- Test: Duplicates ---\n");
    int arr2[] = {1, 2, 2, 2, 3, 4, 4, 5};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printArray(arr2, n2);
    printf("First 2: %d, Last 2: %d\n\n", findFirst(arr2, n2, 2), 2);
    
    printf("=== SUMMARY ===\n");
    printf("Time: O(log n)\n");
    printf("Space: O(1)\n");
    printf("Network: DNS lookup, routing table\n");
    
    return 0;
}
