# 🌐 NGINX — Web Server nổi tiếng viết bằng C

NGINX là một trong những web server mạnh và nổi tiếng nhất thế giới, được viết chủ yếu bằng ngôn ngữ C để đạt hiệu năng cực cao.

---

# 🧠 NGINX thực chất là gì?

NGINX là:
- Web Server
- Reverse Proxy
- Load Balancer
- API Gateway
- Static File Server

Nó chuyên xử lý:
- HTTP
- HTTPS
- TCP
- UDP
- WebSocket
- Streaming

---

# ⚡ Vì sao NGINX viết bằng C?

C giúp:
- truy cập socket trực tiếp
- quản lý memory cực tối ưu
- ít overhead
- xử lý hàng chục nghìn connection đồng thời

Đây là lý do NGINX rất nhanh và nhẹ.

---

# 🏗️ Kiến trúc tự nhiên của NGINX

NGINX không tạo:
> 1 thread / 1 connection

mà dùng:
- event-driven architecture
- non-blocking I/O
- epoll (Linux)
- kqueue (BSD/Mac)

---

# 📌 Luồng hoạt động của NGINX

```text
Browser
   ↓
Socket TCP
   ↓
NGINX Master Process
   ↓
Worker Processes
   ↓
Event Loop (epoll)
   ↓
HTTP Parser
   ↓
Serve HTML / Proxy API