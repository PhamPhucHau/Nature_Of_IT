/**
 * DATA STRUCTURE: Array / Dynamic Array
 * 
 * STORY:
 * Array là cơ bản nhất. Khi bạn cần lưu nhiều dữ liệu cùng kiểu,
 * thay vì tạo 100 biến riêng, bạn tạo 1 mảng 100 phần tử.
 * 
 * Tại sao quan trọng?
 * - Trong network: lưu packet buffers, socket tables
 * - Truy cập: O(1) - cực nhanh
 * - Cạn: không thể thay đổi kích thước sau khi tạo
 * 
 * Giải pháp: Dynamic Array (ArrayList)
 * Khi hết chỗ, cấp phát bộ nhớ lớn hơn và copy dữ liệu cũ vào.
 * 
 * KIẾN TRÚC:
 * ┌─────────────────────────┐
 * │ data (con trỏ)          │  → liên tục trong RAM
 * │ capacity (kích thước)   │  → bao nhiêu phần tử có thể lưu
 * │ size (số phần tử hiện)  │  → bao nhiêu phần tử đang dùng
 * └─────────────────────────┘
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================
// 1. ĐỊNH NGHĨA CẤU TRÚC
// ============================================================

typedef struct {
    int *data;           // Con trỏ tới mảng nguyên số
    int size;            // Số phần tử hiện có
    int capacity;        // Sức chứa hiện tại
} DynamicArray;

// ============================================================
// 2. KHỞI TẠO
// ============================================================

/**
 * Tạo một dynamic array trống
 * 
 * Bước 1: Cấp phát bộ nhớ cho cấu trúc (struct)
 * Bước 2: Cấp phát bộ nhớ cho mảng dữ liệu ban đầu (capacity = 10)
 * Bước 3: Khởi tạo size = 0
 */
DynamicArray* createArray() {
    DynamicArray *arr = (DynamicArray *)malloc(sizeof(DynamicArray));
    
    arr->capacity = 10;
    arr->size = 0;
    arr->data = (int *)malloc(arr->capacity * sizeof(int));
    
    printf("✓ Tạo Array xong. Capacity: %d, Size: %d\n", arr->capacity, arr->size);
    return arr;
}

// ============================================================
// 3. RESIZE - ĐÁY LÀ CÁI CÓ PHẦN KHÓ
// ============================================================

/**
 * Khi hết chỗ, phải resize
 * 
 * Mô phỏng:
 * Trước: [1, 2, 3, _, _, _, _, _, _, _]  (capacity = 10, size = 3)
 * 
 * Sau:   [1, 2, 3, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _]
 *        (capacity = 20, size = 3)
 * 
 * Chi phí: O(n) vì phải copy tất cả phần tử cũ
 * Nhưng để amortized O(1), ta tăng gấp đôi, không tăng từng cái một
 */
void resize(DynamicArray *arr) {
    printf("  → Resize! Capacity cũ: %d\n", arr->capacity);
    
    // Tăng capacity gấp đôi
    int newCapacity = arr->capacity * 2;
    
    // Cấp phát bộ nhớ mới
    int *newData = (int *)malloc(newCapacity * sizeof(int));
    
    // Copy dữ liệu cũ sang bộ nhớ mới
    memcpy(newData, arr->data, arr->size * sizeof(int));
    
    // Giải phóng bộ nhớ cũ
    free(arr->data);
    
    // Cập nhật struct
    arr->data = newData;
    arr->capacity = newCapacity;
    
    printf("  → Resize xong. Capacity mới: %d\n", arr->capacity);
}

// ============================================================
// 4. THÊM PHẦN TỬ VÀO CUỐI
// ============================================================

/**
 * Push: Thêm phần tử vào cuối
 * 
 * if size == capacity:
 *     resize()
 * data[size] = value
 * size++
 * 
 * Độ phức tạp:
 * - Trường hợp bình thường: O(1)
 * - Khi phải resize: O(n)
 * - Amortized (trung bình): O(1)
 */
void push(DynamicArray *arr, int value) {
    // Nếu hết chỗ, phải resize
    if (arr->size == arr->capacity) {
        resize(arr);
    }
    
    // Thêm phần tử vào cuối
    arr->data[arr->size] = value;
    arr->size++;
    
    printf("  + Thêm %d. Size hiện: %d\n", value, arr->size);
}

// ============================================================
// 5. TRUY CẬP
// ============================================================

/**
 * Get: Lấy phần tử tại vị trí i
 * 
 * Độ phức tạp: O(1)
 * Vì RAM là tuần tự, biết địa chỉ đầu + offset là tìm được ngay
 */
