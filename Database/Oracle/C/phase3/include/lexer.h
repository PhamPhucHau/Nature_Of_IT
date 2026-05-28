/*
 * lexer.h - Lexical Analyzer (Lexer) Header
 * 
 * Defines the API for the lexical analyzer that converts SQL strings
 * into a sequence of tokens.
 * 
 * Key Concepts:
 * - Lexical Analysis: Breaking input into tokens
 * - Tokenization: Converting characters to meaningful units
 * - Error Handling: Reporting invalid tokens
 * - Position Tracking: Maintaining line and column information
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include <stddef.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Maximum number of tokens in a single SQL statement */
#define MAX_TOKENS 1000

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Lexer - Lexical analyzer structure
 * 
 * The Lexer structure maintains the state of the lexical analysis process.
 * It tracks:
 * - The input string being tokenized
 * - Current position in the input
 * - Line and column tracking for error reporting
 * - Array of tokens produced
 * 
 * Fields:
 *   - input: The SQL string to tokenize
 *   - position: Current position in input
 *   - line: Current line number
 *   - column: Current column number
 *   - tokens: Array of tokens produced
 *   - token_count: Number of tokens produced
 *   - error_message: Error message if tokenization failed
 * 
 * Example:
 *   Lexer *lexer = lexer_create("SELECT * FROM users");
 *   lexer_tokenize(lexer);
 *   for (int i = 0; i < lexer->token_count; i++) {
 *       token_print(&lexer->tokens[i]);
 *   }
 */
typedef struct {
    const char *input;              /* Input SQL string */
    size_t position;                /* Current position in input */
    uint32_t line;                  /* Current line number (1-indexed) */
    uint32_t column;                /* Current column number (1-indexed) */
    Token *tokens;                  /* Array of tokens */
    uint32_t token_count;           /* Number of tokens */
    char error_message[256];        /* Error message if tokenization failed */
} Lexer;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * lexer_create - Create a new lexer
 * 
 * Allocates and initializes a new Lexer structure for the given input.
 * The lexer is ready to tokenize after creation.
 * 
 * Parameters:
 *   input: SQL string to tokenize
 * 
 * Returns:
 *   Pointer to new Lexer, or NULL on error
 * 
 * Example:
 *   Lexer *lexer = lexer_create("SELECT * FROM users");
 *   if (!lexer) {
 *       fprintf(stderr, "Failed to create lexer\n");
 *       return 1;
 *   }
 */
Lexer* lexer_create(const char *input);

/*
 * lexer_destroy - Free lexer memory
 * 
 * Frees all memory allocated for the lexer, including tokens.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   lexer_destroy(lexer);
 */
void lexer_destroy(Lexer *lexer);

/*
 * lexer_tokenize - Tokenize the input
 * 
 * Performs lexical analysis on the input string, producing a sequence
 * of tokens. This is the main entry point for the lexer.
 * 
 * Algorithm:
 * 1. Skip whitespace
 * 2. Identify token type (keyword, identifier, number, operator, etc.)
 * 3. Extract token value
 * 4. Create token and add to array
 * 5. Repeat until EOF
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   0 on success, -1 on error
 * 
 * Example:
 *   if (lexer_tokenize(lexer) != 0) {
 *       fprintf(stderr, "Tokenization error: %s\n", lexer->error_message);
 *       return 1;
 *   }
 */
int lexer_tokenize(Lexer *lexer);

/*
 * lexer_get_token - Get token at index
 * 
 * Retrieves a token from the token array by index.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 *   index: Token index
 * 
 * Returns:
 *   Pointer to Token, or NULL if index out of bounds
 * 
 * Example:
 *   Token *token = lexer_get_token(lexer, 0);
 *   if (token) {
 *       token_print(token);
 *   }
 */
Token* lexer_get_token(Lexer *lexer, uint32_t index);

/*
 * lexer_get_token_count - Get number of tokens
 * 
 * Returns the number of tokens produced by tokenization.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   Number of tokens
 * 
 * Example:
 *   printf("Total tokens: %u\n", lexer_get_token_count(lexer));
 */
uint32_t lexer_get_token_count(Lexer *lexer);

/*
 * lexer_print_tokens - Print all tokens
 * 
 * Prints all tokens in a human-readable format.
 * Useful for debugging the lexer.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   void
 * 
 * Example:
 *   lexer_print_tokens(lexer);
 */
void lexer_print_tokens(Lexer *lexer);

/*
 * lexer_get_error - Get error message
 * 
 * Returns the error message if tokenization failed.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   Error message string
 * 
 * Example:
 *   if (lexer_tokenize(lexer) != 0) {
 *       printf("Error: %s\n", lexer_get_error(lexer));
 *   }
 */
const char* lexer_get_error(Lexer *lexer);

/*
 * lexer_is_keyword - Check if string is a keyword
 * 
 * Determines if a string is a SQL keyword and returns the corresponding
 * TokenType if it is.
 * 
 * Parameters:
 *   word: String to check
 *   token_type: Output parameter for token type
 * 
 * Returns:
 *   1 if keyword, 0 if not
 * 
 * Example:
 *   TokenType type;
 *   if (lexer_is_keyword("SELECT", &type)) {
 *       printf("Keyword found: %s\n", token_type_to_string(type));
 *   }
 */
int lexer_is_keyword(const char *word, TokenType *token_type);

/*
 * lexer_skip_whitespace - Skip whitespace characters
 * 
 * Advances the lexer position past whitespace and comments.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   void
 * 
 * Note: This is an internal function but exposed for testing
 */
void lexer_skip_whitespace(Lexer *lexer);

/*
 * lexer_peek_char - Peek at current character without advancing
 * 
 * Returns the current character without advancing the position.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   Current character, or '\0' if at EOF
 * 
 * Example:
 *   char c = lexer_peek_char(lexer);
 *   if (c == 'S') {
 *       // Might be SELECT
 *   }
 */
char lexer_peek_char(Lexer *lexer);

/*
 * lexer_advance_char - Advance to next character
 * 
 * Advances the lexer position to the next character and updates
 * line/column tracking.
 * 
 * Parameters:
 *   lexer: Pointer to Lexer
 * 
 * Returns:
 *   Current character before advancing, or '\0' if at EOF
 * 
 * Example:
 *   char c = lexer_advance_char(lexer);
 */
char lexer_advance_char(Lexer *lexer);

#endif // LEXER_H
