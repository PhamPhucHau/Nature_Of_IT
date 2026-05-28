/*
 * main.c - SQL Parser Test Program
 * 
 * Comprehensive test suite for SQL parser implementation.
 * Tests lexer, parser, and AST construction.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * TEST UTILITIES
 * ============================================================================ */

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define RESET   "\x1b[0m"

static int tests_passed = 0;
static int tests_failed = 0;

void print_test_header(const char *test_name) {
    printf("\n" YELLOW "=== %s ===" RESET "\n", test_name);
}

void assert_success(int result, const char *test_name) {
    if (result == 0) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s\n", test_name);
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

void assert_null(void *ptr, const char *test_name) {
    if (ptr == NULL) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (expected NULL)\n", test_name);
        tests_failed++;
    }
}

void assert_equal_int(int actual, int expected, const char *test_name) {
    if (actual == expected) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s (expected %d, got %d)\n", 
               test_name, expected, actual);
        tests_failed++;
    }
}

void assert_equal_str(const char *actual, const char *expected, const char *test_name) {
    if (actual && expected && strcmp(actual, expected) == 0) {
        printf(GREEN "✓ PASS" RESET ": %s\n", test_name);
        tests_passed++;
    } else {
        printf(RED "✗ FAIL" RESET ": %s\n", test_name);
        if (actual && expected) {
            printf("  Expected: %s\n", expected);
            printf("  Actual:   %s\n", actual);
        }
        tests_failed++;
    }
}

/* ============================================================================
 * LEXER TESTS
 * ============================================================================ */

void test_lexer_keywords() {
    print_test_header("Lexer: Keywords");
    
    Lexer *lexer = lexer_create("SELECT FROM WHERE INSERT VALUES CREATE TABLE DELETE");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 9, "Token count is 9 (8 keywords + EOF)");
    
    Token *token = lexer_get_token(lexer, 0);
    assert_not_null(token, "First token exists");
    assert_equal_int(token->type, TOKEN_SELECT, "First token is SELECT");
    
    lexer_destroy(lexer);
}

void test_lexer_identifiers() {
    print_test_header("Lexer: Identifiers");
    
    Lexer *lexer = lexer_create("users id name email");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 5, "Token count is 5 (4 identifiers + EOF)");
    
    Token *token = lexer_get_token(lexer, 0);
    assert_equal_int(token->type, TOKEN_IDENTIFIER, "First token is IDENTIFIER");
    assert_equal_str(token->value, "users", "First identifier is 'users'");
    
    lexer_destroy(lexer);
}

void test_lexer_numbers() {
    print_test_header("Lexer: Numbers");
    
    Lexer *lexer = lexer_create("123 45.67 0 999");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 5, "Token count is 5 (4 numbers + EOF)");
    
    Token *token = lexer_get_token(lexer, 0);
    assert_equal_int(token->type, TOKEN_NUMBER, "First token is NUMBER");
    assert_equal_str(token->value, "123", "First number is '123'");
    
    lexer_destroy(lexer);
}

void test_lexer_strings() {
    print_test_header("Lexer: Strings");
    
    Lexer *lexer = lexer_create("'hello' 'world' 'test'");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 4, "Token count is 4 (3 strings + EOF)");
    
    Token *token = lexer_get_token(lexer, 0);
    assert_equal_int(token->type, TOKEN_STRING, "First token is STRING");
    assert_equal_str(token->value, "hello", "First string is 'hello'");
    
    lexer_destroy(lexer);
}

void test_lexer_operators() {
    print_test_header("Lexer: Operators");
    
    Lexer *lexer = lexer_create("= < > <= >= !=");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 7, "Token count is 7 (6 operators + EOF)");
    
    Token *token = lexer_get_token(lexer, 0);
    assert_equal_int(token->type, TOKEN_EQUALS, "First token is EQUALS");
    
    lexer_destroy(lexer);
}

void test_lexer_punctuation() {
    print_test_header("Lexer: Punctuation");
    
    Lexer *lexer = lexer_create("( ) , ; *");
    assert_not_null(lexer, "Lexer created");
    
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    uint32_t count = lexer_get_token_count(lexer);
    assert_equal_int(count, 6, "Token count is 6 (5 punctuation + EOF)");
    
    lexer_destroy(lexer);
}

/* ============================================================================
 * PARSER TESTS
 * ============================================================================ */

