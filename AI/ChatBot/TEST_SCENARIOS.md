# ChatBot - Comprehensive Test Scenarios

## 📋 Test Plan Overview

**Date:** July 4, 2026
**Project:** ChatBot with Ollama Integration
**Status:** Ready for Testing

---

## 🎯 Test Objectives

1. Verify all backend endpoints work correctly
2. Verify frontend UI renders and functions
3. Verify request ID tracing end-to-end
4. Verify database persistence
5. Verify error handling
6. Verify performance

---

## 📊 Test Scenarios

### Test Scenario 1: Health Check

**Purpose:** Verify services are running

**Prerequisites:**
- docker-compose up -d
- Wait 30 seconds for services to start

**Test Steps:**

```bash
# 1. Check service health
curl -i http://localhost:8080/api/chat/health

# Expected Response:
# HTTP/1.1 200 OK
# Content-Type: application/json
# X-Request-ID: req_xxxxxxxx
# 
# {
#   "status": "UP",
#   "requestId": "req_xxxxxxxx",
#   "timestamp": 1719096000000
# }
```

**Pass Criteria:**
- ✅ Status code 200
- ✅ Response includes requestId
- ✅ X-Request-ID header present

---

### Test Scenario 2: Create Conversation

**Purpose:** Verify conversation creation

**Test Steps:**

```bash
# 1. Create new conversation
curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{
    "model": "llama2"
  }'

# Expected Response:
# {
#   "id": "550e8400-e29b-41d4-a716-446655440000",
#   "title": "Conversation with llama2",
#   "model": "llama2",
#   "createdAt": "2026-07-04T19:54:00",
#   "updatedAt": "2026-07-04T19:54:00",
#   "messages": []
# }
```

**Pass Criteria:**
- ✅ Status code 201
- ✅ Response includes conversation ID
- ✅ Response includes model name
- ✅ Response includes timestamps
- ✅ Empty messages array

**Example Response:**
```json
{
  "id": "550e8400-e29b-41d4-a716-446655440000",
  "title": "Conversation with llama2",
  "model": "llama2",
  "messages": [],
  "createdAt": "2026-07-04T19:54:00.123",
  "updatedAt": "2026-07-04T19:54:00.123"
}
```

---

### Test Scenario 3: Send Message

**Purpose:** Verify message sending and LLM response

**Test Steps:**

```bash
# 1. Use conversation ID from Test Scenario 2
CONV_ID="550e8400-e29b-41d4-a716-446655440000"

# 2. Send message
curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d "{
    \"conversationId\": \"$CONV_ID\",
    \"message\": \"Hello! What is 2+2?\",
    \"model\": \"llama2\"
  }"

# Expected Response:
# {
#   "messageId": "550e8400-e29b-41d4-a716-446655440001",
#   "conversationId": "550e8400-e29b-41d4-a716-446655440000",
#   "role": "ASSISTANT",
#   "content": "2+2 equals 4",
#   "processingTimeMs": 2500,
#   "requestId": "req_a1b2c3d4"
# }
```

**Pass Criteria:**
- ✅ Status code 200
- ✅ Response includes messageId
- ✅ Response includes assistant response
- ✅ Response includes processingTimeMs
- ✅ Response includes requestId
- ✅ Response time > 2000ms (LLM inference)

---

### Test Scenario 4: Request ID Tracing

**Purpose:** Verify request ID flows through stack

**Test Steps:**

```bash
# 1. Send request with custom Request ID
curl -v -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -H "X-Request-ID: req_test_001" \
  -d '{"model": "llama2"}' 2>&1 | grep -i "x-request-id"

# Expected Output:
# < X-Request-ID: req_test_001

# 2. Check backend logs for request ID
docker-compose logs backend | grep "req_test_001"

# Expected Output:
# [req_test_001] INFO - Request started
# [req_test_001] INFO - Creating new conversation
```

