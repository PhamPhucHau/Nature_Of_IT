/*
 * parser.c - SQL Parser Implementation
 * 
 * Implements the recursive descent parser that converts tokens into an AST.
 * 
 * Algorithm:
 * 1. Determine statement type (SELECT, INSERT, CREATE, DELETE)
 * 2. Call appropriate parsing function
 * 3. Build AST nodes recursively
 * 4. Return AST or error
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "parser.h"
#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * PARSER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * parser_create - Create a new parser
 * 
 * Allocates and initializes a new Parser structure with the given tokens.
 */
Parser* parser_create(Token *tokens, uint32_t token_count) {
    if (!tokens || token_count == 0) {
        fprintf(stderr, "ERROR: Invalid tokens\n");
        return NULL;
    }
    
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "ERROR: Failed to allocate memory for parser\n");
        return NULL;
    }
    
    parser->tokens = tokens;
    parser->token_count = token_count;
    parser->current = 0;
    parser->error_message[0] = '\0';
    parser->ast = NULL;
    
    return parser;
}

/*
 * parser_destroy - Free parser memory
 * 
 * Frees all memory allocated for the parser.
 */
void parser_destroy(Parser *parser) {
    if (parser) {
        free(parser);
    }
}

/* ============================================================================
 * TOKEN NAVIGATION
 * ============================================================================ */

/*
 * parser_current_token - Get current token
 * 
 * Returns the current token without advancing.
 */
Token* parser_current_token(Parser *parser) {
    if (!parser || parser->current >= parser->token_count) {
        return NULL;
    }
    return &parser->tokens[parser->current];
}

/*
 * parser_advance - Advance to next token
 * 
 * Advances the parser to the next token.
 */
Token* parser_advance(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser->current < parser->token_count) {
        parser->current++;
    }
    
    return parser_current_token(parser);
}

/*
 * parser_is_at_end - Check if at end of tokens
 * 
 * Determines if the parser has reached the end of the token stream.
 */
int parser_is_at_end(Parser *parser) {
    if (!parser) return 1;
    
    Token *token = parser_current_token(parser);
    return token == NULL || token->type == TOKEN_EOF;
}

/*
 * parser_expect - Expect a specific token type
 * 
 * Checks if the current token matches the expected type.
 */
int parser_expect(Parser *parser, TokenType expected) {
    if (!parser) return -1;
    
    Token *token = parser_current_token(parser);
    if (!token) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Unexpected end of input, expected %s",
                 token_type_to_string(expected));
        return -1;
    }
    
    if (token->type != expected) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected %s but got %s at line %u, column %u",
                 token_type_to_string(expected),
                 token_type_to_string(token->type),
                 token->line,
                 token->column);
        return -1;
    }
    
    parser_advance(parser);
    return 0;
}

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================ */

/*
 * Convert token type to data type
 */
static DataType token_to_data_type(TokenType type) {
    switch (type) {
        case TOKEN_INT:     return DATA_TYPE_INT;
        case TOKEN_VARCHAR: return DATA_TYPE_VARCHAR;
        case TOKEN_FLOAT:   return DATA_TYPE_FLOAT;
        case TOKEN_CHAR:    return DATA_TYPE_CHAR;
        default:            return DATA_TYPE_INT;
    }
}

/*
 * Convert token type to operator type
 */
static OperatorType token_to_operator(TokenType type) {
    switch (type) {
        case TOKEN_EQUALS:      return OPERATOR_EQUALS;
        case TOKEN_NOT_EQUALS:  return OPERATOR_NOT_EQUALS;
        case TOKEN_LESS_THAN:   return OPERATOR_LESS_THAN;
        case TOKEN_GREATER_THAN: return OPERATOR_GREATER_THAN;
        case TOKEN_LESS_EQUAL:  return OPERATOR_LESS_EQUAL;
        case TOKEN_GREATER_EQUAL: return OPERATOR_GREATER_EQUAL;
        case TOKEN_AND:         return OPERATOR_AND;
        case TOKEN_OR:          return OPERATOR_OR;
        default:                return OPERATOR_EQUALS;
    }
}

