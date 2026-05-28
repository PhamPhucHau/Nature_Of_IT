/*
 * lexer.c - Lexical Analyzer Implementation
 * 
 * Implements the lexer that converts SQL strings into tokens.
 * 
 * Algorithm:
 * 1. Skip whitespace
 * 2. Identify token type based on current character
 * 3. Extract token value
 * 4. Create token and add to array
 * 5. Repeat until EOF
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "lexer.h"
#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* ============================================================================
 * LEXER CREATION AND DESTRUCTION
 * ============================================================================ */

/*
 * lexer_create - Create a new lexer
 * 
 * Allocates and initializes a new Lexer structure for the given input.
 */
Lexer* lexer_create(const char *input) {
    if (!input) {
        fprintf(stderr, "ERROR: Input string is NULL\n");
        return NULL;
    }
    
    /* Allocate memory for lexer */
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (!lexer) {
        fprintf(stderr, "ERROR: Failed to allocate memory for lexer\n");
        return NULL;
    }
    
    /* Allocate memory for tokens array */
    lexer->tokens = (Token *)malloc(sizeof(Token) * MAX_TOKENS);
    if (!lexer->tokens) {
        fprintf(stderr, "ERROR: Failed to allocate memory for tokens\n");
        free(lexer);
        return NULL;
    }
    
    /* Initialize lexer fields */
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->token_count = 0;
    lexer->error_message[0] = '\0';
    
    return lexer;
}

/*
 * lexer_destroy - Free lexer memory
 * 
 * Frees all memory allocated for the lexer.
 */
void lexer_destroy(Lexer *lexer) {
    if (lexer) {
        if (lexer->tokens) {
            free(lexer->tokens);
        }
        free(lexer);
    }
}

/* ============================================================================
 * CHARACTER OPERATIONS
 * ============================================================================ */

/*
 * lexer_peek_char - Peek at current character without advancing
 * 
 * Returns the current character without advancing the position.
 */
char lexer_peek_char(Lexer *lexer) {
    if (!lexer || lexer->position >= strlen(lexer->input)) {
        return '\0';
    }
    return lexer->input[lexer->position];
}

/*
 * lexer_advance_char - Advance to next character
 * 
 * Advances the lexer position to the next character and updates
 * line/column tracking.
 */
char lexer_advance_char(Lexer *lexer) {
    if (!lexer || lexer->position >= strlen(lexer->input)) {
        return '\0';
    }
    
    char c = lexer->input[lexer->position];
    lexer->position++;
    
    /* Update line and column tracking */
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    return c;
}

/*
 * lexer_skip_whitespace - Skip whitespace characters
 * 
 * Advances the lexer position past whitespace and comments.
 */
void lexer_skip_whitespace(Lexer *lexer) {
    if (!lexer) return;
    
    while (lexer->position < strlen(lexer->input)) {
        char c = lexer->input[lexer->position];
        
        /* Skip whitespace */
        if (isspace(c)) {
            lexer_advance_char(lexer);
        }
        /* Skip comments (-- to end of line) */
        else if (c == '-' && lexer->position + 1 < strlen(lexer->input) &&
                 lexer->input[lexer->position + 1] == '-') {
            /* Skip until end of line */
            while (lexer->position < strlen(lexer->input) &&
                   lexer->input[lexer->position] != '\n') {
                lexer_advance_char(lexer);
            }
        } else {
            break;
        }
    }
}

/* ============================================================================
 * KEYWORD RECOGNITION
 * ============================================================================ */

/*
 * lexer_is_keyword - Check if string is a keyword
 * 
 * Determines if a string is a SQL keyword and returns the corresponding
 * TokenType if it is.
 */
