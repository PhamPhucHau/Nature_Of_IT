# ChatBot Product Architecture
## Local LLM Integration with Ollama

**Date:** June 23, 2026  
**Version:** 1.0  
**Stack:** React (Frontend) + Java Spring Boot (Backend) + Ollama (LLM)

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Frontend (React)                          │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  ChatUI Component                                        │  │
│  │  - Message Input                                         │  │
│  │  - Message Display                                       │  │
│  │  - Model Selector (Llama, ChatGPT, etc.)               │  │
│  │  - Chat History                                          │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓ HTTP/REST
┌─────────────────────────────────────────────────────────────────┐
│                   Anylink Middleware                             │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Global ID Generator (UUID)                              │  │
│  │  Request Logging & Tracing                               │  │
│  │  CORS & Security                                         │  │
│  │  Request/Response Interceptor                            │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓ REST API
┌─────────────────────────────────────────────────────────────────┐
│                  Java Spring Boot Backend                        │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Controller Layer                                        │  │
│  │  - ChatController                                        │  │
│  │  - ModelController                                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Service Layer                                           │  │
│  │  - ChatService (interface)                              │  │
│  │  - ChatServiceImpl                                        │  │
│  │  - OllamaService                                         │  │
│  │  - ModelService                                          │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Repository Layer                                        │  │
│  │  - ChatHistoryRepository                                 │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Security & Config                                       │  │
│  │  - SecurityConfig                                        │  │
│  │  - CorsMvcConfig                                         │  │
│  │  - RequestIdFilter                                       │  │
│  └──────────────────────────────────────────────────────────┘  │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Common/Utilities                                        │  │
│  │  - LoggingAspect                                         │  │
│  │  - ExceptionHandler                                      │  │
│  │  - Constants                                             │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓ HTTP API
┌─────────────────────────────────────────────────────────────────┐
│                    Ollama LLM Server                             │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  Ollama (Running on localhost:11434)                    │  │
│  │  - Llama 2                                               │  │
│  │  - Mistral                                               │  │
│  │  - Neural Chat                                           │  │
│  │  - Custom Models                                         │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              ↓ Local Storage
┌─────────────────────────────────────────────────────────────────┐
│                      H2 Database                                 │
│  ┌──────────────────────────────────────────────────────────┐  │
│  │  chat_history                                            │  │
│  │  - id (UUID)                                             │  │
│  │  - message                                               │  │
│  │  - response                                              │  │
│  │  - model_used                                            │  │
│  │  - timestamp                                             │  │
│  │  - request_id (tracing)                                 │  │
│  └──────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

---

## 📋 Data Flow

### Message Send Flow:

```
1. User types message in ChatUI
2. React selects model (Llama, Mistral, etc.)
3. Frontend sends POST /api/chat/send
   {
     "message": "Hello, what is AI?",
     "model": "llama2",
     "conversationId": "uuid-123"
   }

4. Anylink Middleware intercepts
   - Generates Global Request ID: "req_12345abcde"
   - Adds to request header: X-Request-ID: req_12345abcde
   - Logs: [req_12345abcde] Incoming request: /api/chat/send

5. Spring Controller receives request
   - Extracts X-Request-ID header
   - Logs: [req_12345abcde] ChatController.send() - Processing message

6. Service Layer processes
   - ChatServiceImpl.send()
   - Logs: [req_12345abcde] Calling Ollama for model: llama2

7. OllamaService calls Ollama API
   - HTTP POST http://localhost:11434/api/generate
   - Logs: [req_12345abcde] Ollama response received in 2.5s

8. Response stored in Database
   - Logs: [req_12345abcde] Saving to ChatHistoryRepository

9. Response sent back to Frontend
   - HTTP 200 with response JSON
   - Logs: [req_12345abcde] Response sent to client

10. React displays message and updates chat history
    - Shows response with timestamp
    - Updates chat history panel
```

---

## 🔗 Request Tracing Example

**All logs include the global request ID for easy tracing:**

```
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] INFO - Incoming request: POST /api/chat/send
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] INFO - Headers: {X-Request-ID: req_a1b2c3d4e5f6, Content-Type: application/json}
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] DEBUG - Request body: {"message":"What is machine learning?","model":"llama2"}
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] INFO - ChatController.send() invoked
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] INFO - ChatServiceImpl.send() started
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] DEBUG - Conversation ID: conv_xyz789
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] INFO - OllamaService.generate() called with model: llama2
[2024-06-23 10:15:30] [req_a1b2c3d4e5f6] DEBUG - Ollama request: {model: "llama2", prompt: "What is machine learning?"}
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] INFO - Ollama response received (2.8s latency)
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] DEBUG - Response: "Machine learning is a subset of AI..."
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] INFO - Saving to ChatHistoryRepository
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] DEBUG - Saved with ID: chat_12345
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] INFO - Response sent to client
[2024-06-23 10:15:33] [req_a1b2c3d4e5f6] INFO - Total request time: 3.2s
```

