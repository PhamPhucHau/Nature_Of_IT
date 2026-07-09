Nếu cấu trúc dữ liệu trả lời câu hỏi:

> **"Dữ liệu nên được tổ chức như thế nào?"**

thì **thuật toán** trả lời câu hỏi lớn hơn:

> **"Ta nên suy nghĩ như thế nào để giải quyết một vấn đề?"**

Lịch sử của thuật toán không chỉ là lịch sử của máy tính, mà là lịch sử của cách con người học cách tư duy.

---

# Câu chuyện 1: Binary Search – "Sức mạnh của việc loại bỏ"

Khoảng 4.000 năm trước, một người quản thư đứng trước hàng nghìn cuộn giấy.

Một học trò hỏi:

> "Thầy tìm cuộn giấy này bằng cách nào?"

Nếu tìm từ đầu đến cuối:

```
A
B
C
...
Z
```

Có thể mất hàng giờ.

Người quản thư không làm vậy.

Ông đi đến giữa.

Nếu tên cuộn giấy đứng sau M, ông bỏ luôn nửa đầu.

Nếu đứng trước M, ông bỏ luôn nửa sau.

Ông không tìm.

Ông **loại bỏ**.

Đó là tư tưởng của Binary Search.

### Triết lý

> Một bài toán lớn thường được giải bằng cách liên tục loại bỏ những phần chắc chắn không chứa đáp án.

### Cái giá

Muốn loại bỏ được.

Dữ liệu phải được sắp xếp.

Không có thứ tự.

Không dùng được.

---

# Câu chuyện 2: Merge Sort – "Chia để trị"

Trong chiến tranh.

Một vị tướng có 100.000 lính.

Ông không thể ra lệnh cho tất cả cùng lúc.

Ông chia:

```
100000

↓

50000 + 50000

↓

25000 + 25000

↓

...
```

Sau đó hợp nhất.

Merge Sort cũng làm như vậy.

### Triết lý

> Một vấn đề lớn thường dễ giải hơn nếu biến thành nhiều vấn đề nhỏ.

Ngày nay.

Đó là nền tảng của:

* MapReduce
* Parallel Computing
* Distributed Systems

### Cái giá

Cần thêm bộ nhớ.

---

# Câu chuyện 3: Quick Sort – "Đừng cố kiểm soát mọi thứ"

Một giáo sư hỏi:

> "Nếu phải xếp 1 triệu lá bài?"

Có người nói:

"Tôi sẽ tìm lá nhỏ nhất."

Một người khác nói:

"Tôi sẽ chọn đại một lá."

Đó là Pivot.

Mọi lá nhỏ hơn sang trái.

Lớn hơn sang phải.

Lặp lại.

Quick Sort ra đời.

### Triết lý

> Không cần quyết định hoàn hảo ngay từ đầu.

Chỉ cần một quyết định đủ tốt.

### Cái giá

Nếu chọn Pivot quá tệ.

Độ phức tạp có thể từ **O(n log n)** thành **O(n²)**.

---

# Câu chuyện 4: BFS – "Kiến thức lan truyền như sóng nước"

Bạn ném viên đá xuống hồ.

```
●

○○○

○○○○○

○○○○○○○
```

Sóng lan đều.

BFS cũng vậy.

Khám phá theo từng lớp.

### Triết lý

> Những thứ gần nhất nên được khám phá trước.

Ứng dụng:

* GPS
* Lan truyền mạng xã hội
* Tìm đường ngắn nhất trên đồ thị không trọng số

### Cái giá

Phải nhớ tất cả các đỉnh đang chờ xử lý, nên có thể tốn nhiều bộ nhớ.

---

# Câu chuyện 5: DFS – "Đi đến tận cùng"

Có hai kiểu người.

Người thứ nhất:

> Xem hết tầng một rồi mới lên tầng hai.

Đó là BFS.

Người thứ hai:

> Thấy cầu thang là leo luôn lên mái.

Đó là DFS.

### Triết lý

> Muốn hiểu sâu.

Phải chấp nhận đi xa.

### Cái giá

Có thể đi rất sâu vào một nhánh "không đúng" trước khi quay lại.

---

# Câu chuyện 6: Dijkstra – "Con đường tốt nhất hôm nay"

Một thương nhân hỏi:

> "Làm sao đi từ A đến Z ít tốn tiền nhất?"

Không ai biết ngay.

