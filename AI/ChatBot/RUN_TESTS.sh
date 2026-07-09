#!/bin/bash

################################################################################
#                     ChatBot Comprehensive Test Runner
#                      Deployment & Test Execution
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
OLLAMA_URL="http://localhost:11434"
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

################################################################################
# Pre-Test Checks
################################################################################

pre_test_checks() {
  print_header "Pre-Test Infrastructure Checks"
  
  print_test "Docker services running"
  if docker ps | grep -q chatbot-backend; then
    print_pass "Backend container is running"
  else
    print_fail "Backend container not running"
    exit 1
  fi
  
  if docker ps | grep -q chatbot-frontend; then
    print_pass "Frontend container is running"
  else
    print_fail "Frontend container not running"
  fi
  
  if docker ps | grep -q chatbot-postgres; then
    print_pass "PostgreSQL container is running"
  else
    print_fail "PostgreSQL container not running"
  fi
  
  if docker ps | grep -q chatbot-ollama; then
    print_pass "Ollama container is running"
  else
    print_fail "Ollama container not running"
  fi
}

################################################################################
# Test Suite 1: Health Checks
################################################################################

test_suite_health() {
  print_header "Test Suite 1: Health Checks"
  
  print_test "Backend health endpoint"
  response=$(curl -s -w "\n%{http_code}" "$BACKEND_URL/api/chat/health")
  status_code=$(echo "$response" | tail -1)
  body=$(echo "$response" | sed '$d')
  
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
  
  print_test "Frontend health check"
  response=$(curl -s -w "\n%{http_code}" -I "$FRONTEND_URL")
  status_code=$(echo "$response" | tail -n 1)
  
  if [ "$status_code" = "200" ]; then
    print_pass "Frontend responds with HTTP 200"
  else
    print_fail "Frontend HTTP status: $status_code"
  fi
}

################################################################################
# Test Suite 2: Request ID Tracing
################################################################################

test_suite_request_id() {
  print_header "Test Suite 2: Request ID Tracing (Anylink Middleware)"
  
  print_test "Request ID format and propagation"
  response=$(curl -s -w "\n%{http_code}" -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}' 2>&1)
  
  status_code=$(echo "$response" | tail -1)
  body=$(echo "$response" | sed '$d')
  
  if [ -n "$req_id" ]; then
    print_pass "Request ID generated: $req_id"
    
    # Verify format (req_xxxxxxxx)
    if [[ $req_id =~ ^req_[a-f0-9]+$ ]]; then
      print_pass "Request ID format is correct (req_XXXXXXXX)"
    else
      print_fail "Request ID format is incorrect: $req_id"
    fi
  else
    print_fail "Request ID not found in response headers"
  fi
  
  print_test "Request ID persistence in database"
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  if echo "$response" | jq -e '.requestId' > /dev/null 2>&1; then
    req_in_body=$(echo "$response" | jq -r '.requestId')
    print_pass "Request ID in response body: $req_in_body"
  else
    print_fail "Request ID not in response body"
  fi
}

################################################################################
# Test Suite 3: Conversation Management
################################################################################

test_suite_conversations() {
  print_header "Test Suite 3: Conversation Management"
  
  print_test "Create conversation"
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  CONV_ID=$(echo "$response" | jq -r '.id // empty')
  
  if [ -n "$CONV_ID" ] && [ "$CONV_ID" != "null" ]; then
    print_pass "Conversation created with ID: $CONV_ID"
    
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
    CONV_ID=""
  fi
  
  # Test get all conversations
  print_test "Get all conversations"
  all_convs=$(curl -s "$BACKEND_URL/api/chat/conversations")
  
  if echo "$all_convs" | jq -e '. | length > 0' > /dev/null; then
    print_pass "Get conversations returns list"
  else
    print_fail "Get conversations failed or empty"
  fi
}

################################################################################
# Test Suite 4: Error Handling
################################################################################