---

## 📦 Project Structure

```
chatbot/
├── frontend/
│   ├── public/
│   ├── src/
│   │   ├── components/
│   │   │   ├── ChatBox.jsx
│   │   │   ├── MessageList.jsx
│   │   │   ├── ModelSelector.jsx
│   │   │   └── ChatHistory.jsx
│   │   ├── services/
│   │   │   └── api.js (Axios instance with request interceptor)
│   │   ├── hooks/
│   │   │   └── useChatbot.js
│   │   ├── App.jsx
│   │   └── index.jsx
│   ├── package.json
│   └── vite.config.js
│
├── backend/
│   ├── src/main/java/com/chatbot/
│   │   ├── controller/
│   │   │   ├── ChatController.java
│   │   │   └── ModelController.java
│   │   ├── service/
│   │   │   ├── ChatService.java (interface)
│   │   │   ├── ChatServiceImpl.java
│   │   │   ├── OllamaService.java
│   │   │   └── ModelService.java
│   │   ├── repository/
│   │   │   └── ChatHistoryRepository.java
│   │   ├── model/
│   │   │   ├── ChatMessage.java
│   │   │   ├── ChatRequest.java
│   │   │   └── ChatResponse.java
│   │   ├── security/
│   │   │   ├── SecurityConfig.java
│   │   │   ├── CorsMvcConfig.java
│   │   │   └── RequestIdFilter.java
│   │   ├── common/
│   │   │   ├── LoggingAspect.java
│   │   │   ├── ExceptionHandler.java
│   │   │   ├── Constants.java
│   │   │   └── RequestContext.java
│   │   └── ChatBotApplication.java
│   ├── src/main/resources/
│   │   ├── application.yml
│   │   ├── application-dev.yml
│   │   └── logback.xml
│   ├── pom.xml
│   └── Dockerfile
│
├── docker-compose.yml (Ollama + Backend)
└── README.md
```

---

## 🔑 Key Components

### 1. Anylink Middleware
- Generates unique Global Request ID (UUID format)
- Adds to request header: `X-Request-ID`
- Logs all incoming/outgoing requests with ID
- Enables distributed tracing across entire stack

### 2. RequestIdFilter (Spring Filter)
- Extracts X-Request-ID from headers
- Stores in ThreadLocal for logging throughout request
- Adds to response headers
- Ensures all logs include the request ID

### 3. LoggingAspect (AOP)
- Intercepts all @Service methods
- Logs method entry/exit with request ID
- Captures execution time
- Logs exceptions with full context

### 4. OllamaService
- Manages connection to Ollama server
- Supports multiple models
- Handles streaming responses
- Error handling & retries

### 5. ChatHistoryRepository
- Persists chat conversations
- Queries by conversation ID, model, date range
- Supports pagination

---

## 🚀 Key Features

✅ **Model Selection:** Switch between Llama, Mistral, and other Ollama models  
✅ **Chat History:** Full conversation history with timestamps  
✅ **Request Tracing:** Global request ID in all logs  
✅ **Clean Architecture:** Proper layer separation (Controller → Service → Repository)  
✅ **Error Handling:** Centralized exception handling with request context  
✅ **Security:** CORS configuration, input validation  
✅ **Logging:** Structured logging with MDC (Mapped Diagnostic Context)  
✅ **Local First:** Runs entirely locally (no cloud dependency)

---

## 🔧 Technology Stack

| Layer | Technology | Version |
|-------|-----------|---------|
| **Frontend** | React | 18+ |
| **State Mgmt** | React Hooks/Context | - |
| **HTTP Client** | Axios | 1.6+ |
| **Backend** | Spring Boot | 3.1+ |
| **Data** | Spring Data JPA | 3.1+ |
| **Database** | H2 / PostgreSQL | Latest |
| **LLM** | Ollama | Latest |
| **Logging** | SLF4J + Logback | 2.0+ |
| **Build** | Maven | 3.8+ |
| **Containerization** | Docker | Latest |

---

## Next Steps

See individual component documentation:
- `BACKEND_IMPLEMENTATION.md` - Backend setup and code
- `FRONTEND_IMPLEMENTATION.md` - React components and styling
- `OLLAMA_INTEGRATION.md` - Ollama configuration
- `DEPLOYMENT.md` - Docker & deployment guide