/* ============================================================================
 * PARSING FUNCTIONS
 * ============================================================================ */

/*
 * parser_parse_column_list - Parse column list
 * 
 * Parses a comma-separated list of columns.
 */
int parser_parse_column_list(Parser *parser, ASTNode ***columns,
                             uint32_t *column_count) {
    if (!parser || !columns || !column_count) return -1;
    
    *columns = (ASTNode **)malloc(sizeof(ASTNode *) * 100);
    if (!*columns) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Failed to allocate memory for column list");
        return -1;
    }
    
    *column_count = 0;
    
    while (1) {
        Token *token = parser_current_token(parser);
        if (!token || token->type == TOKEN_EOF) {
            snprintf(parser->error_message, sizeof(parser->error_message),
                     "Unexpected end of input in column list");
            return -1;
        }
        
        /* Parse column name or * */
        if (token->type == TOKEN_ASTERISK) {
            (*columns)[(*column_count)++] = ast_create_identifier("*");
            parser_advance(parser);
        } else if (token->type == TOKEN_IDENTIFIER) {
            (*columns)[(*column_count)++] = ast_create_identifier(token->value);
            parser_advance(parser);
        } else {
            snprintf(parser->error_message, sizeof(parser->error_message),
                     "Expected column name or * at line %u, column %u",
                     token->line, token->column);
            return -1;
        }
        
        /* Check for comma (more columns) or end of list */
        token = parser_current_token(parser);
        if (token && token->type == TOKEN_COMMA) {
            parser_advance(parser);
        } else {
            break;
        }
    }
    
    return 0;
}

/*
 * parser_parse_value_list - Parse value list
 * 
 * Parses a comma-separated list of values.
 */
int parser_parse_value_list(Parser *parser, ASTNode ***values,
                            uint32_t *value_count) {
    if (!parser || !values || !value_count) return -1;
    
    *values = (ASTNode **)malloc(sizeof(ASTNode *) * 100);
    if (!*values) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Failed to allocate memory for value list");
        return -1;
    }
    
    *value_count = 0;
    
    while (1) {
        Token *token = parser_current_token(parser);
        if (!token || token->type == TOKEN_EOF) {
            snprintf(parser->error_message, sizeof(parser->error_message),
                     "Unexpected end of input in value list");
            return -1;
        }
        
        /* Parse value (number or string) */
        if (token->type == TOKEN_NUMBER) {
            (*values)[(*value_count)++] = ast_create_literal(token->value, 0);
            parser_advance(parser);
        } else if (token->type == TOKEN_STRING) {
            (*values)[(*value_count)++] = ast_create_literal(token->value, 1);
            parser_advance(parser);
        } else {
            snprintf(parser->error_message, sizeof(parser->error_message),
                     "Expected value (number or string) at line %u, column %u",
                     token->line, token->column);
            return -1;
        }
        
        /* Check for comma (more values) or end of list */
        token = parser_current_token(parser);
        if (token && token->type == TOKEN_COMMA) {
            parser_advance(parser);
        } else {
            break;
        }
    }
    
    return 0;
}

/*
 * parser_parse_condition - Parse comparison condition
 * 
 * Parses a single comparison condition.
 */
ASTNode* parser_parse_condition(Parser *parser) {
    if (!parser) return NULL;
    
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected identifier in condition");
        return NULL;
    }
    
    ASTNode *left = ast_create_identifier(token->value);
    parser_advance(parser);
    
    /* Get operator */
    token = parser_current_token(parser);
    if (!token || !token_is_operator(token->type)) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected operator in condition");
        ast_destroy(left);
        return NULL;
    }
    
    OperatorType op = token_to_operator(token->type);
    parser_advance(parser);
    
    /* Get right operand */
    token = parser_current_token(parser);
    if (!token) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected value in condition");
        ast_destroy(left);
        return NULL;
    }
    
    ASTNode *right = NULL;
    if (token->type == TOKEN_NUMBER) {
        right = ast_create_literal(token->value, 0);
        parser_advance(parser);
    } else if (token->type == TOKEN_STRING) {
        right = ast_create_literal(token->value, 1);
        parser_advance(parser);
    } else if (token->type == TOKEN_IDENTIFIER) {
        right = ast_create_identifier(token->value);
        parser_advance(parser);
    } else {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected value in condition");
        ast_destroy(left);
        return NULL;
    }
    
    return ast_create_condition(left, op, right);
}