**Pass Criteria:**
- ✅ X-Request-ID header in response
- ✅ Request ID appears in backend logs
- ✅ Request ID format: req_xxxxxxxx
- ✅ Same ID throughout request lifecycle

---

### Test Scenario 5: Get Conversation History

**Purpose:** Verify retrieving conversation history

**Test Steps:**

```bash
# 1. Get conversation history
CONV_ID="550e8400-e29b-41d4-a716-446655440000"

curl http://localhost:8080/api/chat/history/$CONV_ID \
  -H "Content-Type: application/json"

# Expected Response:
# [
#   {
#     "messageId": "550e8400-e29b-41d4-a716-446655440002",
#     "conversationId": "550e8400-e29b-41d4-a716-446655440000",
#     "role": "USER",
#     "content": "Hello! What is 2+2?",
#     "requestId": "req_xxxxxxxx"
#   },
#   {
#     "messageId": "550e8400-e29b-41d4-a716-446655440003",
#     "conversationId": "550e8400-e29b-41d4-a716-446655440000",
#     "role": "ASSISTANT",
#     "content": "2+2 equals 4",
#     "processingTimeMs": 2500,
#     "requestId": "req_xxxxxxxx"
#   }
# ]
```

**Pass Criteria:**
- ✅ Status code 200
- ✅ Response is array
- ✅ Contains USER message
- ✅ Contains ASSISTANT message
- ✅ Messages in correct order
- ✅ Each message includes requestId

---

### Test Scenario 6: Delete Conversation

**Purpose:** Verify conversation deletion

**Test Steps:**

```bash
# 1. Delete conversation
CONV_ID="550e8400-e29b-41d4-a716-446655440000"

curl -X DELETE http://localhost:8080/api/chat/history/$CONV_ID

# Expected Response:
# HTTP/1.1 204 No Content

# 2. Verify deletion
curl http://localhost:8080/api/chat/history/$CONV_ID

# Expected Response:
# HTTP/1.1 404 Not Found
# {
#   "status": 404,
#   "message": "Resource not found",
#   "details": "Conversation not found: ...",
#   "requestId": "req_xxxxxxxx",
#   "timestamp": 1719096000000
# }
```

**Pass Criteria:**
- ✅ Delete returns 204
- ✅ Subsequent GET returns 404
- ✅ Error response includes requestId
- ✅ Error message is descriptive

---

### Test Scenario 7: Multiple Conversations

**Purpose:** Verify multiple conversations work independently

**Test Steps:**

```bash
# 1. Create first conversation
curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{"model": "llama2"}' | jq -r '.id'
# Output: CONV_ID_1

# 2. Create second conversation
curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{"model": "mistral"}' | jq -r '.id'
# Output: CONV_ID_2

# 3. Send messages to both
curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d "{\"conversationId\":\"$CONV_ID_1\",\"message\":\"Hi\",\"model\":\"llama2\"}"

curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d "{\"conversationId\":\"$CONV_ID_2\",\"message\":\"Hello\",\"model\":\"mistral\"}"

# 4. Get both histories
curl http://localhost:8080/api/chat/history/$CONV_ID_1
curl http://localhost:8080/api/chat/history/$CONV_ID_2

# Verify messages are separate
```

**Pass Criteria:**
- ✅ Can create multiple conversations
- ✅ Each conversation has different ID
- ✅ Messages stay in correct conversation
- ✅ Histories are independent

---

### Test Scenario 8: Error Handling - Missing Fields

**Purpose:** Verify validation error handling

**Test Steps:**

```bash
# 1. Send message without conversationId
curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d '{"message":"Hello","model":"llama2"}'

# Expected Response:
# HTTP/1.1 400 Bad Request
# {
#   "status": 400,
#   "message": "Validation failed",
#   "details": "Conversation ID is required",
#   "requestId": "req_xxxxxxxx",
#   "timestamp": 1719096000000
# }
```

