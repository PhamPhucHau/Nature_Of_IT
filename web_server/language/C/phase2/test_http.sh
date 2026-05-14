#!/bin/bash

# HTTP Server Test Script
# Tests all HTTP features

SERVER_URL="http://localhost:8080"
TESTS_PASSED=0
TESTS_FAILED=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "HTTP Server Test Suite"
echo "========================================"
echo ""

# Function to run a test
run_test() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    
    if [ "$expected" == "$actual" ]; then
        echo -e "${GREEN}✓${NC} $test_name"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗${NC} $test_name"
        echo "  Expected: $expected"
        echo "  Actual: $actual"
        ((TESTS_FAILED++))
    fi
}

# Function to run a test with partial match
run_test_contains() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    
    if echo "$actual" | grep -q "$expected"; then
        echo -e "${GREEN}✓${NC} $test_name"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗${NC} $test_name"
        echo "  Expected to contain: $expected"
        echo "  Actual: $actual"
        ((TESTS_FAILED++))
    fi
}

# Function to run a test with status code
run_status_test() {
    local test_name="$1"
    local expected_code="$2"
    local url="$3"
    
    local status_code=$(curl -s -o /dev/null -w "%{http_code}" "$url" 2>/dev/null)
    
    if [ "$status_code" == "$expected_code" ]; then
        echo -e "${GREEN}✓${NC} $test_name (Status: $status_code)"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}✗${NC} $test_name"
        echo "  Expected status: $expected_code"
        echo "  Actual status: $status_code"
        ((TESTS_FAILED++))
    fi
}

echo "1. Testing Basic HTTP Request"
echo "-------------------------------"

# Test 1: Basic GET request
response=$(curl -s "$SERVER_URL/" 2>/dev/null)
run_test_contains "Basic GET request" "<!DOCTYPE html>" "$response"

# Test 2: Check status code
run_status_test "Status code 200" "200" "$SERVER_URL/"

echo ""
echo "2. Testing 404 Error Handling"
echo "-------------------------------"

# Test 3: Non-existent file
run_status_test "404 for non-existent file" "404" "$SERVER_URL/nonexistent.html"

# Test 4: 404 response contains error message
response=$(curl -s "$SERVER_URL/nonexistent.html" 2>/dev/null)
run_test_contains "404 response body" "404" "$response"

echo ""
echo "3. Testing Content Types"
echo "-------------------------------"

# Test 5: HTML content type
response=$(curl -sI "$SERVER_URL/" 2>/dev/null)
run_test_contains "HTML Content-Type" "text/html" "$response"

# Test 6: CSS content type
response=$(curl -sI "$SERVER_URL/styles.css" 2>/dev/null)
run_test_contains "CSS Content-Type" "text/css" "$response"

# Test 7: JavaScript content type
response=$(curl -sI "$SERVER_URL/app.js" 2>/dev/null)
run_test_contains "JS Content-Type" "application/javascript" "$response"

echo ""
echo "4. Testing HTTP Methods"
echo "-------------------------------"

# Test 8: GET method
run_status_test "GET method" "200" "$SERVER_URL/"

# Test 9: HEAD method
status=$(curl -sI "$SERVER_URL/" 2>/dev/null | head -1)
run_test_contains "HEAD method" "200" "$status"

# Test 10: OPTIONS method
status=$(curl -s -X OPTIONS -I "$SERVER_URL/" 2>/dev/null | head -1)
run_test_contains "OPTIONS method" "200" "$status"

echo ""
echo "5. Testing Response Headers"
echo "-------------------------------"

# Test 11: Content-Length header
response=$(curl -sI "$SERVER_URL/" 2>/dev/null)
run_test_contains "Content-Length header" "Content-Length:" "$response"

# Test 12: Server header
response=$(curl -sI "$SERVER_URL/" 2>/dev/null)
run_test_contains "Server header" "Server:" "$response"

echo ""
echo "========================================"
echo "Test Results"
echo "========================================"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo "Total:  $((TESTS_PASSED + TESTS_FAILED))"
echo "========================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi