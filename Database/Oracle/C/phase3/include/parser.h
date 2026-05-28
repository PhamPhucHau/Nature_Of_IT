/*
 * parser.h - SQL Parser Header
 * 
 * Defines the API for the SQL parser that converts tokens into an
 * Abstract Syntax Tree (AST).
 * 
 * Key Concepts:
 * - Syntax Analysis: Checking if tokens follow SQL grammar
 * - Recursive Descent Parsing: Top-down parsing technique
 * - Error Recovery: Reporting helpful error messages
 * - AST Construction: Building tree representation
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef PARSER_H
#define PARSER_H

#include "token.h"   
#include "ast.h"
#include <stddef.h>

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Parser - SQL parser structure
 * 
 * The Parser structure maintains the state of the parsing process.
 * It tracks:
 * - The tokens to parse
 * - Current position in token stream
 * - Error information
 * - The resulting AST
 * 
 * Fields:
 *   - tokens: Array of tokens from lexer
 *   - token_count: Number of tokens
 *   - current: Current token index
 *   - error_message: Error message if parsing failed
 *   - ast: Resulting AST (NULL if parsing failed)
 * 
 * Example:
 *   Parser *parser = parser_create(tokens, token_count);
 *   ASTNode *ast = parser_parse(parser);
 *   if (ast) {
 *       ast_print(ast, 0);
 *   } else {
 *       printf("Parse error: %s\n", parser->error_message);
 *   }
 */
typedef struct {
    Token *tokens;                  /* Array of tokens */
    uint32_t token_count;           /* Number of tokens */
    uint32_t current;               /* Current token index */
    char error_message[256];        /* Error message if parsing failed */
    ASTNode *ast;                   /* Resulting AST */
} Parser;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * parser_create - Create a new parser
 * 
 * Allocates and initializes a new Parser structure with the given tokens.
 * The parser is ready to parse after creation.
 * 
 * Parameters:
 *   tokens: Array of tokens from lexer
 *   token_count: Number of tokens
 * 
 * Returns:
 *   Pointer to new Parser, or NULL on error
 * 
 * Example:
 *   Parser *parser = parser_create(tokens, token_count);
 *   if (!parser) {
 *       fprintf(stderr, "Failed to create parser\n");
 *       return 1;
 *   }
 */
Parser* parser_create(Token *tokens, uint32_t token_count);

/*
 * parser_destroy - Free parser memory
 * 
 * Frees all memory allocated for the parser.
 * Note: The AST is NOT freed by this function. Use ast_destroy() separately.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   parser_destroy(parser);
 */
void parser_destroy(Parser *parser);

/*
 * parser_parse - Parse tokens into AST
 * 
 * Performs syntax analysis on the tokens, producing an Abstract Syntax Tree.
 * This is the main entry point for the parser.
 * 
 * Algorithm:
 * 1. Determine statement type (SELECT, INSERT, CREATE, DELETE)
 * 2. Call appropriate parsing function
 * 3. Build AST nodes
 * 4. Return AST or error
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to AST root node on success, NULL on error
 * 
 * Example:
 *   ASTNode *ast = parser_parse(parser);
 *   if (!ast) {
 *       fprintf(stderr, "Parse error: %s\n", parser->error_message);
 *       return 1;
 *   }
 */
ASTNode* parser_parse(Parser *parser);

/*
 * parser_get_error - Get error message
 * 
 * Returns the error message if parsing failed.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Error message string
 * 
 * Example:
 *   if (!parser_parse(parser)) {
 *       printf("Error: %s\n", parser_get_error(parser));
 *   }
 */
const char* parser_get_error(Parser *parser);

/*
 * parser_parse_select - Parse SELECT statement
 * 
 * Parses a SELECT statement and returns an AST node.
 * 
 * Grammar:
 *   SELECT column_list FROM table_name [WHERE condition]
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to SELECT AST node, or NULL on error
 * 
 * Example:
 *   ASTNode *select = parser_parse_select(parser);
 */
