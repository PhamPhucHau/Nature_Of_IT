#!/bin/bash

################################################################################
#                     ChatBot Test Execution Script
#                      Automated Test Runner
################################################################################

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BACKEND_URL="http://localhost:8080"
FRONTEND_URL="http://localhost:3000"
TEST_RESULTS="test_results.txt"
PASS_COUNT=0
FAIL_COUNT=0

################################################################################
# Helper Functions
################################################################################

print_header() {
  echo -e "\n${BLUE}═════════════════════════════════════════════════════════════${NC}"
  echo -e "${BLUE}$1${NC}"
  echo -e "${BLUE}═════════════════════════════════════════════════════════════${NC}\n"
}

print_test() {
  echo -e "${YELLOW}TEST: $1${NC}"
}

print_pass() {
  echo -e "${GREEN}✓ PASS: $1${NC}"
  ((PASS_COUNT++))
  echo "[PASS] $1" >> $TEST_RESULTS
}

print_fail() {
  echo -e "${RED}✗ FAIL: $1${NC}"
  ((FAIL_COUNT++))
  echo "[FAIL] $1" >> $TEST_RESULTS
}

print_info() {
  echo -e "${BLUE}ℹ $1${NC}"
}

# Check if service is running
check_service() {
  local url=$1
  local name=$2
  
  if curl -s "$url" > /dev/null 2>&1; then
    print_pass "$name is running"
    return 0
  else
    print_fail "$name is not running at $url"
    return 1
  fi
}

################################################################################
# Test Suite 1: Health Checks
################################################################################

test_suite_health() {
  print_header "Test Suite 1: Health Checks"
  
  print_test "Backend health endpoint"
  response=$(curl -s -w "\n%{http_code}" "$BACKEND_URL/api/chat/health")
  status_code=$(echo "$response" | tail -n 1)
  body=$(echo "$response" | head -n -1)
  
  if [ "$status_code" = "200" ]; then
    print_pass "Backend health check (HTTP $status_code)"
    if echo "$body" | grep -q "UP"; then
      print_pass "Backend status is UP"
    else
      print_fail "Backend status is not UP"
    fi
    if echo "$body" | grep -q "requestId"; then
      print_pass "Backend response includes requestId"
    else
      print_fail "Backend response missing requestId"
    fi
  else
    print_fail "Backend health check (HTTP $status_code)"
  fi
}

################################################################################
# Test Suite 2: Conversation Management
################################################################################

test_suite_conversations() {
  print_header "Test Suite 2: Conversation Management"
  
  print_test "Create conversation"
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  CONV_ID=$(echo "$response" | jq -r '.id // empty')
  
  if [ -n "$CONV_ID" ] && [ "$CONV_ID" != "null" ]; then
    print_pass "Conversation created with ID: $CONV_ID"
    
    # Verify conversation properties
    if echo "$response" | jq -e '.model == "llama2"' > /dev/null; then
      print_pass "Conversation has correct model"
    else
      print_fail "Conversation model mismatch"
    fi
    
    if echo "$response" | jq -e '.title' > /dev/null; then
      print_pass "Conversation has title"
    else
      print_fail "Conversation missing title"
    fi
    
    if echo "$response" | jq -e '.messages | length == 0' > /dev/null; then
      print_pass "Conversation messages initially empty"
    else
      print_fail "Conversation should have empty messages"
    fi
    
  else
    print_fail "Failed to create conversation"
    return 1
  fi
  
  # Test get all conversations
  print_test "Get all conversations"
  all_convs=$(curl -s "$BACKEND_URL/api/chat/conversations")
  
  if echo "$all_convs" | jq -e '. | length > 0' > /dev/null; then
    print_pass "Get conversations returns list"
  else
    print_fail "Get conversations failed"
  fi
}

################################################################################
# Test Suite 3: Message Handling
################################################################################

test_suite_messages() {
  print_header "Test Suite 3: Message Handling"
  
  # First create a conversation
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  CONV_ID=$(echo "$response" | jq -r '.id')
  print_info "Using conversation: $CONV_ID"
  
  # Get initial history (should be empty)
  print_test "Get conversation history (initial)"
  history=$(curl -s "$BACKEND_URL/api/chat/history/$CONV_ID")
  
  if echo "$history" | jq -e '. | length == 0' > /dev/null; then
    print_pass "Initial history is empty"
  else
    print_fail "Initial history should be empty"
  fi
}

################################################################################
# Test Suite 4: Request ID Tracing
################################################################################

test_suite_request_id() {
  print_header "Test Suite 4: Request ID Tracing"
  
  print_test "Request ID generation and propagation"
  response=$(curl -s -i -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}' 2>&1)
  
  # Extract request ID from headers
  req_id=$(echo "$response" | grep -i "x-request-id" | awk '{print $2}' | tr -d '\r')
  
  if [ -n "$req_id" ]; then
    print_pass "Request ID generated: $req_id"
    
    # Verify format
    if [[ $req_id =~ ^req_[a-f0-9]+$ ]]; then
      print_pass "Request ID format is correct"
    else
      print_fail "Request ID format is incorrect: $req_id"
    fi
  else
    print_fail "Request ID not found in response headers"
  fi
}

################################################################################
# Test Suite 5: Error Handling
################################################################################