Ông chỉ biết:

> Mỗi lần chọn thành phố gần nhất mà mình chắc chắn đã tối ưu.

Rồi mở rộng.

Đó là Dijkstra.

### Triết lý

> Điều tốt nhất toàn cục đôi khi được xây dựng từ những quyết định cục bộ đã được chứng minh là tối ưu.

### Cái giá

Không dùng được nếu có cạnh trọng số âm.

---

# Câu chuyện 7: Dynamic Programming – "Đừng làm lại điều mình đã biết"

Một cậu bé leo cầu thang.

Muốn lên bậc 100.

Cậu luôn tính lại:

```
100

↓

99

↓

98

↓

...
```

Một ông già cười.

> "Con vừa tính bậc 98 cách đây một phút."

Thế là cậu ghi kết quả vào sổ.

Đó là Dynamic Programming.

### Triết lý

> Đừng giải cùng một bài toán hai lần.

### Cái giá

Phải đánh đổi bằng bộ nhớ để lưu các kết quả trung gian.

---

# Câu chuyện 8: Greedy – "Sống cho hiện tại"

Một người du lịch.

Mỗi lần đến ngã ba.

Ông chọn con đường ngắn nhất trước mắt.

Nhiều lần.

Ông đến đích rất nhanh.

Nhưng không phải lúc nào cũng.

Đó là Greedy.

### Triết lý

> Hy vọng rằng lựa chọn tốt nhất ở hiện tại sẽ dẫn đến kết quả tốt nhất sau cùng.

### Cái giá

Không có bảo đảm đúng cho mọi bài toán.

---

# Câu chuyện 9: Backtracking – "Biết sai thì quay lại"

Một mê cung.

Bạn đi.

Gặp ngõ cụt.

Quay lại.

Đi hướng khác.

Đó là Backtracking.

### Triết lý

> Sai không đáng sợ.

Quan trọng là biết quay về đúng điểm để thử hướng khác.

### Cái giá

Không gian tìm kiếm có thể tăng theo cấp số nhân, khiến thời gian chạy rất lớn.

---

# Câu chuyện 10: Hashing – "Đừng hỏi ai ở đâu, hãy tính"

Một thành phố có 10 triệu hộ dân.

Thay vì hỏi từng người.

Người đưa thư nói:

> "Số nhà chính là công thức."

Hashing ra đời.

### Triết lý

> Tính toán thường rẻ hơn tìm kiếm.

### Cái giá

Hai địa chỉ khác nhau đôi khi lại rơi vào cùng một "ô" (collision), nên phải có cách xử lý.

---

# Câu chuyện lớn nhất: Khoa học máy tính là khoa học của sự đánh đổi

Nếu nhìn sâu hơn, gần như mọi thuật toán kinh điển đều phản ánh một cách tư duy:

* **Binary Search:** Muốn nhanh thì hãy loại bỏ những điều không cần thiết.
* **Merge Sort:** Chia vấn đề lớn thành nhiều phần nhỏ.
* **Quick Sort:** Một quyết định đủ tốt, lặp đi lặp lại, có thể dẫn đến kết quả rất tốt.
* **BFS:** Khám phá theo chiều rộng để tìm cái gần nhất.
* **DFS:** Đi thật sâu để hiểu cấu trúc.
* **Dijkstra:** Xây dựng lời giải tối ưu bằng các bước nhỏ đã được chứng minh đúng.
* **Dynamic Programming:** Học từ quá khứ, không lặp lại sai lầm hay công việc đã làm.
* **Greedy:** Ra quyết định nhanh khi cấu trúc bài toán cho phép.
* **Backtracking:** Dũng cảm thử nghiệm và sẵn sàng quay lại.
* **Hashing:** Thay đổi cách nhìn vấn đề để biến tìm kiếm thành tính toán.

Đó cũng là lý do nhiều nhà khoa học nói rằng **thuật toán không chỉ là công thức để máy tính chạy**. Chúng là những mô hình tư duy. Khi học một thuật toán, bạn không chỉ học cách giải một bài toán cụ thể, mà còn học một cách nhìn nhận vấn đề. Những cách tư duy này xuất hiện ở khắp nơi: từ định tuyến trên Internet, tìm kiếm của công cụ tìm kiếm, lập lịch trong hệ điều hành, cho đến việc đưa ra quyết định trong kinh doanh và khoa học.