/*
 * parser_parse_where_clause - Parse WHERE clause
 * 
 * Parses a WHERE clause with conditions.
 */
ASTNode* parser_parse_where_clause(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser_expect(parser, TOKEN_WHERE) != 0) {
        return NULL;
    }
    
    ASTNode *condition = parser_parse_condition(parser);
    if (!condition) {
        return NULL;
    }
    
    /* TODO: Handle AND/OR for multiple conditions */
    
    return condition;
}

/*
 * parser_parse_column_def - Parse column definition
 * 
 * Parses a column definition in CREATE TABLE.
 */
ASTNode* parser_parse_column_def(Parser *parser) {
    if (!parser) return NULL;
    
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected column name");
        return NULL;
    }
    
    char col_name[256];
    strcpy(col_name, token->value);
    parser_advance(parser);
    
    /* Get data type */
    token = parser_current_token(parser);
    if (!token) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected data type");
        return NULL;
    }
    
    DataType type = token_to_data_type(token->type);
    parser_advance(parser);
    
    /* Check for size (for VARCHAR) */
    uint32_t size = 0;
    token = parser_current_token(parser);
    if (token && token->type == TOKEN_LPAREN) {
        parser_advance(parser);
        token = parser_current_token(parser);
        if (token && token->type == TOKEN_NUMBER) {
            size = atoi(token->value);
            parser_advance(parser);
        }
        if (parser_expect(parser, TOKEN_RPAREN) != 0) {
            return NULL;
        }
    }
    
    return ast_create_column_def(col_name, type, size);
}

/*
 * parser_parse_select - Parse SELECT statement
 * 
 * Parses a SELECT statement and returns an AST node.
 */
ASTNode* parser_parse_select(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser_expect(parser, TOKEN_SELECT) != 0) {
        return NULL;
    }
    
    /* Parse column list */
    ASTNode **columns = NULL;
    uint32_t column_count = 0;
    if (parser_parse_column_list(parser, &columns, &column_count) != 0) {
        return NULL;
    }
    
    /* Expect FROM */
    if (parser_expect(parser, TOKEN_FROM) != 0) {
        return NULL;
    }
    
    /* Get table name */
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected table name");
        return NULL;
    }
    
    char table_name[256];
    strcpy(table_name, token->value);
    parser_advance(parser);
    
    /* Check for WHERE clause */
    ASTNode *where_clause = NULL;
    token = parser_current_token(parser);
    if (token && token->type == TOKEN_WHERE) {
        where_clause = parser_parse_where_clause(parser);
        if (!where_clause) {
            return NULL;
        }
    }
    
    return ast_create_select(table_name, columns, column_count, where_clause);
}

/*
 * parser_parse_insert - Parse INSERT statement
 * 
 * Parses an INSERT statement and returns an AST node.
 */
ASTNode* parser_parse_insert(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser_expect(parser, TOKEN_INSERT) != 0) {
        return NULL;
    }
    
    if (parser_expect(parser, TOKEN_INTO) != 0) {
        return NULL;
    }
    
    /* Get table name */
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected table name");
        return NULL;
    }
    
    char table_name[256];
    strcpy(table_name, token->value);
    parser_advance(parser);
    
    /* Parse column list */
    if (parser_expect(parser, TOKEN_LPAREN) != 0) {
        return NULL;
    }
    
    ASTNode **columns = NULL;
    uint32_t column_count = 0;
    if (parser_parse_column_list(parser, &columns, &column_count) != 0) {
        return NULL;
    }
    
    if (parser_expect(parser, TOKEN_RPAREN) != 0) {
        return NULL;
    }
    
    /* Expect VALUES */
    if (parser_expect(parser, TOKEN_VALUES) != 0) {
        return NULL;
    }
    
    /* Parse value list */
    if (parser_expect(parser, TOKEN_LPAREN) != 0) {
        return NULL;
    }
    
    ASTNode **values = NULL;
    uint32_t value_count = 0;
    if (parser_parse_value_list(parser, &values, &value_count) != 0) {
        return NULL;
    }
    
    if (parser_expect(parser, TOKEN_RPAREN) != 0) {
        return NULL;
    }
    
    return ast_create_insert(table_name, columns, column_count, values, value_count);
}

