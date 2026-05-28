# 🐛 Phase 3: SQL Parser - Debugging Guide

## Overview

This guide covers debugging techniques for the SQL parser, including lexer, parser, and AST issues.

## Table of Contents

1. [Common Issues](#common-issues)
2. [Debugging Tools](#debugging-tools)
3. [Debugging Techniques](#debugging-techniques)
4. [Memory Debugging](#memory-debugging)
5. [Performance Debugging](#performance-debugging)

---

## Common Issues

### Issue 1: Tokenization Fails

**Symptoms:**
- Lexer returns -1
- Error message: "Unexpected character"

**Debugging Steps:**

1. Print the input string:
```c
printf("Input: %s\n", input);
```

2. Print tokens as they're created:
```c
lexer_print_tokens(lexer);
```

3. Check for unsupported characters:
```c
// Add to lexer.c
if (c == '@' || c == '#' || c == '$') {
    printf("DEBUG: Unsupported character '%c' at position %zu\n", c, lexer->position);
}
```

4. Verify keyword recognition:
```c
TokenType type;
if (lexer_is_keyword("SELECT", &type)) {
    printf("DEBUG: SELECT recognized as %d\n", type);
}
```

**Solution:**
- Check SQL syntax for invalid characters
- Add support for new characters in lexer if needed

---

### Issue 2: Parser Fails

**Symptoms:**
- Parser returns NULL
- Error message: "Expected X but got Y"

**Debugging Steps:**

1. Print current token:
```c
Token *token = parser_current_token(parser);
if (token) {
    printf("DEBUG: Current token: %s at line %u, column %u\n",
           token_type_to_string(token->type),
           token->line,
           token->column);
}
```

2. Print parser state:
```c
printf("DEBUG: Parser position: %u/%u\n", parser->current, parser->token_count);
```

3. Add debug output to parser functions:
```c
ASTNode* parser_parse_select(Parser *parser) {
    printf("DEBUG: Entering parser_parse_select\n");
    // ... parsing code ...
    printf("DEBUG: Exiting parser_parse_select\n");
    return ast;
}
```

4. Check error message:
```c
if (!ast) {
    printf("Parse error: %s\n", parser_get_error(parser));
}
```

**Solution:**
- Verify SQL syntax matches expected grammar
- Check token sequence with lexer_print_tokens()
- Add debug output to identify where parsing fails

---

### Issue 3: AST is NULL or Invalid

**Symptoms:**
- AST pointer is NULL
- AST structure is corrupted
- Segmentation fault when accessing AST

**Debugging Steps:**

1. Check AST creation:
```c
ASTNode *ast = parser_parse(parser);
if (!ast) {
    printf("DEBUG: AST is NULL\n");
    printf("Error: %s\n", parser_get_error(parser));
    return;
}
```

2. Print AST structure:
```c
ast_print(ast, 0);
```

3. Check node types:
```c
printf("DEBUG: AST node type: %s\n", ast_get_type_string(ast->type));
```

4. Verify node data:
```c
if (ast->type == AST_SELECT) {
    SelectStatement *select = &ast->data.select;
    printf("DEBUG: Table: %s\n", select->table_name);
    printf("DEBUG: Columns: %u\n", select->column_count);
}
```

**Solution:**
- Check parser error message
- Verify SQL syntax
- Use ast_print() to visualize structure

---

### Issue 4: Memory Leaks

**Symptoms:**
- Valgrind reports memory leaks
- Program crashes after multiple runs
- Memory usage increases over time

**Debugging Steps:**

1. Run with valgrind:
```bash
make valgrind
```

2. Look for "definitely lost" blocks:
```
definitely lost: 1,024 bytes in 10 blocks
```

3. Check for missing ast_destroy() calls:
```c
// WRONG: Memory leak
ASTNode *ast = parser_parse(parser);
// ... use ast ...
// Missing: ast_destroy(ast);

// CORRECT: No leak
ASTNode *ast = parser_parse(parser);
// ... use ast ...
ast_destroy(ast);
```

4. Check for missing lexer_destroy() calls:
```c
Lexer *lexer = lexer_create(input);
lexer_tokenize(lexer);
// ... use lexer ...
lexer_destroy(lexer);  // Don't forget!
```

5. Check for missing parser_destroy() calls:
```c
Parser *parser = parser_create(tokens, count);
ASTNode *ast = parser_parse(parser);
// ... use ast ...
ast_destroy(ast);
parser_destroy(parser);  // Don't forget!
```

**Solution:**
- Always call destroy functions
- Use valgrind to find leaks
- Check all code paths for cleanup

---

## Debugging Tools

### 1. GDB (GNU Debugger)

**Start GDB:**
```bash
make gdb
```

**Common Commands:**

```gdb
# Set breakpoint
(gdb) break parser_parse
(gdb) break lexer.c:100

# Run program
(gdb) run

# Step through code
(gdb) step
(gdb) next

# Print variables
(gdb) print parser->current
(gdb) print token->type
(gdb) print *ast

# Print array elements
(gdb) print lexer->tokens[0]
(gdb) print lexer->tokens[0]@5

# Continue execution
(gdb) continue

# Backtrace (see call stack)
(gdb) backtrace

# Quit
(gdb) quit
```

**Example Debugging Session:**

```bash
$ make gdb
...
(gdb) break parser_parse
Breakpoint 1 at 0x...
(gdb) run
Starting program: ./bin/parser_test
...
Breakpoint 1, parser_parse (parser=0x...) at src/parser.c:...
(gdb) print parser->current
$1 = 0
(gdb) print parser->tokens[0]
$2 = {type = TOKEN_SELECT, value = "SELECT", line = 1, column = 1}
(gdb) next
(gdb) print parser->current
$3 = 1
(gdb) continue
```

### 2. Valgrind (Memory Checker)

**Run with valgrind:**
```bash
make valgrind
```

**Output Example:**

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap alloc: 1,024 bytes in 10 blocks
==12345==   total heap freed: 1,024 bytes in 10 blocks
==12345==   total heap alloc'd: 1,024 bytes in 10 blocks
==12345== All heap blocks were freed -- no leaks are possible
```

**Leak Detection:**

```
==12345== 256 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x...: malloc (vg_replace_malloc.c:...)
==12345==    by 0x...: lexer_create (lexer.c:...)
==12345==    by 0x...: test_lexer_keywords (main.c:...)
==12345==    by 0x...: main (main.c:...)
```

### 3. Printf Debugging

**Add debug output:**

```c
#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)
#endif

// Usage:
DEBUG_PRINT("Token type: %d", token->type);
DEBUG_PRINT("Parser position: %u", parser->current);
```

**Compile with debug:**
```bash
make debug
```

---

## Debugging Techniques

### 1. Trace Execution

Add debug output at function entry/exit:

```c
ASTNode* parser_parse_select(Parser *parser) {
    DEBUG_PRINT("ENTER: parser_parse_select");
    
    if (parser_expect(parser, TOKEN_SELECT) != 0) {
        DEBUG_PRINT("EXIT: parser_parse_select (error)");
        return NULL;
    }
    
    // ... parsing code ...
    
    DEBUG_PRINT("EXIT: parser_parse_select (success)");
    return ast;
}
```

### 2. Validate State

Check invariants:

```c
// After tokenization
assert(lexer->token_count > 0);
assert(lexer->tokens[lexer->token_count - 1].type == TOKEN_EOF);

// After parsing
assert(ast != NULL);
assert(ast->type >= AST_SELECT && ast->type <= AST_LITERAL);
```

### 3. Print Intermediate Results

```c
// After lexing
printf("Tokens produced: %u\n", lexer_get_token_count(lexer));
lexer_print_tokens(lexer);

// After parsing
printf("AST structure:\n");
ast_print(ast, 0);
```

### 4. Test Incrementally

Test each component separately:

```c
// Test 1: Lexer only
Lexer *lexer = lexer_create("SELECT * FROM users");
lexer_tokenize(lexer);
lexer_print_tokens(lexer);
lexer_destroy(lexer);

// Test 2: Parser only (after lexer works)
Lexer *lexer = lexer_create("SELECT * FROM users");
lexer_tokenize(lexer);
Parser *parser = parser_create(lexer->tokens, lexer->token_count);
ASTNode *ast = parser_parse(parser);
ast_print(ast, 0);
ast_destroy(ast);
parser_destroy(parser);
lexer_destroy(lexer);
```

---

## Memory Debugging

### 1. Check for Leaks

```bash
# Full leak check
valgrind --leak-check=full --show-leak-kinds=all ./bin/parser_test

# Track allocations
valgrind --track-origins=yes ./bin/parser_test

# Generate report
valgrind --log-file=valgrind.log ./bin/parser_test
cat valgrind.log
```

### 2. Find Double-Free

```bash
valgrind --tool=memcheck ./bin/parser_test
```

Look for:
```
Invalid free() / delete / delete[] / realloc()
```

### 3. Find Buffer Overflows

```bash
valgrind --tool=memcheck --read-var-info=yes ./bin/parser_test
```

Look for:
```
Invalid write of size X
```

---

## Performance Debugging

### 1. Measure Parsing Time

```c
#include <time.h>

clock_t start = clock();
ASTNode *ast = parser_parse(parser);
clock_t end = clock();

double time_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000;
printf("Parse time: %.2f ms\n", time_ms);
```

### 2. Profile with Valgrind

```bash
valgrind --tool=callgrind ./bin/parser_test
kcachegrind callgrind.out.*
```

### 3. Check Token Count

```c
printf("Tokens: %u\n", lexer_get_token_count(lexer));
```

---

## Debugging Checklist

- [ ] Verify SQL syntax is correct
- [ ] Check lexer output with lexer_print_tokens()
- [ ] Check parser error message with parser_get_error()
- [ ] Print AST with ast_print()
- [ ] Run with valgrind for memory leaks
- [ ] Run with GDB to step through code
- [ ] Check all destroy() calls are made
- [ ] Verify token types match expected values
- [ ] Check array bounds and counts
- [ ] Test with simple queries first

---

## Quick Reference

| Issue | Command | Solution |
|-------|---------|----------|
| Tokenization fails | `lexer_print_tokens()` | Check SQL syntax |
| Parser fails | `parser_get_error()` | Check token sequence |
| AST is NULL | `ast_print()` | Check parser error |
| Memory leak | `make valgrind` | Add destroy() calls |
| Segfault | `make gdb` | Check pointers |
| Performance | `clock()` | Profile code |

---

**Happy Debugging! 🐛**
