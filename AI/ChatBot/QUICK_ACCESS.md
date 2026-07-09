# ChatBot - Quick Access Guide

## 🚀 Application Status: LIVE & RUNNING

All services are currently running and healthy. Access them now:

### 📱 Frontends & APIs

| Service | URL | Status |
|---------|-----|--------|
| **Frontend** | http://localhost:3000 | ✅ Running |
| **Backend API** | http://localhost:8080 | ✅ Running |
| **Ollama LLM** | http://localhost:11434 | ✅ Running |
| **Database** | localhost:5432 | ✅ Running |

---

## 🧪 Test It Now

### Health Check
```bash
curl http://localhost:8080/api/chat/health | jq .
```

Expected Response:
```json
{
  "status": "UP",
  "requestId": "req_XXXXXXXX"
}
```

### Create a Conversation
```bash
curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{"model":"llama2"}' | jq .
```

Expected: Returns conversation with request ID included

---

## 📚 Key Features

✅ **Global Request ID Tracing**
- Every request gets a unique ID: `req_XXXXXXXX`
- Visible in responses, logs, and database

✅ **Clean Architecture**
- Controller → Service → Repository layers
- Separation of concerns
- Testable and maintainable

✅ **Production Ready**
- Input validation
- Error handling
- Security (CORS, etc.)
- Health checks

---

## 🛠️ Common Commands

### View Logs
```bash
# Backend logs
docker-compose logs -f backend

# Frontend logs  
docker-compose logs -f frontend

# All services
docker-compose logs -f
```

### Manage Services
```bash
# View status
docker ps --filter "name=chatbot"

# Stop all
docker-compose down

# Start all
docker-compose up -d

# Restart backend
docker-compose up -d backend
```

### Database Access
```bash
# Connect to PostgreSQL
docker exec -it chatbot-postgres psql -U chatbot_user -d chatbot

# Check tables
\dt

# View conversations
SELECT id, title, model, created_at FROM conversations;
```

---

## 📖 Documentation Files

| File | Purpose |
|------|---------|
| **README.md** | Project overview |
| **ARCHITECTURE.md** | System design |
| **BACKEND_IMPLEMENTATION.md** | Backend details |
| **FRONTEND_IMPLEMENTATION.md** | Frontend details |
| **DEPLOYMENT_SUCCESS.md** | Deployment report |
| **DEPLOYMENT_GUIDE.md** | How to deploy |
| **TEST_SCENARIOS.md** | Test cases |

---

## 🔍 Request ID Tracing Example

### Request
```bash
curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{"model":"llama2"}'
```

### Response
```json
{
  "id": "20671c2c-421a-48de-a6fa-d9f377e456eb",
  "title": "Conversation with llama2",
  "model": "llama2",
  "messages": [],
  "requestId": "req_b050d686"
}
```

### In Logs
```
[] 13:47:22.656 [http-nio-8080-exec-1] INFO com.chatbot.controller.ChatController
- Creating new conversation with model: llama2 [req_b050d686]
```

### In Database
```sql
SELECT id, conversation_id, request_id FROM chat_messages 
WHERE request_id = 'req_b050d686';
```

---

## 🎯 API Endpoints

### Conversations
```bash
# Create
POST /api/chat/conversations
{ "model": "llama2" }

# List all
GET /api/chat/conversations

# Get one
GET /api/chat/conversations/{id}
```

### Messages
```bash
# Get history
GET /api/chat/history/{conversationId}

# Send message
POST /api/chat/send
{
  "conversationId": "...",
  "message": "Hello!",
  "model": "llama2"
}

# Delete conversation
DELETE /api/chat/history/{conversationId}
```

### System
```bash
# Health check
GET /api/chat/health
```

---

## 🧠 Ollama LLM

### Check Status
```bash
curl http://localhost:11434/api/tags | jq .

# or
docker exec chatbot-ollama ollama list
```

### Pull Model
```bash
docker exec chatbot-ollama ollama pull llama2
```

### Chat with LLM (direct)
```bash
curl http://localhost:11434/api/generate \
  -X POST \
  -d '{"model":"llama2","prompt":"Hello!"}' \
  | jq .
```

---

## 📊 Test Results

**Last Test Run:** July 4, 2026 13:48 UTC  
**Pass Rate:** 100% (13/13 tests)

✅ All services healthy  
✅ Request ID tracing working  
✅ API endpoints functioning  
✅ Database persisting data  
✅ Frontend responsive  
✅ Error handling validated  

---

## ⚙️ Configuration

### Environment
- **SPRING_DATASOURCE_URL:** `jdbc:postgresql://postgres:5432/chatbot`
- **OLLAMA_API_URL:** `http://ollama:11434`
- **Profiles:** `prod` (default)

### Ports
- Frontend: 3000
- Backend: 8080
- PostgreSQL: 5432
- Ollama: 11434

### Database
- Username: `chatbot_user`
- Password: `chatbot_password`
- Database: `chatbot`

---

## 🐛 Troubleshooting

### Backend won't start
```bash
# Check logs
docker logs chatbot-backend

# Check database connection
docker exec chatbot-postgres pg_isready -U chatbot_user
```

### Frontend can't reach backend
```bash
# Verify backend is running
curl http://localhost:8080/api/chat/health

# Check network
docker network inspect chatbot_chatbot-network
```

### Ollama not responding
```bash
# Check Ollama status
docker logs chatbot-ollama

# Restart Ollama
docker-compose restart ollama
```

---

## 📞 Support

For detailed information, see:
- **DEPLOYMENT_GUIDE.md** - Production deployment
- **DEPLOYMENT_SUCCESS.md** - Test results & metrics
- **FINAL_SUMMARY.txt** - Project overview

---

**Last Updated:** July 4, 2026  
**Status:** ✅ All systems operational