void test_parser_select_simple() {
    print_test_header("Parser: Simple SELECT");
    
    Lexer *lexer = lexer_create("SELECT * FROM users");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    assert_not_null(parser, "Parser created");
    
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    assert_equal_int(ast->type, AST_SELECT, "AST is SELECT statement");
    
    SelectStatement *select = &ast->data.select;
    assert_equal_str(select->table_name, "users", "Table name is 'users'");
    assert_equal_int(select->column_count, 1, "Column count is 1");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_parser_select_columns() {
    print_test_header("Parser: SELECT with columns");
    
    Lexer *lexer = lexer_create("SELECT id, name, email FROM users");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    
    SelectStatement *select = &ast->data.select;
    assert_equal_int(select->column_count, 3, "Column count is 3");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_parser_select_where() {
    print_test_header("Parser: SELECT with WHERE");
    
    Lexer *lexer = lexer_create("SELECT * FROM users WHERE id = 5");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    
    SelectStatement *select = &ast->data.select;
    assert_not_null(select->where_clause, "WHERE clause exists");
    assert_equal_int(select->where_clause->type, AST_CONDITION, "WHERE is condition");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_parser_insert() {
    print_test_header("Parser: INSERT");
    
    Lexer *lexer = lexer_create("INSERT INTO users (id, name) VALUES (1, 'Alice')");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    assert_equal_int(ast->type, AST_INSERT, "AST is INSERT statement");
    
    InsertStatement *insert = &ast->data.insert;
    assert_equal_str(insert->table_name, "users", "Table name is 'users'");
    assert_equal_int(insert->column_count, 2, "Column count is 2");
    assert_equal_int(insert->value_count, 2, "Value count is 2");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_parser_create_table() {
    print_test_header("Parser: CREATE TABLE");
    
    Lexer *lexer = lexer_create("CREATE TABLE users (id INT, name VARCHAR(50))");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    assert_equal_int(ast->type, AST_CREATE_TABLE, "AST is CREATE TABLE statement");
    
    CreateTableStatement *create = &ast->data.create;
    assert_equal_str(create->table_name, "users", "Table name is 'users'");
    assert_equal_int(create->column_count, 2, "Column count is 2");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_parser_delete() {
    print_test_header("Parser: DELETE");
    
    Lexer *lexer = lexer_create("DELETE FROM users WHERE id = 5");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    assert_equal_int(ast->type, AST_DELETE, "AST is DELETE statement");
    
    DeleteStatement *delete = &ast->data.delete;
    assert_equal_str(delete->table_name, "users", "Table name is 'users'");
    assert_not_null(delete->where_clause, "WHERE clause exists");
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}

/* ============================================================================
 * ERROR HANDLING TESTS
 * ============================================================================ */

void test_error_invalid_token() {
    print_test_header("Error Handling: Invalid token");
    
    Lexer *lexer = lexer_create("SELECT @ FROM users");
    int result = lexer_tokenize(lexer);
    assert_equal_int(result, -1, "Tokenization fails");
    
    const char *error = lexer_get_error(lexer);
    assert_not_null(error, "Error message exists");
    
    lexer_destroy(lexer);
}

void test_error_missing_keyword() {
    print_test_header("Error Handling: Missing keyword");
    
    Lexer *lexer = lexer_create("SELECT * users");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_null(ast, "AST is NULL (parse error)");
    
    const char *error = parser_get_error(parser);
    assert_not_null(error, "Error message exists");
    
    parser_destroy(parser);
    lexer_destroy(lexer);
}

void test_error_missing_table_name() {
    print_test_header("Error Handling: Missing table name");
    
    Lexer *lexer = lexer_create("SELECT * FROM");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_null(ast, "AST is NULL (parse error)");
    
    parser_destroy(parser);
    lexer_destroy(lexer);
}

/* ============================================================================
 * MAIN TEST RUNNER
 * ============================================================================ */

int main(void) {
    printf(BLUE "\n╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(BLUE "║         SQL Parser Test Suite - Phase 3                    ║\n" RESET);
    printf(BLUE "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
    /* Lexer tests */
    test_lexer_keywords();
    test_lexer_identifiers();
    test_lexer_numbers();
    test_lexer_strings();
    test_lexer_operators();
    test_lexer_punctuation();
    
    /* Parser tests */
    test_parser_select_simple();
    test_parser_select_columns();
    test_parser_select_where();
    test_parser_insert();
    test_parser_create_table();
    test_parser_delete();
    
    /* Error handling tests */
    test_error_invalid_token();
    test_error_missing_keyword();
    test_error_missing_table_name();
    
    /* Print summary */
    printf("\n" BLUE "╔════════════════════════════════════════════════════════════╗\n" RESET);
    printf(BLUE "║                      TEST SUMMARY                           ║\n" RESET);
    printf(BLUE "╚════════════════════════════════════════════════════════════╝\n" RESET);
    
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