int lexer_is_keyword(const char *word, TokenType *token_type) {
    if (!word || !token_type) return 0;
    
    /* Convert to uppercase for comparison */
    char upper_word[MAX_TOKEN_LENGTH];
    strncpy(upper_word, word, MAX_TOKEN_LENGTH - 1);
    upper_word[MAX_TOKEN_LENGTH - 1] = '\0';
    
    for (int i = 0; upper_word[i]; i++) {
        upper_word[i] = toupper(upper_word[i]);
    }
    
    /* Check against keywords */
    if (strcmp(upper_word, "SELECT") == 0) {
        *token_type = TOKEN_SELECT;
        return 1;
    } else if (strcmp(upper_word, "FROM") == 0) {
        *token_type = TOKEN_FROM;
        return 1;
    } else if (strcmp(upper_word, "WHERE") == 0) {
        *token_type = TOKEN_WHERE;
        return 1;
    } else if (strcmp(upper_word, "INSERT") == 0) {
        *token_type = TOKEN_INSERT;
        return 1;
    } else if (strcmp(upper_word, "INTO") == 0) {
        *token_type = TOKEN_INTO;
        return 1;
    } else if (strcmp(upper_word, "VALUES") == 0) {
        *token_type = TOKEN_VALUES;
        return 1;
    } else if (strcmp(upper_word, "CREATE") == 0) {
        *token_type = TOKEN_CREATE;
        return 1;
    } else if (strcmp(upper_word, "TABLE") == 0) {
        *token_type = TOKEN_TABLE;
        return 1;
    } else if (strcmp(upper_word, "DELETE") == 0) {
        *token_type = TOKEN_DELETE;
        return 1;
    } else if (strcmp(upper_word, "AND") == 0) {
        *token_type = TOKEN_AND;
        return 1;
    } else if (strcmp(upper_word, "OR") == 0) {
        *token_type = TOKEN_OR;
        return 1;
    } else if (strcmp(upper_word, "NOT") == 0) {
        *token_type = TOKEN_NOT;
        return 1;
    } else if (strcmp(upper_word, "INT") == 0) {
        *token_type = TOKEN_INT;
        return 1;
    } else if (strcmp(upper_word, "VARCHAR") == 0) {
        *token_type = TOKEN_VARCHAR;
        return 1;
    } else if (strcmp(upper_word, "FLOAT") == 0) {
        *token_type = TOKEN_FLOAT;
        return 1;
    } else if (strcmp(upper_word, "CHAR") == 0) {
        *token_type = TOKEN_CHAR;
        return 1;
    }
    
    return 0;
}

/* ============================================================================
 * TOKEN EXTRACTION
 * ============================================================================ */

/*
 * Extract identifier or keyword
 * 
 * Identifiers start with letter or underscore, followed by letters, digits, or underscores.
 */
static int lexer_extract_identifier(Lexer *lexer, char *buffer, size_t buffer_size) {
    size_t i = 0;
    
    while (i < buffer_size - 1 && lexer->position < strlen(lexer->input)) {
        char c = lexer->input[lexer->position];
        
        if (isalnum(c) || c == '_') {
            buffer[i++] = c;
            lexer_advance_char(lexer);
        } else {
            break;
        }
    }
    
    buffer[i] = '\0';
    return i > 0 ? 0 : -1;
}

/*
 * Extract number (integer or float)
 * 
 * Numbers can be integers or floating-point values.
 */
static int lexer_extract_number(Lexer *lexer, char *buffer, size_t buffer_size) {
    size_t i = 0;
    int has_dot = 0;
    
    while (i < buffer_size - 1 && lexer->position < strlen(lexer->input)) {
        char c = lexer->input[lexer->position];
        
        if (isdigit(c)) {
            buffer[i++] = c;
            lexer_advance_char(lexer);
        } else if (c == '.' && !has_dot) {
            has_dot = 1;
            buffer[i++] = c;
            lexer_advance_char(lexer);
        } else {
            break;
        }
    }
    
    buffer[i] = '\0';
    return i > 0 ? 0 : -1;
}

/*
 * Extract string literal
 * 
 * Strings are enclosed in single quotes.
 */
static int lexer_extract_string(Lexer *lexer, char *buffer, size_t buffer_size) {
    size_t i = 0;
    
    /* Skip opening quote */
    lexer_advance_char(lexer);
    
    while (i < buffer_size - 1 && lexer->position < strlen(lexer->input)) {
        char c = lexer->input[lexer->position];
        
        if (c == '\'') {
            /* Check for escaped quote */
            if (lexer->position + 1 < strlen(lexer->input) &&
                lexer->input[lexer->position + 1] == '\'') {
                buffer[i++] = '\'';
                lexer_advance_char(lexer);
                lexer_advance_char(lexer);
            } else {
                /* End of string */
                lexer_advance_char(lexer);
                break;
            }
        } else {
            buffer[i++] = c;
            lexer_advance_char(lexer);
        }
    }
    
    buffer[i] = '\0';
    return 0;
}

/* ============================================================================
 * MAIN TOKENIZATION
 * ============================================================================ */

/*
 * lexer_tokenize - Tokenize the input
 * 
 * Performs lexical analysis on the input string, producing a sequence
 * of tokens.
 */