/*
 * parser_parse_create_table - Parse CREATE TABLE statement
 * 
 * Parses a CREATE TABLE statement and returns an AST node.
 */
ASTNode* parser_parse_create_table(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser_expect(parser, TOKEN_CREATE) != 0) {
        return NULL;
    }
    
    if (parser_expect(parser, TOKEN_TABLE) != 0) {
        return NULL;
    }
    
    /* Get table name */
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected table name");
        return NULL;
    }
    
    char table_name[256];
    strcpy(table_name, token->value);
    parser_advance(parser);
    
    /* Parse column definitions */
    if (parser_expect(parser, TOKEN_LPAREN) != 0) {
        return NULL;
    }
    
    ASTNode **columns = (ASTNode **)malloc(sizeof(ASTNode *) * 100);
    if (!columns) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Failed to allocate memory for columns");
        return NULL;
    }
    
    uint32_t column_count = 0;
    while (1) {
        ASTNode *col_def = parser_parse_column_def(parser);
        if (!col_def) {
            return NULL;
        }
        columns[column_count++] = col_def;
        
        token = parser_current_token(parser);
        if (token && token->type == TOKEN_COMMA) {
            parser_advance(parser);
        } else {
            break;
        }
    }
    
    if (parser_expect(parser, TOKEN_RPAREN) != 0) {
        return NULL;
    }
    
    return ast_create_create_table(table_name, columns, column_count);
}

/*
 * parser_parse_delete - Parse DELETE statement
 * 
 * Parses a DELETE statement and returns an AST node.
 */
ASTNode* parser_parse_delete(Parser *parser) {
    if (!parser) return NULL;
    
    if (parser_expect(parser, TOKEN_DELETE) != 0) {
        return NULL;
    }
    
    if (parser_expect(parser, TOKEN_FROM) != 0) {
        return NULL;
    }
    
    /* Get table name */
    Token *token = parser_current_token(parser);
    if (!token || token->type != TOKEN_IDENTIFIER) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "Expected table name");
        return NULL;
    }
    
    char table_name[256];
    strcpy(table_name, token->value);
    parser_advance(parser);
    
    /* Check for WHERE clause */
    ASTNode *where_clause = NULL;
    token = parser_current_token(parser);
    if (token && token->type == TOKEN_WHERE) {
        where_clause = parser_parse_where_clause(parser);
        if (!where_clause) {
            return NULL;
        }
    }
    
    return ast_create_delete(table_name, where_clause);
}

/*
 * parser_parse - Parse tokens into AST
 * 
 * Performs syntax analysis on the tokens, producing an Abstract Syntax Tree.
 */
ASTNode* parser_parse(Parser *parser) {
    if (!parser) return NULL;
    
    Token *token = parser_current_token(parser);
    if (!token) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                 "No tokens to parse");
        return NULL;
    }
    
    /* Determine statement type and parse accordingly */
    switch (token->type) {
        case TOKEN_SELECT:
            parser->ast = parser_parse_select(parser);
            break;
        case TOKEN_INSERT:
            parser->ast = parser_parse_insert(parser);
            break;
        case TOKEN_CREATE:
            parser->ast = parser_parse_create_table(parser);
            break;
        case TOKEN_DELETE:
            parser->ast = parser_parse_delete(parser);
            break;
        default:
            snprintf(parser->error_message, sizeof(parser->error_message),
                     "Unexpected token at start of statement: %s",
                     token_type_to_string(token->type));
            return NULL;
    }
    
    return parser->ast;
}

/*
 * parser_get_error - Get error message
 * 
 * Returns the error message if parsing failed.
 */
const char* parser_get_error(Parser *parser) {
    if (!parser) return "NULL parser";
    return parser->error_message;
}