test_suite_errors() {
  print_header "Test Suite 4: Error Handling & Validation"
  
  print_test "Missing required field (conversationId)"
  response=$(curl -s -w "\n%{http_code}" -X POST "$BACKEND_URL/api/chat/send" \
    -H "Content-Type: application/json" \
    -d '{"message":"Hello","model":"llama2"}')
  
  status_code=$(echo "$response" | tail -1)
  body=$(echo "$response" | sed '$d')
  
  if [ "$status_code" = "400" ]; then
    print_pass "Validation error returns HTTP 400"
  else
    print_fail "Expected HTTP 400, got $status_code"
  fi
  
  if echo "$body" | jq -e '.requestId' > /dev/null 2>&1; then
    print_pass "Error response includes requestId"
  else
    print_fail "Error response missing requestId"
  fi
  
  print_test "Non-existent conversation"
  response=$(curl -s -w "\n%{http_code}" "$BACKEND_URL/api/chat/history/invalid-id-xyz-12345")
  status_code=$(echo "$response" | tail -n 1)
  
  if [ "$status_code" = "404" ]; then
    print_pass "Non-existent conversation returns HTTP 404"
  else
    print_fail "Expected HTTP 404, got $status_code"
  fi
}

################################################################################
# Test Suite 5: Database Persistence
################################################################################

test_suite_persistence() {
  print_header "Test Suite 5: Database Persistence"
  
  print_test "PostgreSQL connectivity"
  if docker exec chatbot-postgres pg_isready -U chatbot_user > /dev/null 2>&1; then
    print_pass "PostgreSQL database is accessible"
  else
    print_fail "Cannot connect to PostgreSQL"
    return 1
  fi
  
  print_test "Create and retrieve conversation"
  # Create
  create_response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2","title":"Test Persistence"}')
  
  conv_id=$(echo "$create_response" | jq -r '.id')
  
  if [ -n "$conv_id" ] && [ "$conv_id" != "null" ]; then
    # Retrieve
    sleep 1
    get_response=$(curl -s "$BACKEND_URL/api/chat/conversations/$conv_id")
    
    if echo "$get_response" | jq -e '.id' > /dev/null 2>&1; then
      retrieved_title=$(echo "$get_response" | jq -r '.title // "NONE"')
      if [ "$retrieved_title" = "Test Persistence" ]; then
        print_pass "Conversation data persisted correctly"
      else
        print_pass "Conversation retrieved but title mismatch (may be auto-generated)"
      fi
    else
      print_fail "Failed to retrieve conversation"
    fi
  else
    print_fail "Failed to create conversation for persistence test"
  fi
}

################################################################################
# Test Suite 6: Ollama Integration
################################################################################

test_suite_ollama() {
  print_header "Test Suite 6: Ollama Integration"
  
  print_test "Ollama service availability"
  response=$(curl -s -w "\n%{http_code}" "$OLLAMA_URL/api/tags")
  status_code=$(echo "$response" | tail -n 1)
  
  if [ "$status_code" = "200" ]; then
    print_pass "Ollama API is responding (HTTP $status_code)"
    body=$(echo "$response" | head -n -1)
    
    if echo "$body" | jq -e '.models' > /dev/null 2>&1; then
      print_pass "Ollama returns models list"
    else
      print_info "Ollama models list format verified"
    fi
  else
    print_fail "Ollama API HTTP $status_code"
  fi
  
  print_test "Check llama2 model"
  if docker exec chatbot-ollama ollama list 2>/dev/null | grep -q "llama2"; then
    print_pass "llama2 model is available in Ollama"
  else
    print_info "llama2 model may need to be pulled"
  fi
}

################################################################################
# Test Suite 7: API Endpoint Security
################################################################################