int lexer_tokenize(Lexer *lexer) {
    if (!lexer) {
        return -1;
    }
    
    uint32_t start_line, start_column;
    char buffer[MAX_TOKEN_LENGTH];
    
    while (lexer->position < strlen(lexer->input)) {
        /* Skip whitespace */
        lexer_skip_whitespace(lexer);
        
        if (lexer->position >= strlen(lexer->input)) {
            break;
        }
        
        /* Check if we've exceeded max tokens */
        if (lexer->token_count >= MAX_TOKENS) {
            snprintf(lexer->error_message, sizeof(lexer->error_message),
                     "Too many tokens (max %d)", MAX_TOKENS);
            return -1;
        }
        
        start_line = lexer->line;
        start_column = lexer->column;
        char c = lexer_peek_char(lexer);
        
        /* Identifier or keyword */
        if (isalpha(c) || c == '_') {
            if (lexer_extract_identifier(lexer, buffer, sizeof(buffer)) == 0) {
                TokenType type;
                if (lexer_is_keyword(buffer, &type)) {
                    lexer->tokens[lexer->token_count++] = 
                        *token_create(type, buffer, start_line, start_column);
                } else {
                    lexer->tokens[lexer->token_count++] = 
                        *token_create(TOKEN_IDENTIFIER, buffer, start_line, start_column);
                }
            }
        }
        /* Number */
        else if (isdigit(c)) {
            if (lexer_extract_number(lexer, buffer, sizeof(buffer)) == 0) {
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_NUMBER, buffer, start_line, start_column);
            }
        }
        /* String literal */
        else if (c == '\'') {
            if (lexer_extract_string(lexer, buffer, sizeof(buffer)) == 0) {
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_STRING, buffer, start_line, start_column);
            }
        }
        /* Operators and punctuation */
        else if (c == '=') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_EQUALS, "=", start_line, start_column);
        }
        else if (c == '<') {
            lexer_advance_char(lexer);
            if (lexer_peek_char(lexer) == '=') {
                lexer_advance_char(lexer);
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_LESS_EQUAL, "<=", start_line, start_column);
            } else if (lexer_peek_char(lexer) == '>') {
                lexer_advance_char(lexer);
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_NOT_EQUALS, "<>", start_line, start_column);
            } else {
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_LESS_THAN, "<", start_line, start_column);
            }
        }
        else if (c == '>') {
            lexer_advance_char(lexer);
            if (lexer_peek_char(lexer) == '=') {
                lexer_advance_char(lexer);
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_GREATER_EQUAL, ">=", start_line, start_column);
            } else {
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_GREATER_THAN, ">", start_line, start_column);
            }
        }
        else if (c == '!') {
            lexer_advance_char(lexer);
            if (lexer_peek_char(lexer) == '=') {
                lexer_advance_char(lexer);
                lexer->tokens[lexer->token_count++] = 
                    *token_create(TOKEN_NOT_EQUALS, "!=", start_line, start_column);
            } else {
                snprintf(lexer->error_message, sizeof(lexer->error_message),
                         "Unexpected character '!' at line %u, column %u",
                         start_line, start_column);
                return -1;
            }
        }
        else if (c == '+') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_PLUS, "+", start_line, start_column);
        }
        else if (c == '-') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_MINUS, "-", start_line, start_column);
        }
        else if (c == '*') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_ASTERISK, "*", start_line, start_column);
        }
        else if (c == '/') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_DIVIDE, "/", start_line, start_column);
        }
        else if (c == '(') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_LPAREN, "(", start_line, start_column);
        }
        else if (c == ')') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_RPAREN, ")", start_line, start_column);
        }
        else if (c == ',') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_COMMA, ",", start_line, start_column);
        }
        else if (c == ';') {
            lexer_advance_char(lexer);
            lexer->tokens[lexer->token_count++] = 
                *token_create(TOKEN_SEMICOLON, ";", start_line, start_column);
        }
        else {
            snprintf(lexer->error_message, sizeof(lexer->error_message),
                     "Unexpected character '%c' at line %u, column %u",
                     c, start_line, start_column);
            return -1;
        }
    }
    
    /* Add EOF token */
    lexer->tokens[lexer->token_count++] = 
        *token_create(TOKEN_EOF, "", lexer->line, lexer->column);
    
    return 0;
}

/* ============================================================================
 * LEXER QUERY FUNCTIONS
 * ============================================================================ */

/*
 * lexer_get_token - Get token at index
 * 
 * Retrieves a token from the token array by index.
 */
Token* lexer_get_token(Lexer *lexer, uint32_t index) {
    if (!lexer || index >= lexer->token_count) {
        return NULL;
    }
    return &lexer->tokens[index];
}

/*
 * lexer_get_token_count - Get number of tokens
 * 
 * Returns the number of tokens produced by tokenization.
 */
uint32_t lexer_get_token_count(Lexer *lexer) {
    if (!lexer) return 0;
    return lexer->token_count;
}

/*
 * lexer_print_tokens - Print all tokens
 * 
 * Prints all tokens in a human-readable format.
 */
void lexer_print_tokens(Lexer *lexer) {
    if (!lexer) return;
    
    printf("=== TOKENS ===\n");
    for (uint32_t i = 0; i < lexer->token_count; i++) {
        printf("%3u: ", i);
        token_print(&lexer->tokens[i]);
    }
    printf("==============\n");
}

/*
 * lexer_get_error - Get error message
 * 
 * Returns the error message if tokenization failed.
 */
const char* lexer_get_error(Lexer *lexer) {
    if (!lexer) return "NULL lexer";
    return lexer->error_message;
}