int get(DynamicArray *arr, int index) {
    if (index < 0 || index >= arr->size) {
        printf("⚠ Lỗi: Index %d ngoài phạm vi [0, %d)\n", index, arr->size);
        return -1;
    }
    return arr->data[index];
}

// ============================================================
// 6. XOÁ PHẦN TỬ
// ============================================================

/**
 * Pop: Xóa phần tử cuối cùng
 * 
 * size--
 * 
 * Độ phức tạp: O(1)
 */
int pop(DynamicArray *arr) {
    if (arr->size == 0) {
        printf("⚠ Lỗi: Array rỗng\n");
        return -1;
    }
    
    int value = arr->data[arr->size - 1];
    arr->size--;
    
    printf("  - Xóa %d. Size hiện: %d\n", value, arr->size);
    return value;
}

// ============================================================
// 7. XOÁ TẠI VỊ TRÍ
// ============================================================

/**
 * Remove: Xóa phần tử tại vị trí index
 * 
 * Mô phỏng (xóa index=2):
 * [1, 2, 3, 4, 5]  →  [1, 2, 4, 5]
 * 
 * Phải dịch tất cả phần tử sau nó về phía trước
 * Chi phí: O(n) trong trường hợp xấu nhất (xóa ở đầu)
 */
void removeAt(DynamicArray *arr, int index) {
    if (index < 0 || index >= arr->size) {
        printf("⚠ Lỗi: Index %d ngoài phạm vi\n", index);
        return;
    }
    
    // Dịch tất cả phần tử sau index về trước 1 vị trí
    for (int i = index; i < arr->size - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }
    
    arr->size--;
    printf("  - Xóa tại index %d. Size hiện: %d\n", index, arr->size);
}

// ============================================================
// 8. IN RA
// ============================================================

void printArray(DynamicArray *arr) {
    printf("Array: [");
    for (int i = 0; i < arr->size; i++) {
        printf("%d", arr->data[i]);
        if (i < arr->size - 1) printf(", ");
    }
    printf("] (Size: %d, Capacity: %d)\n", arr->size, arr->capacity);
}

// ============================================================
// 9. GIẢI PHÓNG BỘ NHỚ
// ============================================================

void freeArray(DynamicArray *arr) {
    free(arr->data);
    free(arr);
    printf("✓ Giải phóng Array xong\n");
}

// ============================================================
// 10. DEMO & TEST
// ============================================================

int main() {
    printf("=== DYNAMIC ARRAY / ARRAYLIST DEMO ===\n\n");
    
    // Khởi tạo
    DynamicArray *arr = createArray();
    printArray(arr);
    
    printf("\n--- THÊM PHẦN TỬ ---\n");
    // Thêm vài phần tử (không cần resize)
    push(arr, 10);
    push(arr, 20);
    push(arr, 30);
    printArray(arr);
    
    printf("\n--- TRUY CẬP ---\n");
    printf("Element tại index 0: %d\n", get(arr, 0));
    printf("Element tại index 2: %d\n", get(arr, 2));
    
    printf("\n--- THÊM PHẦN TỬ CHI PHÍ CAO (RESIZE) ---\n");
    // Thêm cho đến khi cần resize
    for (int i = 4; i <= 15; i++) {
        push(arr, i * 10);
    }
    printArray(arr);
    
    printf("\n--- XOÁ CUỐI ---\n");
    pop(arr);
    pop(arr);
    printArray(arr);
    
    printf("\n--- XOÁ TẠI VỊ TRÍ ---\n");
    removeAt(arr, 2);
    printArray(arr);
    
    // Giải phóng
    freeArray(arr);
    
    return 0;
}

/*
 * ============================================================
 * TÓM TẮT ĐỘ PHỨC TẠP
 * ============================================================
 * 
 * | Phép toán   | Độ phức tạp | Ghi chú                    |
 * |-------------|-------------|---------------------------|
 * | Get         | O(1)        | Truy cập trực tiếp         |
 * | Push (cuối) | O(1)*       | * Amortized, resize O(n)   |
 * | Pop (cuối)  | O(1)        |                            |
 * | Insert (k)  | O(n)        | Cần dịch các phần tử sau    |
 * | Remove (k)  | O(n)        | Cần dịch các phần tử sau    |
 * | Search      | O(n)        | Phải kiểm tra từng phần tử  |
 * 
 * ============================================================
 * NETWORK APPLICATION
 * ============================================================
 * 
 * 1. Packet Buffer: lưu các packet đến
 * 2. Socket Table: danh sách các connection
 * 3. Routing Table: danh sách các route (thường dùng hash table bên trên)
 * 
 * ============================================================
 */
