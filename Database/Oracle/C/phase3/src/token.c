/*
 * token.c - Token Implementation
 * 
 * Implements token creation, destruction, and utility functions.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * TOKEN CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * token_create - Create a new token
 * 
 * Allocates memory for a new Token structure and initializes it with
 * the provided values.
 */
Token* token_create(TokenType type, const char *value, uint32_t line, uint32_t column) {
    /* Allocate memory for token */
    Token *token = (Token *)malloc(sizeof(Token));
    if (!token) {
        fprintf(stderr, "ERROR: Failed to allocate memory for token\n");
        return NULL;
    }
    
    /* Initialize token fields */
    token->type = type;
    token->line = line;
    token->column = column;
    
    /* Copy value string (with bounds checking) */
    if (value) {
        strncpy(token->value, value, MAX_TOKEN_LENGTH - 1);
        token->value[MAX_TOKEN_LENGTH - 1] = '\0';
    } else {
        token->value[0] = '\0';
    }
    
    return token;
}

/*
 * token_destroy - Free token memory
 * 
 * Frees the memory allocated for a token.
 */
void token_destroy(Token *token) {
    if (token) {
        free(token);
    }
}

/* ============================================================================
 * TOKEN UTILITY FUNCTIONS
 * ============================================================================ */

/*
 * token_type_to_string - Convert token type to string
 * 
 * Returns a human-readable string representation of a token type.
 * Useful for debugging and error messages.
 */
const char* token_type_to_string(TokenType type) {
    switch (type) {
        /* Keywords */
        case TOKEN_SELECT:      return "SELECT";
        case TOKEN_FROM:        return "FROM";
        case TOKEN_WHERE:       return "WHERE";
        case TOKEN_INSERT:      return "INSERT";
        case TOKEN_INTO:        return "INTO";
        case TOKEN_VALUES:      return "VALUES";
        case TOKEN_CREATE:      return "CREATE";
        case TOKEN_TABLE:       return "TABLE";
        case TOKEN_DELETE:      return "DELETE";
        case TOKEN_AND:         return "AND";
        case TOKEN_OR:          return "OR";
        case TOKEN_NOT:         return "NOT";
        case TOKEN_INT:         return "INT";
        case TOKEN_VARCHAR:     return "VARCHAR";
        case TOKEN_FLOAT:       return "FLOAT";
        case TOKEN_CHAR:        return "CHAR";
        
        /* Identifiers and Literals */
        case TOKEN_IDENTIFIER:  return "IDENTIFIER";
        case TOKEN_NUMBER:      return "NUMBER";
        case TOKEN_STRING:      return "STRING";
        
        /* Operators */
        case TOKEN_EQUALS:      return "=";
        case TOKEN_NOT_EQUALS:  return "!=";
        case TOKEN_LESS_THAN:   return "<";
        case TOKEN_GREATER_THAN: return ">";
        case TOKEN_LESS_EQUAL:  return "<=";
        case TOKEN_GREATER_EQUAL: return ">=";
        case TOKEN_PLUS:        return "+";
        case TOKEN_MINUS:       return "-";
        case TOKEN_MULTIPLY:    return "*";
        case TOKEN_DIVIDE:      return "/";
        
        /* Punctuation */
        case TOKEN_LPAREN:      return "(";
        case TOKEN_RPAREN:      return ")";
        case TOKEN_COMMA:       return ",";
        case TOKEN_SEMICOLON:   return ";";
        case TOKEN_ASTERISK:    return "*";
        
        /* Special */
        case TOKEN_EOF:         return "EOF";
        case TOKEN_ERROR:       return "ERROR";
        
        default:                return "UNKNOWN";
    }
}

/*
 * token_print - Print token information
 * 
 * Prints token information in a human-readable format.
 * Format: [TYPE] "value" at line X, column Y
 */
void token_print(Token *token) {
    if (!token) {
        printf("NULL token\n");
        return;
    }
    
    printf("[%s] \"%s\" at line %u, column %u\n",
           token_type_to_string(token->type),
           token->value,
           token->line,
           token->column);
}

/*
 * token_is_keyword - Check if token is a keyword
 * 
 * Determines if a token type is a SQL keyword.
 */
int token_is_keyword(TokenType type) {
    switch (type) {
        case TOKEN_SELECT:
        case TOKEN_FROM:
        case TOKEN_WHERE:
        case TOKEN_INSERT:
        case TOKEN_INTO:
        case TOKEN_VALUES:
        case TOKEN_CREATE:
        case TOKEN_TABLE:
        case TOKEN_DELETE:
        case TOKEN_AND:
        case TOKEN_OR:
        case TOKEN_NOT:
        case TOKEN_INT:
        case TOKEN_VARCHAR:
        case TOKEN_FLOAT:
        case TOKEN_CHAR:
            return 1;
        default:
            return 0;
    }
}

/*
 * token_is_operator - Check if token is an operator
 * 
 * Determines if a token type is an operator.
 */
int token_is_operator(TokenType type) {
    switch (type) {
        case TOKEN_EQUALS:
        case TOKEN_NOT_EQUALS:
        case TOKEN_LESS_THAN:
        case TOKEN_GREATER_THAN:
        case TOKEN_LESS_EQUAL:
        case TOKEN_GREATER_EQUAL:
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_MULTIPLY:
        case TOKEN_DIVIDE:
            return 1;
        default:
            return 0;
    }
}
