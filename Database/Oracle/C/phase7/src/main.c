/*
 * main.c - Client-Server Test Program
 * 
 * Comprehensive test suite for client-server implementation.
 * Tests server, client, connection pool, and protocol.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "server.h"
#include "client.h"
#include "connection_pool.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ============================================================================
 * TEST UTILITIES
 * ============================================================================ */

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define RESET   "\x1b[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void assert_equal(int actual, int expected, const char *test_name) {
    if (actual == expected) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (expected %d, got %d)\n",
               test_name, expected, actual);
        tests_failed++;
    }
}

void assert_not_null(void *ptr, const char *test_name) {
    if (ptr != NULL) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (NULL pointer)\n", test_name);
        tests_failed++;
    }
}

void assert_true(int condition, const char *test_name) {
    if (condition) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (condition false)\n", test_name);
        tests_failed++;
    }
}

/* ============================================================================
 * SERVER TESTS
 * ============================================================================ */

void test_server_create_destroy() {
    printf("\n" YELLOW "=== Test: Server Create/Destroy ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    assert_not_null(server, "Server created");
    assert_equal(server->port, 5432, "Server port is 5432");
    assert_equal(server->is_running, 0, "Server not running");
    
    server_destroy(server);
}

void test_server_start_stop() {
    printf("\n" YELLOW "=== Test: Server Start/Stop ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    
    int result = server_start(server);
    assert_equal(result, 0, "Server started");
    assert_equal(server->is_running, 1, "Server is running");
    
    result = server_stop(server);
    assert_equal(result, 0, "Server stopped");
    assert_equal(server->is_running, 0, "Server not running");
    
    server_destroy(server);
}

void test_server_accept_connection() {
    printf("\n" YELLOW "=== Test: Server Accept Connection ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    server_start(server);
    
    uint32_t conn_id = server_accept_connection(server);
    assert_true(conn_id > 0, "Connection accepted");
    assert_equal(server->conn_count, 1, "Connection count is 1");
    
    server_destroy(server);
}

void test_server_close_connection() {
    printf("\n" YELLOW "=== Test: Server Close Connection ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    server_start(server);
    
    uint32_t conn_id = server_accept_connection(server);
    int result = server_close_connection(server, conn_id);
    assert_equal(result, 0, "Connection closed");
    
    server_destroy(server);
}

void test_server_process_request() {
    printf("\n" YELLOW "=== Test: Server Process Request ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    server_start(server);
    
    uint32_t conn_id = server_accept_connection(server);
    
    ClientRequest request;
    request.type = REQ_QUERY;
    strcpy(request.query, "SELECT * FROM users");
    request.client_id = 1;
    
    ServerResponse response;
    int result = server_process_request(server, conn_id, &request, &response);
    assert_equal(result, 0, "Request processed");
    assert_equal(response.status, RESP_OK, "Response OK");
    
    server_destroy(server);
}

void test_server_get_connection_count() {
    printf("\n" YELLOW "=== Test: Server Get Connection Count ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    server_start(server);
    
    server_accept_connection(server);
    server_accept_connection(server);
    
    uint32_t count = server_get_connection_count(server);
    assert_equal(count, 2, "Connection count is 2");
    
    server_destroy(server);
}

/* ============================================================================
 * CLIENT TESTS
 * ============================================================================ */

void test_client_create_destroy() {
    printf("\n" YELLOW "=== Test: Client Create/Destroy ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    assert_not_null(client, "Client created");
    assert_equal(client->port, 5432, "Client port is 5432");
    assert_equal(client->status, CLIENT_DISCONNECTED, "Client disconnected");
    
    client_destroy(client);
}

void test_client_connect_disconnect() {
    printf("\n" YELLOW "=== Test: Client Connect/Disconnect ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    
    int result = client_connect(client);
    assert_equal(result, 0, "Client connected");
    assert_equal(client->status, CLIENT_CONNECTED, "Client status is connected");
    
    result = client_disconnect(client);
    assert_equal(result, 0, "Client disconnected");
    assert_equal(client->status, CLIENT_DISCONNECTED, "Client status is disconnected");
    
    client_destroy(client);
}

void test_client_is_connected() {
    printf("\n" YELLOW "=== Test: Client Is Connected ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    
    assert_equal(client_is_connected(client), 0, "Client not connected initially");
    
    client_connect(client);
    assert_equal(client_is_connected(client), 1, "Client is connected");
    
    client_destroy(client);
}

void test_client_execute_query() {
    printf("\n" YELLOW "=== Test: Client Execute Query ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    client_connect(client);
    
    QueryResult result;
    int ret = client_execute_query(client, "SELECT * FROM users", &result);
    assert_equal(ret, 0, "Query executed");
    assert_equal(result.success, 1, "Query successful");
    
    client_destroy(client);
}

void test_client_ping() {
    printf("\n" YELLOW "=== Test: Client Ping ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    client_connect(client);
    
    int result = client_ping(client);
    assert_equal(result, 0, "Ping successful");
    
    client_destroy(client);
}

void test_client_get_query_count() {
    printf("\n" YELLOW "=== Test: Client Get Query Count ===" RESET "\n");
    
    DatabaseClient *client = client_create("localhost", 5432);
    client_connect(client);
    
    QueryResult result;
    client_execute_query(client, "SELECT 1", &result);
    client_execute_query(client, "SELECT 2", &result);
    
    uint64_t count = client_get_query_count(client);
    assert_equal(count, 2, "Query count is 2");
    
    client_destroy(client);
}

/* ============================================================================
 * CONNECTION POOL TESTS
 * ============================================================================ */

void test_connection_pool_create_destroy() {
    printf("\n" YELLOW "=== Test: Connection Pool Create/Destroy ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
    assert_not_null(pool, "Connection pool created");
    assert_equal(pool->max_size, 10, "Max size is 10");
    
    connection_pool_destroy(pool);
}

void test_connection_pool_get_connection() {
    printf("\n" YELLOW "=== Test: Connection Pool Get Connection ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
    
    DatabaseClient *client = connection_pool_get_connection(pool);
    assert_not_null(client, "Connection obtained");
    assert_equal(pool->active_count, 1, "Active count is 1");
    
    connection_pool_destroy(pool);
}

void test_connection_pool_return_connection() {
    printf("\n" YELLOW "=== Test: Connection Pool Return Connection ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
    
    DatabaseClient *client = connection_pool_get_connection(pool);
    int result = connection_pool_return_connection(pool, client);
    assert_equal(result, 0, "Connection returned");
    assert_equal(pool->idle_count, 6, "Idle count is 6");
    
    connection_pool_destroy(pool);
}

void test_connection_pool_get_idle_count() {
    printf("\n" YELLOW "=== Test: Connection Pool Get Idle Count ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
    
    uint32_t idle = connection_pool_get_idle_count(pool);
    assert_equal(idle, 5, "Idle count is 5");
    
    connection_pool_destroy(pool);
}

void test_connection_pool_get_active_count() {
    printf("\n" YELLOW "=== Test: Connection Pool Get Active Count ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 5, 10);
    
    DatabaseClient *client = connection_pool_get_connection(pool);
    uint32_t active = connection_pool_get_active_count(pool);
    assert_equal(active, 1, "Active count is 1");
    
    connection_pool_destroy(pool);
}

/* ============================================================================
 * PROTOCOL TESTS
 * ============================================================================ */

void test_protocol_create_message() {
    printf("\n" YELLOW "=== Test: Protocol Create Message ===" RESET "\n");
    
    Message *msg = protocol_create_message(MSG_QUERY, "SELECT 1", 8);
    assert_not_null(msg, "Message created");
    assert_equal(msg->header.version, PROTOCOL_VERSION, "Version correct");
    assert_equal(msg->header.type, MSG_QUERY, "Type is QUERY");
    
    protocol_destroy_message(msg);
}

void test_protocol_serialize_message() {
    printf("\n" YELLOW "=== Test: Protocol Serialize Message ===" RESET "\n");
    
    Message *msg = protocol_create_message(MSG_QUERY, "SELECT 1", 8);
    
    char buffer[1024];
    int size = protocol_serialize_message(msg, buffer, sizeof(buffer));
    assert_true(size > 0, "Message serialized");
    
    protocol_destroy_message(msg);
}

void test_protocol_deserialize_message() {
    printf("\n" YELLOW "=== Test: Protocol Deserialize Message ===" RESET "\n");
    
    Message *msg1 = protocol_create_message(MSG_QUERY, "SELECT 1", 8);
    
    char buffer[1024];
    int size = protocol_serialize_message(msg1, buffer, sizeof(buffer));
    
    Message msg2;
    int read_size = protocol_deserialize_message(buffer, size, &msg2);
    assert_equal(read_size, size, "Message deserialized");
    assert_equal(msg2.header.type, MSG_QUERY, "Type is QUERY");
    
    protocol_destroy_message(msg1);
}

void test_protocol_create_query_message() {
    printf("\n" YELLOW "=== Test: Protocol Create Query Message ===" RESET "\n");
    
    Message *msg = protocol_create_query_message(1, 0, "SELECT * FROM users");
    assert_not_null(msg, "Query message created");
    assert_equal(msg->header.type, MSG_QUERY, "Type is QUERY");
    
    protocol_destroy_message(msg);
}

void test_protocol_create_result_message() {
    printf("\n" YELLOW "=== Test: Protocol Create Result Message ===" RESET "\n");
    
    Message *msg = protocol_create_result_message(1, 0, 10, "result data");
    assert_not_null(msg, "Result message created");
    assert_equal(msg->header.type, MSG_RESULT, "Type is RESULT");
    
    protocol_destroy_message(msg);
}

void test_protocol_create_error_message() {
    printf("\n" YELLOW "=== Test: Protocol Create Error Message ===" RESET "\n");
    
    Message *msg = protocol_create_error_message(1, 1, "Error occurred");
    assert_not_null(msg, "Error message created");
    assert_equal(msg->header.type, MSG_ERROR, "Type is ERROR");
    
    protocol_destroy_message(msg);
}

/* ============================================================================
 * INTEGRATION TESTS
 * ============================================================================ */

void test_server_client_integration() {
    printf("\n" YELLOW "=== Test: Server-Client Integration ===" RESET "\n");
    
    DatabaseServer *server = server_create(5432);
    server_start(server);
    
    DatabaseClient *client = client_create("localhost", 5432);
    client_connect(client);
    
    uint32_t conn_id = server_accept_connection(server);
    
    ClientRequest request;
    request.type = REQ_QUERY;
    strcpy(request.query, "SELECT 1");
    
    ServerResponse response;
    server_process_request(server, conn_id, &request, &response);
    
    assert_equal(response.status, RESP_OK, "Response OK");
    
    server_destroy(server);
    client_destroy(client);
}

void test_connection_pool_integration() {
    printf("\n" YELLOW "=== Test: Connection Pool Integration ===" RESET "\n");
    
    ConnectionPool *pool = connection_pool_create("localhost", 5432, 3, 5);
    
    DatabaseClient *c1 = connection_pool_get_connection(pool);
    DatabaseClient *c2 = connection_pool_get_connection(pool);
    
    assert_equal(pool->active_count, 2, "Active count is 2");
    
    connection_pool_return_connection(pool, c1);
    assert_equal(pool->idle_count, 2, "Idle count is 2");
    
    connection_pool_destroy(pool);
}

void test_protocol_integration() {
    printf("\n" YELLOW "=== Test: Protocol Integration ===" RESET "\n");
    
    Message *query_msg = protocol_create_query_message(1, 0, "SELECT 1");
    
    char buffer[1024];
    int size = protocol_serialize_message(query_msg, buffer, sizeof(buffer));
    
    Message result_msg;
    protocol_deserialize_message(buffer, size, &result_msg);
    
    assert_equal(result_msg.header.type, MSG_QUERY, "Message type preserved");
    
    protocol_destroy_message(query_msg);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main(void) {
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║        Client-Server Test Suite - Phase 7                  ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    /* Server tests */
    test_server_create_destroy();
    test_server_start_stop();
    test_server_accept_connection();
    test_server_close_connection();
    test_server_process_request();
    test_server_get_connection_count();
    
    /* Client tests */
    test_client_create_destroy();
    test_client_connect_disconnect();
    test_client_is_connected();
    test_client_execute_query();
    test_client_ping();
    test_client_get_query_count();
    
    /* Connection Pool tests */
    test_connection_pool_create_destroy();
    test_connection_pool_get_connection();
    test_connection_pool_return_connection();
    test_connection_pool_get_idle_count();
    test_connection_pool_get_active_count();
    
    /* Protocol tests */
    test_protocol_create_message();
    test_protocol_serialize_message();
    test_protocol_deserialize_message();
    test_protocol_create_query_message();
    test_protocol_create_result_message();
    test_protocol_create_error_message();
    
    /* Integration tests */
    test_server_client_integration();
    test_connection_pool_integration();
    test_protocol_integration();
    
    /* Print summary */
    printf("\n" YELLOW "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(YELLOW "║                      TEST SUMMARY                           ║\n" RESET);
    printf(YELLOW "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    int total = tests_passed + tests_failed;
    printf("Total Tests: %d\n", total);
    printf(GREEN "Passed: %d\n" RESET, tests_passed);
    printf(RED "Failed: %d\n" RESET, tests_failed);
    
    if (tests_failed == 0) {
        printf(GREEN "\n✓ All tests passed!\n" RESET);
        return 0;
    } else {
        printf(RED "\n✗ Some tests failed\n" RESET);
        return 1;
    }
}