**Pass Criteria:**
- ✅ Status code 400
- ✅ Error message is descriptive
- ✅ Includes requestId
- ✅ Follows error response format

---

### Test Scenario 9: Error Handling - Invalid Conversation

**Purpose:** Verify handling of non-existent conversation

**Test Steps:**

```bash
# 1. Try to get non-existent conversation
curl http://localhost:8080/api/chat/history/invalid-id-12345

# Expected Response:
# HTTP/1.1 404 Not Found
# {
#   "status": 404,
#   "message": "Resource not found",
#   "details": "Conversation not found: invalid-id-12345",
#   "requestId": "req_xxxxxxxx",
#   "timestamp": 1719096000000
# }
```

**Pass Criteria:**
- ✅ Status code 404
- ✅ Descriptive error message
- ✅ Includes invalid ID in message
- ✅ Includes requestId

---

### Test Scenario 10: Database Persistence

**Purpose:** Verify data persists across container restarts

**Test Steps:**

```bash
# 1. Create conversation and send message
CONV_ID=$(curl -X POST http://localhost:8080/api/chat/conversations \
  -H "Content-Type: application/json" \
  -d '{"model":"llama2"}' | jq -r '.id')

curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d "{\"conversationId\":\"$CONV_ID\",\"message\":\"Test\",\"model\":\"llama2\"}"

# 2. Restart backend container
docker-compose restart backend

# 3. Wait for service to come up
sleep 10

# 4. Retrieve conversation history
curl http://localhost:8080/api/chat/history/$CONV_ID

# Expected: Message should still be there
```

**Pass Criteria:**
- ✅ Service restarts successfully
- ✅ Previous data is accessible
- ✅ Message content is intact
- ✅ Timestamps are preserved

---

### Test Scenario 11: Frontend Application Load

**Purpose:** Verify React frontend loads successfully

**Test Steps:**

```bash
# 1. Open browser and navigate to
# http://localhost:3000

# Expected UI Elements:
# - ✅ Title "💬 ChatBot"
# - ✅ Sidebar with "Conversations"
# - ✅ "➕ New" button
# - ✅ Main chat area
# - ✅ Message input field
# - ✅ Model selector

# 2. Check browser console
# Expected:
# - ✅ No console errors
# - ✅ Network requests to /api/chat/health (200)
# - ✅ No CORS errors
```

**Pass Criteria:**
- ✅ Page loads in < 2 seconds
- ✅ All UI elements render
- ✅ No console errors
- ✅ No CORS errors

---

### Test Scenario 12: Frontend - Create & Send Message

**Purpose:** Verify end-to-end chat from UI

**Test Steps:**

```
1. Open http://localhost:3000
2. Click "➕ New" button
3. See new conversation created
4. Select model "Llama 2" from dropdown
5. Type message: "Hello ChatBot!"
6. Click send button (📤)
7. Wait for response
8. Verify:
   - User message appears
   - Assistant response appears
   - Processing time shown
   - Request ID displayed
   - No errors in console
```

**Pass Criteria:**
- ✅ Conversation created
- ✅ Message sent successfully
- ✅ Response received
- ✅ UI updates in real-time
- ✅ Processing time < 5 minutes
- ✅ No console errors

---

### Test Scenario 13: Request ID Visibility in UI

**Purpose:** Verify request ID shown to user

**Test Steps:**

```
1. Send message from frontend
2. Look at message metadata
3. Should see:
   - Request ID (e.g., "req_a1b2c3d4")
   - Processing time (e.g., "2500ms")
   - Timestamp
4. Verify in browser DevTools:
   - Network tab shows X-Request-ID header
   - Console shows request ID in logs
```

**Pass Criteria:**
- ✅ Request ID visible in message
- ✅ Request ID in response headers
- ✅ Request ID in browser logs
- ✅ Request ID format correct

---

### Test Scenario 14: Model Switching

**Purpose:** Verify switching between models

