/**
 * DATA STRUCTURE: Linked List (Danh sách liên kết)
 * 
 * STORY:
 * Array yêu cầu bộ nhớ liên tục. Nếu đã dùng hết, phải resize (O(n)).
 * 
 * Nếu bạn không biết trước số lượng phần tử?
 * Nếu bạn thường xuyên thêm/xóa ở đầu danh sách?
 * 
 * Giải pháp: Linked List
 * 
 * KIẾN TRÚC:
 * Array:                          Linked List:
 * [10, 20, 30, 40, 50]           10 → 20 → 30 → 40 → 50 → NULL
 * 
 * Con trỏ đầu (head) chỉ đến node đầu tiên.
 * Mỗi node chứa:
 *  - data: giá trị
 *  - next: con trỏ tới node tiếp theo
 * 
 * TRADEOFF:
 * - Thêm/Xóa: O(1) (nếu biết vị trí)
 * - Truy cập: O(n) (phải đi từ đầu)
 * - Bộ nhớ: +8 bytes/node cho con trỏ
 * 
 * NETWORK APPLICATION:
 * - TCP connection queue
 * - Packet queue tại network interface
 */

#include <stdio.h>
#include <stdlib.h>

// ============================================================
// 1. ĐỊNH NGHĨA NODE VÀ LINKED LIST
// ============================================================

/**
 * Node: là khối xây dựng cơ bản
 * 
 * ┌────────┐
 * │ data   │  = giá trị (int)
 * │ next   │  = con trỏ tới node tiếp theo
 * └────────┘
 */
typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;  // Con trỏ tới node đầu tiên
    int size;    // Số phần tử hiện có
} LinkedList;

// ============================================================
// 2. TẠO NODE MỚI
// ============================================================

/**
 * Cấp phát bộ nhớ cho một node mới
 */
Node* createNode(int data) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    return node;
}

// ============================================================
// 3. KHỞI TẠO LINKED LIST
// ============================================================

LinkedList* createLinkedList() {
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->size = 0;
    printf("✓ Tạo LinkedList xong\n");
    return list;
}

// ============================================================
// 4. THÊM PHẦN TỬ VÀO CUỐI
// ============================================================

/**
 * Push Back: Thêm phần tử vào cuối
 * 
 * Bước:
 * 1. Tạo node mới
 * 2. Tìm node cuối cùng (next == NULL)
 * 3. Gắn node cuối cùng.next = node mới
 * 4. size++
 * 
 * Độ phức tạp: O(n) vì phải tìm node cuối
 * 
 * Mô phỏng (thêm 40):
 * Trước:  10 → 20 → 30 → NULL
 * Sau:    10 → 20 → 30 → 40 → NULL
 */
void pushBack(LinkedList *list, int data) {
    Node *newNode = createNode(data);
    
    // Trường hợp danh sách trống
    if (list->head == NULL) {
        list->head = newNode;
        list->size++;
        printf("  + Thêm %d vào danh sách trống\n", data);
        return;
    }
    
    // Tìm node cuối
    Node *current = list->head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    // Gắn node mới vào cuối
    current->next = newNode;
    list->size++;
    printf("  + Thêm %d vào cuối. Size: %d\n", data, list->size);
}

// ============================================================
// 5. THÊM PHẦN TỬ VÀO ĐẦU (ĐÂY LÀ ĐIỂM MẠNH CỦA LINKED LIST)
// ============================================================

/**
 * Push Front: Thêm phần tử vào đầu
 * 
 * Bước:
 * 1. Tạo node mới
 * 2. node mới.next = head
 * 3. head = node mới
 * 4. size++
 * 
 * Độ phức tạp: O(1) ← CỰC NHANH!
 * 
 * So sánh với Array (thêm vào đầu):
 * - Linked List: O(1)
 * - Array: O(n) (vì phải dịch tất cả)
 * 
 * Mô phỏng (thêm 5):
 * Trước:  10 → 20 → 30 → NULL
 * Sau:    5 → 10 → 20 → 30 → NULL
 */
void pushFront(LinkedList *list, int data) {
    Node *newNode = createNode(data);
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
    printf("  + Thêm %d vào đầu. Size: %d\n", data, list->size);
}

// ============================================================
// 6. TRUY CẬP PHẦN TỬ
// ============================================================

/**
 * Get: Lấy giá trị tại vị trí index
 * 
 * Phải đi từ đầu đến vị trí index
 * 
 * Độ phức tạp: O(n)
 */
int get(LinkedList *list, int index) {
    if (index < 0 || index >= list->size) {
        printf("⚠ Lỗi: Index %d ngoài phạm vi [0, %d)\n", index, list->size);
        return -1;
    }
    
    Node *current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    
    return current->data;
}

// ============================================================
// 7. XOÁ PHẦN TỬ TẠI VỊ TRÍ
// ============================================================