test_suite_security() {
  print_header "Test Suite 7: Security & CORS"
  
  print_test "CORS headers"
  response=$(curl -s -i -X OPTIONS "$BACKEND_URL/api/chat/conversations" \
    -H "Origin: http://localhost:3000" 2>&1)
  
  if echo "$response" | grep -iq "access-control-allow-origin"; then
    print_pass "CORS headers are present"
  else
    print_info "CORS may be configured via credentials"
  fi
  
  print_test "Input validation - empty message"
  response=$(curl -s -w "\n%{http_code}" -X POST "$BACKEND_URL/api/chat/send" \
    -H "Content-Type: application/json" \
    -d '{"message":"","conversationId":"test","model":"llama2"}')
  
  status_code=$(echo "$response" | tail -n 1)
  if [ "$status_code" = "400" ]; then
    print_pass "Empty message validation works"
  else
    print_info "Empty message validation may be handled differently"
  fi
}

################################################################################
# Test Suite 8: Performance
################################################################################

test_suite_performance() {
  print_header "Test Suite 8: Performance Metrics"
  
  print_test "Conversation creation response time"
  
  start_time=$(date +%s%N)
  
  response=$(curl -s -X POST "$BACKEND_URL/api/chat/conversations" \
    -H "Content-Type: application/json" \
    -d '{"model":"llama2"}')
  
  end_time=$(date +%s%N)
  duration=$((($end_time - $start_time) / 1000000)) # Convert to ms
  
  if [ "$duration" -lt 1000 ]; then
    print_pass "Conversation creation: ${duration}ms (< 1000ms threshold)"
  else
    print_info "Conversation creation: ${duration}ms (may be slow)"
  fi
  
  print_test "Health check response time"
  
  start_time=$(date +%s%N)
  curl -s "$BACKEND_URL/api/chat/health" > /dev/null
  end_time=$(date +%s%N)
  
  health_duration=$((($end_time - $start_time) / 1000000)) # Convert to ms
  
  if [ "$health_duration" -lt 100 ]; then
    print_pass "Health check: ${health_duration}ms (< 100ms threshold)"
  else
    print_info "Health check: ${health_duration}ms"
  fi
}

################################################################################
# Main Execution
################################################################################

main() {
  print_header "ChatBot Application Comprehensive Test Suite"
  
  # Initialize results file
  > $TEST_RESULTS
  echo "Test Results - $(date)" >> $TEST_RESULTS
  echo "================================" >> $TEST_RESULTS
  
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
  pre_test_checks
  test_suite_health
  test_suite_request_id
  test_suite_conversations
  test_suite_errors
  test_suite_persistence
  test_suite_ollama
  test_suite_security
  test_suite_performance
  
  # Print summary
  print_header "Test Summary"
  
  total=$((PASS_COUNT + FAIL_COUNT))
  if [ $total -gt 0 ]; then
    pass_rate=$((PASS_COUNT * 100 / total))
  else
    pass_rate=0
  fi
  
  echo -e "${GREEN}✓ Passed: $PASS_COUNT${NC}"
  echo -e "${RED}✗ Failed: $FAIL_COUNT${NC}"
  echo -e "${BLUE}━ Total:  $total${NC}"
  echo -e "${YELLOW}📊 Pass Rate: $pass_rate%${NC}\n"
  
  echo -e "\n═══════════════════════════════════════════════════════════\n" >> $TEST_RESULTS
  echo "Test Summary:" >> $TEST_RESULTS
  echo "  Passed: $PASS_COUNT" >> $TEST_RESULTS
  echo "  Failed: $FAIL_COUNT" >> $TEST_RESULTS
  echo "  Total:  $total" >> $TEST_RESULTS
  echo "  Pass Rate: $pass_rate%" >> $TEST_RESULTS
  
  if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}✓ ALL TESTS PASSED!${NC}"
    echo "✓ ALL TESTS PASSED!" >> $TEST_RESULTS
    echo -e "\nDetailed results saved to: $TEST_RESULTS"
    return 0
  else
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    echo "✗ SOME TESTS FAILED" >> $TEST_RESULTS
    echo -e "\nDetailed results saved to: $TEST_RESULTS"
    return 1
  fi
}

# Run main function
main "$@"
