#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer/lexer.h"
#include "parser/parser.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);
    
    return content;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <go_file>\n", argv[0]);
        return 1;
    }
    
    printf("=== Go Compiler ===\n");
    printf("Compiling: %s\n\n", argv[1]);
    
    // Read source file
    char* source = read_file(argv[1]);
    if (!source) {
        return 1;
    }
    
    // Lexical analysis
    printf("Phase 1: Lexical Analysis\n");
    Lexer* lexer = lexer_create(source);
    Token** tokens = NULL;
    int token_count = 0;
    int token_capacity = 0;
    
    Token* token;
    while ((token = lexer_next_token(lexer)) && token->type != TOKEN_EOF) {
        if (token_count >= token_capacity) {
            token_capacity = token_capacity == 0 ? 64 : token_capacity * 2;
            tokens = (Token**)realloc(tokens, token_capacity * sizeof(Token*));
        }
        tokens[token_count++] = token;
    }
    tokens[token_count++] = token;  // Add EOF token
    
    printf("  Found %d tokens\n\n", token_count);
    
    // Parse
    printf("Phase 2: Syntax Analysis\n");
    Parser* parser = parser_create(tokens, token_count);
    ASTNode* ast = parse_program(parser);
    
    printf("AST:\n");
    print_ast(ast, 0);
    printf("\n");
    
    // Cleanup
    lexer_destroy(lexer);
    parser_destroy(parser);
    free_ast(ast);
    
    for (int i = 0; i < token_count; i++) {
        free_token(tokens[i]);
    }
    free(tokens);
    free(source);
    
    printf("Compilation completed!\n");
    return 0;
}