/**
 * Remove: Xóa phần tử tại vị trí index
 * 
 * Bước:
 * 1. Tìm node trước vị trí index
 * 2. Gắn node trước.next = node hiện tại.next
 * 3. Giải phóng bộ nhớ node hiện tại
 * 4. size--
 * 
 * Độ phức tạp: O(n)
 * 
 * Mô phỏng (xóa index=1):
 * Trước:  10 → 20 → 30 → NULL
 * Sau:    10 → 30 → NULL
 */
void removeAt(LinkedList *list, int index) {
    if (index < 0 || index >= list->size) {
        printf("⚠ Lỗi: Index %d ngoài phạm vi\n", index);
        return;
    }
    
    Node *toDelete;
    
    // Nếu xóa node đầu
    if (index == 0) {
        toDelete = list->head;
        list->head = list->head->next;
    } else {
        // Tìm node trước
        Node *prev = list->head;
        for (int i = 0; i < index - 1; i++) {
            prev = prev->next;
        }
        
        toDelete = prev->next;
        prev->next = toDelete->next;
    }
    
    free(toDelete);
    list->size--;
    printf("  - Xóa tại index %d. Size: %d\n", index, list->size);
}

// ============================================================
// 8. XOÁ NODE ĐẦU (O(1))
// ============================================================

/**
 * Pop Front: Xóa node đầu tiên
 * 
 * Độ phức tạp: O(1)
 * (Array phải O(n) để dịch tất cả)
 */
int popFront(LinkedList *list) {
    if (list->head == NULL) {
        printf("⚠ Lỗi: LinkedList rỗng\n");
        return -1;
    }
    
    int data = list->head->data;
    Node *temp = list->head;
    list->head = list->head->next;
    free(temp);
    list->size--;
    
    printf("  - Xóa đầu (giá trị %d). Size: %d\n", data, list->size);
    return data;
}

// ============================================================
// 9. IN RA
// ============================================================

void printList(LinkedList *list) {
    printf("LinkedList: [");
    Node *current = list->head;
    while (current != NULL) {
        printf("%d", current->data);
        if (current->next != NULL) printf(" → ");
        current = current->next;
    }
    printf("] (Size: %d)\n", list->size);
}

// ============================================================
// 10. GIẢI PHÓNG BỘ NHỚ
// ============================================================

void freeLinkedList(LinkedList *list) {
    Node *current = list->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
    printf("✓ Giải phóng LinkedList xong\n");
}

// ============================================================
// 11. DEMO & TEST
// ============================================================

int main() {
    printf("=== LINKED LIST DEMO ===\n\n");
    
    // Khởi tạo
    LinkedList *list = createLinkedList();
    printList(list);
    
    printf("\n--- THÊM VÀO CUỐI ---\n");
    pushBack(list, 10);
    pushBack(list, 20);
    pushBack(list, 30);
    printList(list);
    
    printf("\n--- THÊM VÀO ĐẦU (O(1) - NHANH!) ---\n");
    pushFront(list, 5);
    pushFront(list, 1);
    printList(list);
    
    printf("\n--- TRUY CẬP ---\n");
    printf("Element tại index 0: %d\n", get(list, 0));
    printf("Element tại index 2: %d\n", get(list, 2));
    printf("Element tại index 4: %d\n", get(list, 4));
    
    printf("\n--- XOÁ TẠI VỊ TRÍ ---\n");
    removeAt(list, 2);
    printList(list);
    
    printf("\n--- XOÁ ĐẦU ---\n");
    popFront(list);
    popFront(list);
    printList(list);
    
    // Giải phóng
    freeLinkedList(list);
    
    printf("\n=== SO SÁNH: ARRAY vs LINKED LIST ===\n\n");
    printf("| Phép toán     | Array | LinkedList |\n");
    printf("|---------------|-------|------------|\n");
    printf("| Get (index)   | O(1)  | O(n)       |\n");
    printf("| Add (cuối)    | O(1)* | O(n)       |\n");
    printf("| Add (đầu)     | O(n)  | O(1)  ← ✓  |\n");
    printf("| Remove (vị trí)| O(n)  | O(n)       |\n");
    printf("| Remove (đầu)  | O(n)  | O(1)  ← ✓  |\n");
    printf("| Bộ nhớ        | Liên tục | Rời rạc  |\n");
    printf("| Cache locality| Tốt   | Kém        |\n");
    printf("\\n* Array amortized\n");
    
    return 0;
}

/*
 * ============================================================
 * NETWORK APPLICATION EXAMPLE
 * ============================================================
 * 
 * TCP CONNECTION QUEUE:
 * 
 * Mỗi khi client kết nối, bạn cần:
 * 1. Thêm connection vào queue (pushBack)
 * 2. Sau khi xử lý xong, xóa (popFront)
 * 
 * Với Linked List:
 * - popFront: O(1) ← Cực nhanh!
 * 
 * Với Array:
 * - removeAt(0): O(n) ← Phải dịch tất cả
 * 
 * Đó là lý do network software thường dùng Linked List
 * hoặc Ring Buffer (circular array) cho queue.
 * 
 * ============================================================
 */