test_suite_errors() {
  print_header "Test Suite 5: Error Handling"
  
  print_test "Missing required field (conversationId)"
  response=$(curl -s -w "\n%{http_code}" -X POST "$BACKEND_URL/api/chat/send" \
    -H "Content-Type: application/json" \
    -d '{"message":"Hello","model":"llama2"}')
  
  status_code=$(echo "$response" | tail -n 1)
  body=$(echo "$response" | head -n -1)
  
  if [ "$status_code" = "400" ]; then
    print_pass "Validation error returns HTTP 400"
  else
    print_fail "Expected HTTP 400, got $status_code"
  fi
  
  if echo "$body" | jq -e '.requestId' > /dev/null; then
    print_pass "Error response includes requestId"
  else
    print_fail "Error response missing requestId"
  fi
  
  print_test "Non-existent conversation"
  response=$(curl -s -w "\n%{http_code}" "$BACKEND_URL/api/chat/history/invalid-id-xyz")
  status_code=$(echo "$response" | tail -n 1)
  
  if [ "$status_code" = "404" ]; then
    print_pass "Non-existent conversation returns HTTP 404"
  else
    print_fail "Expected HTTP 404, got $status_code"
  fi
}

################################################################################
# Test Suite 6: Database Persistence
################################################################################

test_suite_persistence() {
  print_header "Test Suite 6: Database Persistence"
  
  print_test "Check PostgreSQL container"
  if docker ps | grep -q chatbot-postgres; then
    print_pass "PostgreSQL container is running"
  else
    print_fail "PostgreSQL container is not running"
    return 1
  fi
  
  print_test "Database connectivity"
  if docker exec chatbot-postgres pg_isready -U chatbot_user > /dev/null 2>&1; then
    print_pass "Can connect to PostgreSQL database"
  else
    print_fail "Cannot connect to PostgreSQL database"
  fi
}

################################################################################
# Test Suite 7: Frontend
################################################################################

test_suite_frontend() {
  print_header "Test Suite 7: Frontend"
  
  print_test "Frontend application loads"
  response=$(curl -s -w "\n%{http_code}" "$FRONTEND_URL")
  status_code=$(echo "$response" | tail -n 1)
  body=$(echo "$response" | head -n -1)
  
  if [ "$status_code" = "200" ]; then
    print_pass "Frontend returns HTTP 200"
    
    if echo "$body" | grep -q "ChatBot"; then
      print_pass "Frontend HTML contains 'ChatBot'"
    else
      print_fail "Frontend HTML missing 'ChatBot'"
    fi
    
    if echo "$body" | grep -q "root"; then
      print_pass "Frontend has React root element"
    else
      print_fail "Frontend missing React root element"
    fi
    
  else
    print_fail "Frontend returns HTTP $status_code"
  fi
}

################################################################################
# Test Suite 8: Service Dependencies
################################################################################

test_suite_services() {
  print_header "Test Suite 8: Service Dependencies"
  
  print_test "Backend service"
  check_service "$BACKEND_URL/api/chat/health" "Backend"
  
  print_test "Frontend service"
  check_service "$FRONTEND_URL" "Frontend"
  
  print_test "Ollama service"
  if check_service "http://localhost:11434/api/tags" "Ollama"; then
    print_info "Ollama is ready for LLM inference"
  fi
}

################################################################################
# Performance Tests
################################################################################

test_suite_performance() {
  print_header "Test Suite 9: Performance"
  
  print_test "Create conversation response time"
  
  start_time=$(date +%s%N)
  
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  end_time=$(date +%s%N)
  duration=$((($end_time - $start_time) / 1000000)) # Convert to ms
  
  if [ "$duration" -lt 500 ]; then
    print_pass "Create conversation: ${duration}ms (< 500ms)"
  else
    print_fail "Create conversation: ${duration}ms (> 500ms)"
  fi
}

################################################################################
# Main Execution
################################################################################

main() {
  print_header "ChatBot Application Test Suite"
  
  # Initialize results file
  > $TEST_RESULTS
  
  # Check prerequisites
  print_info "Checking prerequisites..."
  
  if ! command -v curl &> /dev/null; then
    echo "ERROR: curl is required but not installed"
    exit 1
  fi
  
  if ! command -v jq &> /dev/null; then
    echo "ERROR: jq is required but not installed"
    exit 1
  fi
  
  if ! command -v docker &> /dev/null; then
    echo "ERROR: docker is required but not installed"
    exit 1
  fi
  
  print_pass "All prerequisites installed"
  
  # Run all test suites
  test_suite_health
  test_suite_services
  test_suite_conversations
  test_suite_messages
  test_suite_request_id
  test_suite_errors
  test_suite_persistence
  test_suite_frontend
  test_suite_performance
  
  # Print summary
  print_header "Test Summary"
  
  total=$((PASS_COUNT + FAIL_COUNT))
  pass_rate=$((PASS_COUNT * 100 / total))
  
  echo -e "${GREEN}✓ Passed: $PASS_COUNT${NC}"
  echo -e "${RED}✗ Failed: $FAIL_COUNT${NC}"
  echo -e "${BLUE}━ Total:  $total${NC}"
  echo -e "${YELLOW}📊 Pass Rate: $pass_rate%${NC}\n"
  
  if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    echo -e "\nDetailed results saved to: $TEST_RESULTS"
    return 0
  else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    echo -e "\nDetailed results saved to: $TEST_RESULTS"
    return 1
  fi
}

# Run main function
main "$@"
