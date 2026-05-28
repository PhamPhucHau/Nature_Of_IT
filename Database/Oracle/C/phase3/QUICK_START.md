# 🚀 Phase 3: SQL Parser - Quick Start (5 Minutes)

## What You'll Do

Build and run a SQL parser that converts SQL strings into an Abstract Syntax Tree (AST).

## Prerequisites

- GCC compiler
- Make build tool
- Basic C knowledge

## Step 1: Build the Project (1 minute)

```bash
cd Database/Oracle/C/phase3
make
```

Expected output:
```
Compiling src/token.c...
Compiling src/lexer.c...
Compiling src/ast.c...
Compiling src/parser.c...
Compiling src/main.c...
Linking bin/parser_test...
Build complete: bin/parser_test
```

## Step 2: Run the Tests (1 minute)

```bash
make run
```

Expected output:
```
╔════════════════════════════════════════════════════════════╗
║         SQL Parser Test Suite - Phase 3                    ║
╚════════════════════════════════════════════════════════════╝

=== Lexer: Keywords ===
✓ PASS: Lexer created
✓ PASS: Tokenization successful
✓ PASS: Token count is 9 (8 keywords + EOF)
✓ PASS: First token exists
✓ PASS: First token is SELECT

... (more tests) ...

╔════════════════════════════════════════════════════════════╗
║                      TEST SUMMARY                           ║
╚════════════════════════════════════════════════════════════╝
Total Tests: 19
Passed: 19
Failed: 0

✓ All tests passed!
```

## Step 3: Understand the Components (2 minutes)

### Token (token.h/token.c)
- Represents a single token from the lexer
- Contains token type, value, line, and column

### Lexer (lexer.h/lexer.c)
- Converts SQL strings into tokens
- Recognizes keywords, identifiers, numbers, strings, operators

### AST (ast.h/ast.c)
- Represents parsed SQL as a tree structure
- Supports SELECT, INSERT, CREATE TABLE, DELETE statements

### Parser (parser.h/parser.c)
- Converts tokens into AST using recursive descent parsing
- Validates SQL syntax and reports errors

## Step 4: Try It Yourself (1 minute)

Edit `src/main.c` and add a new test:

```c
void test_my_query() {
    print_test_header("My Test");
    
    Lexer *lexer = lexer_create("SELECT id FROM users WHERE id = 10");
    assert_success(lexer_tokenize(lexer), "Tokenization successful");
    
    Parser *parser = parser_create(lexer->tokens, lexer->token_count);
    ASTNode *ast = parser_parse(parser);
    assert_not_null(ast, "AST created");
    
    // Print the AST
    ast_print(ast, 0);
    
    ast_destroy(ast);
    parser_destroy(parser);
    lexer_destroy(lexer);
}
```

Then add to main():
```c
test_my_query();
```

Rebuild and run:
```bash
make run
```

## Debugging Tips

### Print Tokens
```bash
# Add to your test:
lexer_print_tokens(lexer);
```

### Print AST
```bash
# Add to your test:
ast_print(ast, 0);
```

### Run with GDB
```bash
make gdb
(gdb) break parser_parse
(gdb) run
(gdb) print parser->current
```

### Check for Memory Leaks
```bash
make valgrind
```

## Common Issues

### Issue: "Unexpected token"
**Solution**: Check your SQL syntax. The parser expects valid SQL.

### Issue: "Segmentation fault"
**Solution**: Use GDB to find the line. Check pointer initialization.

### Issue: "Memory leak"
**Solution**: Run valgrind to find the leak. Check ast_destroy() calls.

## Next Steps

1. ✅ Understand lexical analysis
2. ✅ Understand syntax analysis
3. ✅ Understand AST construction
4. 📚 Read DEBUGGING_GUIDE.md for advanced debugging
5. 📚 Read COMPLETION_SUMMARY.md for implementation details
6. 🎯 Move to Phase 4: Buffer Pool

## Key Files

- `include/token.h` - Token definitions
- `include/lexer.h` - Lexer API
- `include/ast.h` - AST node definitions
- `include/parser.h` - Parser API
- `src/token.c` - Token implementation
- `src/lexer.c` - Lexer implementation
- `src/ast.c` - AST implementation
- `src/parser.c` - Parser implementation
- `src/main.c` - Test program

## Learning Resources

- **Compiler Design**: "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- **Parsing**: "Crafting Interpreters" (https://craftinginterpreters.com/)
- **SQL Grammar**: PostgreSQL documentation

---

**Happy Learning! 📚**