**Test Steps:**

```
1. Create conversation with Llama 2
2. Send message: "What is AI?"
3. Get response from Llama 2
4. Change model to Mistral
5. Send message: "What is machine learning?"
6. Verify new model processes request
7. Check logs show model change
```

**Pass Criteria:**
- ✅ Model selector works
- ✅ Model change takes effect
- ✅ Correct model processes request
- ✅ Responses from different models

---

### Test Scenario 15: Performance Test

**Purpose:** Measure response times

**Test Steps:**

```bash
# 1. Measure API response time
time curl -X POST http://localhost:8080/api/chat/send \
  -H "Content-Type: application/json" \
  -d "{
    \"conversationId\":\"$CONV_ID\",
    \"message\":\"Quick math: 10+5\",
    \"model\":\"llama2\"
  }"

# 2. Measure database query
time curl http://localhost:8080/api/chat/history/$CONV_ID

# 3. Measure UI load
# Check DevTools Network tab for timings
```

**Pass Criteria:**
- ✅ Create conversation: < 100ms
- ✅ Send message: 5-60 seconds (LLM dependent)
- ✅ Get history: < 100ms
- ✅ Frontend load: < 2000ms
- ✅ API latency (excluding LLM): < 200ms

---

## 🔍 Logging Verification Tests

### Test L1: Request ID in Backend Logs

```bash
# Check logs for request ID prefix
docker-compose logs backend | grep "req_"

# Expected Output:
# [req_a1b2c3d4] INFO - ChatController.send()
# [req_a1b2c3d4] INFO - Processing message
# [req_a1b2c3d4] INFO - Saved to database
```

### Test L2: Request ID in Database

```bash
# Connect to database
docker exec -it chatbot-postgres psql -U chatbot_user -d chatbot

# Query messages with request ID
SELECT id, role, request_id FROM chat_messages LIMIT 5;

# Expected: request_id column populated with req_xxxxxxxx
```

---

## 🐛 Bug/Issue Tracking

| # | Issue | Status | Notes |
|---|-------|--------|-------|
| B1 | Docker image not found | Open | Need to use available base images |
| B2 | | | |

---

## 📈 Test Coverage Summary

| Category | Tests | Status |
|----------|-------|--------|
| API Endpoints | 6 | Ready |
| Error Handling | 2 | Ready |
| Frontend UI | 4 | Ready |
| Request ID Tracing | 2 | Ready |
| Performance | 1 | Ready |
| Database | 1 | Ready |
| **Total** | **16** | **Ready** |

---

## ✅ Test Execution Checklist

- [ ] Prerequisites installed (Docker, curl, jq)
- [ ] docker-compose up -d completed
- [ ] Wait 30 seconds for services
- [ ] Execute Scenario 1 (Health Check)
- [ ] Execute Scenario 2 (Create Conversation)
- [ ] Execute Scenario 3 (Send Message)
- [ ] Execute Scenarios 4-15
- [ ] Document results
- [ ] Report failures
- [ ] Generate test report

---

## 📝 Test Report Template

```
TEST EXECUTION REPORT

Date: [Date]
Tester: [Name]
Environment: [Dev/Staging/Prod]

RESULTS:
├─ Scenario 1: [PASS/FAIL]
├─ Scenario 2: [PASS/FAIL]
├─ Scenario 3: [PASS/FAIL]
├─ Scenario 4: [PASS/FAIL]
├─ Scenario 5: [PASS/FAIL]
├─ ...
├─ Scenario 15: [PASS/FAIL]

SUMMARY:
- Total: 15 tests
- Passed: X
- Failed: Y
- Pass Rate: X/15 (XX%)

NOTES:
[Any observations or issues]

STATUS: [APPROVED/NEEDS FIXES]
```

---

**Test Scenarios Created:** July 4, 2026
**Total Scenarios:** 15 comprehensive tests
**Ready for Execution:** ✅ YES

