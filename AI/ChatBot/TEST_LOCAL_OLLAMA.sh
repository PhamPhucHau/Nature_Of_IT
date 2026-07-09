#!/bin/bash

# Test Local Ollama Integration
# This script tests the ChatBot application with local Ollama installation

set -e

echo "========================================"
echo "LOCAL OLLAMA INTEGRATION TEST"
echo "========================================"
echo ""

API_BASE="http://localhost:8080/api/chat"

# Color codes for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test 1: Backend Health Check
echo -e "${YELLOW}[TEST 1]${NC} Backend Health Check"
HEALTH_RESPONSE=$(curl -s -w "\n%{http_code}" $API_BASE/health)
HTTP_CODE=$(echo "$HEALTH_RESPONSE" | tail -1)
BODY=$(echo "$HEALTH_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}✓ PASS${NC} Backend is healthy"
    echo "Response: $BODY"
else
    echo -e "${RED}✗ FAIL${NC} Backend health check failed (HTTP $HTTP_CODE)"
    exit 1
fi

echo ""

# Test 2: Create Conversation with llama2:13b-chat
echo -e "${YELLOW}[TEST 2]${NC} Create Conversation with llama2:13b-chat"
CONV_RESPONSE=$(curl -s -w "\n%{http_code}" -X POST $API_BASE/conversations \
  -H "Content-Type: application/json" \
  -d '{"model":"llama2:13b-chat"}')

HTTP_CODE=$(echo "$CONV_RESPONSE" | tail -1)
BODY=$(echo "$CONV_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "201" ]; then
    echo -e "${GREEN}✓ PASS${NC} Conversation created successfully"
    echo "Response: $BODY" | jq .
    
    CONV_ID=$(echo "$BODY" | jq -r '.id')
    echo "Conversation ID: $CONV_ID"
else
    echo -e "${RED}✗ FAIL${NC} Failed to create conversation (HTTP $HTTP_CODE)"
    echo "Response: $BODY"
    exit 1
fi

echo ""

# Test 3: Send Message (this will test Ollama connectivity)
echo -e "${YELLOW}[TEST 3]${NC} Send Message to Ollama"
echo "Sending: 'Hello, what is 2+2?'"

MESSAGE_RESPONSE=$(curl -s -w "\n%{http_code}" -X POST $API_BASE/send \
  -H "Content-Type: application/json" \
  -d "{
    \"conversationId\":\"$CONV_ID\",
    \"message\":\"Hello, what is 2+2?\",
    \"model\":\"llama2:13b-chat\"
  }")

HTTP_CODE=$(echo "$MESSAGE_RESPONSE" | tail -1)
BODY=$(echo "$MESSAGE_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}✓ PASS${NC} Message sent successfully"
    echo "Response:" 
    echo "$BODY" | jq .
    
    # Check if response contains content
    CONTENT=$(echo "$BODY" | jq -r '.content // empty')
    if [ -z "$CONTENT" ]; then
        echo -e "${YELLOW}⚠ WARNING${NC} Response content is empty - Ollama may not be responding"
    else
        echo -e "${GREEN}✓ Ollama Response Length: ${#CONTENT}${NC} characters"
        echo "First 100 chars: ${CONTENT:0:100}..."
    fi
else
    echo -e "${RED}✗ FAIL${NC} Failed to send message (HTTP $HTTP_CODE)"
    echo "Response: $BODY"
    echo ""
    echo "This error typically means:"
    echo "1. Local Ollama is not running at http://host.docker.internal:11434"
    echo "2. Model 'llama2:13b-chat' is not pulled in Ollama"
    echo ""
    echo "To fix:"
    echo "  1. Ensure Ollama is running: ollama serve"
    echo "  2. Pull the model: ollama pull llama2:13b-chat"
    echo "  3. Check available models: ollama list"
    exit 1
fi

echo ""

# Test 4: Get Conversation History
echo -e "${YELLOW}[TEST 4]${NC} Get Conversation History"
HISTORY_RESPONSE=$(curl -s -w "\n%{http_code}" $API_BASE/history/$CONV_ID)
HTTP_CODE=$(echo "$HISTORY_RESPONSE" | tail -1)
BODY=$(echo "$HISTORY_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "200" ]; then
    echo -e "${GREEN}✓ PASS${NC} History retrieved successfully"
    NUM_MESSAGES=$(echo "$BODY" | jq '. | length')
    echo "Total messages: $NUM_MESSAGES"
else
    echo -e "${RED}✗ FAIL${NC} Failed to get history (HTTP $HTTP_CODE)"
    echo "Response: $BODY"
    exit 1
fi

echo ""

# Test 5: Test Other Models
echo -e "${YELLOW}[TEST 5]${NC} Create Conversation with llama3:latest"
CONV_RESPONSE=$(curl -s -w "\n%{http_code}" -X POST $API_BASE/conversations \
  -H "Content-Type: application/json" \
  -d '{"model":"llama3:latest"}')

HTTP_CODE=$(echo "$CONV_RESPONSE" | tail -1)
BODY=$(echo "$CONV_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "201" ]; then
    echo -e "${GREEN}✓ PASS${NC} Llama3 conversation created"
else
    echo -e "${YELLOW}⚠ SKIP${NC} Llama3 test skipped (model may not be installed)"
fi

echo ""

# Test 6: Test Default Model (when no model specified)
echo -e "${YELLOW}[TEST 6]${NC} Create Conversation without specifying model (should use default)"
CONV_RESPONSE=$(curl -s -w "\n%{http_code}" -X POST $API_BASE/conversations \
  -H "Content-Type: application/json" \
  -d '{}')

HTTP_CODE=$(echo "$CONV_RESPONSE" | tail -1)
BODY=$(echo "$CONV_RESPONSE" | head -1)

if [ "$HTTP_CODE" = "201" ]; then
    echo -e "${GREEN}✓ PASS${NC} Default model conversation created"
    MODEL=$(echo "$BODY" | jq -r '.model')
    echo "Default model used: $MODEL"
else
    echo -e "${RED}✗ FAIL${NC} Failed to create default conversation (HTTP $HTTP_CODE)"
fi

echo ""
echo "========================================"
echo -e "${GREEN}ALL TESTS COMPLETED${NC}"
echo "========================================"

