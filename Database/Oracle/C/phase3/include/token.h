/*
 * token.h - Token Definitions for SQL Lexer
 * 
 * Defines token types and the Token structure used by the lexer.
 * Tokens are the basic units produced by lexical analysis.
 * 
 * Key Concepts:
 * - TokenType: Enumeration of all possible token types
 * - Token: Structure containing token information
 * - Token attributes: Type, value, line number, column
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <stdint.h>
#include <stddef.h>

/* ============================================================================
 * TOKEN TYPES
 * ============================================================================
 * 
 * TokenType enumeration defines all possible token types in SQL.
 * Each token type represents a different element of the SQL language.
 * 
 * Categories:
 * - Keywords: SQL reserved words (SELECT, FROM, WHERE, etc.)
 * - Identifiers: User-defined names (table names, column names)
 * - Literals: Constant values (numbers, strings)
 * - Operators: Comparison and arithmetic operators
 * - Punctuation: Delimiters and separators
 * - Special: EOF and error tokens
 */

typedef enum {
    /* Keywords */
    TOKEN_SELECT,           /* SELECT keyword */
    TOKEN_FROM,             /* FROM keyword */
    TOKEN_WHERE,            /* WHERE keyword */
    TOKEN_INSERT,           /* INSERT keyword */
    TOKEN_INTO,             /* INTO keyword */
    TOKEN_VALUES,           /* VALUES keyword */
    TOKEN_CREATE,           /* CREATE keyword */
    TOKEN_TABLE,            /* TABLE keyword */
    TOKEN_DELETE,           /* DELETE keyword */
    TOKEN_AND,              /* AND keyword */
    TOKEN_OR,               /* OR keyword */
    TOKEN_NOT,              /* NOT keyword */
    TOKEN_INT,              /* INT type */
    TOKEN_VARCHAR,          /* VARCHAR type */
    TOKEN_FLOAT,            /* FLOAT type */
    TOKEN_CHAR,             /* CHAR type */
    
    /* Identifiers and Literals */
    TOKEN_IDENTIFIER,       /* User-defined name (table, column) */
    TOKEN_NUMBER,           /* Numeric literal (integer or float) */
    TOKEN_STRING,           /* String literal (quoted text) */
    
    /* Operators */
    TOKEN_EQUALS,           /* = operator */
    TOKEN_NOT_EQUALS,       /* != or <> operator */
    TOKEN_LESS_THAN,        /* < operator */
    TOKEN_GREATER_THAN,     /* > operator */
    TOKEN_LESS_EQUAL,       /* <= operator */
    TOKEN_GREATER_EQUAL,    /* >= operator */
    TOKEN_PLUS,             /* + operator */
    TOKEN_MINUS,            /* - operator */
    TOKEN_MULTIPLY,         /* * operator */
    TOKEN_DIVIDE,           /* / operator */
    
    /* Punctuation */
    TOKEN_LPAREN,           /* ( left parenthesis */
    TOKEN_RPAREN,           /* ) right parenthesis */
    TOKEN_COMMA,            /* , comma */
    TOKEN_SEMICOLON,        /* ; semicolon */
    TOKEN_ASTERISK,         /* * (for SELECT *) */
    
    /* Special */
    TOKEN_EOF,              /* End of input */
    TOKEN_ERROR             /* Lexical error */
} TokenType;

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum length for token value */
#define MAX_TOKEN_LENGTH 256

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Token - Represents a single token from lexical analysis
 * 
 * A token is the output of the lexer. It contains:
 * - The type of token (keyword, identifier, number, etc.)
 * - The actual text/value of the token
 * - Position information (line and column) for error reporting
 * 
 * Fields:
 *   - type: TokenType enum value
 *   - value: String representation of the token
 *   - line: Line number in source (1-indexed)
 *   - column: Column number in source (1-indexed)
 * 
 * Example:
 *   Token token;
 *   token.type = TOKEN_SELECT;
 *   token.value = "SELECT";
 *   token.line = 1;
 *   token.column = 1;
 */
typedef struct {
    TokenType type;                 /* Type of token */
    char value[MAX_TOKEN_LENGTH];   /* String value of token */
    uint32_t line;                  /* Line number (1-indexed) */
    uint32_t column;                /* Column number (1-indexed) */
} Token;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * token_create - Create a new token
 * 
 * Allocates and initializes a new Token structure with the given values.
 * 
 * Parameters:
 *   type: TokenType enum value
 *   value: String value of the token
 *   line: Line number in source
 *   column: Column number in source
 * 
 * Returns:
 *   Pointer to new Token, or NULL on error
 * 
 * Example:
 *   Token *token = token_create(TOKEN_SELECT, "SELECT", 1, 1);
 */
Token* token_create(TokenType type, const char *value, uint32_t line, uint32_t column);

/*
 * token_destroy - Free token memory
 * 
 * Frees the memory allocated for a token.
 * 
 * Parameters:
 *   token: Pointer to Token
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   token_destroy(token);
 */
void token_destroy(Token *token);

/*
 * token_type_to_string - Convert token type to string
 * 
 * Returns a human-readable string representation of a token type.
 * Useful for debugging and error messages.
 * 
 * Parameters:
 *   type: TokenType enum value
 * 
 * Returns:
 *   String representation of token type
 * 
 * Example:
 *   printf("Token type: %s\n", token_type_to_string(TOKEN_SELECT));
 *   // Output: Token type: SELECT
 */
const char* token_type_to_string(TokenType type);

/*
 * token_print - Print token information
 * 
 * Prints token information in a human-readable format.
 * Useful for debugging the lexer.
 * 
 * Parameters:
 *   token: Pointer to Token
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   token_print(token);
 *   // Output: [SELECT] "SELECT" at line 1, column 1
 */
void token_print(Token *token);

/*
 * token_is_keyword - Check if token is a keyword
 * 
 * Determines if a token type is a SQL keyword.
 * 
 * Parameters:
 *   type: TokenType enum value
 * 
 * Returns:
 *   1 if keyword, 0 if not
 * 
 * Example:
 *   if (token_is_keyword(TOKEN_SELECT)) {
 *       printf("This is a keyword\n");
 *   }
 */
int token_is_keyword(TokenType type);

/*
 * token_is_operator - Check if token is an operator
 * 
 * Determines if a token type is an operator.
 * 
 * Parameters:
 *   type: TokenType enum value
 * 
 * Returns:
 *   1 if operator, 0 if not
 * 
 * Example:
 *   if (token_is_operator(TOKEN_EQUALS)) {
 *       printf("This is an operator\n");
 *   }
 */
int token_is_operator(TokenType type);

#endif // TOKEN_H