ASTNode* parser_parse_select(Parser *parser);

/*
 * parser_parse_insert - Parse INSERT statement
 * 
 * Parses an INSERT statement and returns an AST node.
 * 
 * Grammar:
 *   INSERT INTO table_name (column_list) VALUES (value_list)
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to INSERT AST node, or NULL on error
 */
ASTNode* parser_parse_insert(Parser *parser);

/*
 * parser_parse_create_table - Parse CREATE TABLE statement
 * 
 * Parses a CREATE TABLE statement and returns an AST node.
 * 
 * Grammar:
 *   CREATE TABLE table_name (column_def, ...)
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to CREATE TABLE AST node, or NULL on error
 */
ASTNode* parser_parse_create_table(Parser *parser);

/*
 * parser_parse_delete - Parse DELETE statement
 * 
 * Parses a DELETE statement and returns an AST node.
 * 
 * Grammar:
 *   DELETE FROM table_name [WHERE condition]
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to DELETE AST node, or NULL on error
 */
ASTNode* parser_parse_delete(Parser *parser);

/*
 * parser_parse_column_list - Parse column list
 * 
 * Parses a comma-separated list of columns.
 * 
 * Grammar:
 *   column_name [, column_name]*
 * 
 * Parameters:
 *   parser: Pointer to Parser
 *   columns: Output array of column nodes
 *   column_count: Output count of columns
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int parser_parse_column_list(Parser *parser, ASTNode ***columns,
                             uint32_t *column_count);

/*
 * parser_parse_value_list - Parse value list
 * 
 * Parses a comma-separated list of values.
 * 
 * Grammar:
 *   value [, value]*
 * 
 * Parameters:
 *   parser: Pointer to Parser
 *   values: Output array of value nodes
 *   value_count: Output count of values
 * 
 * Returns:
 *   0 on success, -1 on error
 */
int parser_parse_value_list(Parser *parser, ASTNode ***values,
                            uint32_t *value_count);

/*
 * parser_parse_where_clause - Parse WHERE clause
 * 
 * Parses a WHERE clause with conditions.
 * 
 * Grammar:
 *   WHERE condition [AND condition]*
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to condition AST node, or NULL on error
 */
ASTNode* parser_parse_where_clause(Parser *parser);

/*
 * parser_parse_condition - Parse comparison condition
 * 
 * Parses a single comparison condition.
 * 
 * Grammar:
 *   identifier operator value
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to condition AST node, or NULL on error
 */
ASTNode* parser_parse_condition(Parser *parser);

/*
 * parser_parse_column_def - Parse column definition
 * 
 * Parses a column definition in CREATE TABLE.
 * 
 * Grammar:
 *   column_name TYPE [size]
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to column definition AST node, or NULL on error
 */
ASTNode* parser_parse_column_def(Parser *parser);

/*
 * parser_expect - Expect a specific token type
 * 
 * Checks if the current token matches the expected type.
 * If not, sets an error message and returns -1.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 *   expected: Expected token type
 * 
 * Returns:
 *   0 if match, -1 if mismatch
 * 
 * Example:
 *   if (parser_expect(parser, TOKEN_SELECT) != 0) {
 *       printf("Expected SELECT\n");
 *       return NULL;
 *   }
 */
int parser_expect(Parser *parser, TokenType expected);

/*
 * parser_current_token - Get current token
 * 
 * Returns the current token without advancing.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to current Token, or NULL if at EOF
 */
Token* parser_current_token(Parser *parser);

/*
 * parser_advance - Advance to next token
 * 
 * Advances the parser to the next token.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   Pointer to new current Token, or NULL if at EOF
 */
Token* parser_advance(Parser *parser);

/*
 * parser_is_at_end - Check if at end of tokens
 * 
 * Determines if the parser has reached the end of the token stream.
 * 
 * Parameters:
 *   parser: Pointer to Parser
 * 
 * Returns:
 *   1 if at end, 0 if not
 */
int parser_is_at_end(Parser *parser);

#endif // PARSER_H
