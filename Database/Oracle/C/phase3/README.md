# 🔤 Phase 3: SQL Parser

## Goal
Parse SQL commands into an Abstract Syntax Tree (AST). This phase teaches compiler design concepts: lexical analysis, syntax analysis, and AST construction.

---

## What You'll Learn

### Core Concepts
- **Lexical Analysis** - Breaking SQL into tokens
- **Syntax Analysis** - Parsing tokens into tree structure
- **Abstract Syntax Tree** - Representing SQL commands
- **Grammar Definition** - Formal language specification
- **Error Handling** - Reporting parse errors

### Practical Skills
- Tokenization algorithms
- Recursive descent parsing
- Tree data structure construction
- Error recovery
- Grammar implementation

---

## What You'll Build

### Features
- ✅ Lexer - Tokenize SQL strings
- ✅ Parser - Parse tokens into AST
- ✅ AST Nodes - Represent SQL commands
- ✅ CREATE TABLE support
- ✅ INSERT support
- ✅ SELECT support
- ✅ DELETE support
- ✅ Error reporting

### SQL Grammar Supported
```
CREATE TABLE table_name (
    column_name TYPE,
    ...
);

INSERT INTO table_name (col1, col2, ...) 
VALUES (val1, val2, ...);

SELECT col1, col2, ... FROM table_name 
WHERE condition;

DELETE FROM table_name WHERE condition;
```

---

## File Structure

```
phase3/
├── include/
│   ├── token.h          # Token definitions
│   ├── lexer.h          # Lexer API
│   ├── ast.h            # AST node definitions
│   └── parser.h         # Parser API
├── src/
│   ├── token.c          # Token implementation
│   ├── lexer.c          # Lexer implementation
│   ├── ast.c            # AST implementation
│   ├── parser.c         # Parser implementation
│   └── main.c           # Test program
├── tests/
│   └── test_parser.c    # Unit tests
├── Makefile             # Build configuration
└── README.md            # This file
```

---

## Building and Running

### Build
```bash
cd Database/Oracle/C/phase3
make
```

### Run
```bash
./bin/parser_test
```

### Run with Debugging
```bash
# With debug symbols
make debug

# With memory checking
valgrind --leak-check=full ./bin/parser_test

# With GDB
gdb ./bin/parser_test
```

### Clean
```bash
make clean
```

---

## Key Concepts Explained

### 1. Lexical Analysis (Tokenization)
Breaking SQL into tokens:
```
Input:  "SELECT * FROM users WHERE id = 5"
Output: [SELECT] [*] [FROM] [users] [WHERE] [id] [=] [5]
```

### 2. Token Types
```c
typedef enum {
    TOKEN_SELECT,
    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_INSERT,
    TOKEN_VALUES,
    TOKEN_CREATE,
    TOKEN_TABLE,
    TOKEN_DELETE,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_EQUALS,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;
```

### 3. Syntax Analysis (Parsing)
Converting tokens to AST:
```
Tokens: [SELECT] [*] [FROM] [users]

AST:
SelectStatement
├── columns: [*]
└── from_table: users
```

### 4. Abstract Syntax Tree
Representing SQL structure:
```c
typedef struct {
    ASTNodeType type;
    union {
        SelectStatement select;
        InsertStatement insert;
        CreateTableStatement create;
        DeleteStatement delete;
    } data;
} ASTNode;
```

### 5. Recursive Descent Parsing
```
parse_select()
├── expect(SELECT)
├── parse_column_list()
├── expect(FROM)
├── parse_table_name()
├── if (WHERE) parse_where_clause()
└── return SelectStatement
```

---

## Testing

### Unit Tests
```bash
make run
```

### Manual Testing
```bash
./bin/parser_test
```

### Memory Checking
```bash
valgrind --leak-check=full ./bin/parser_test
```

### Performance Testing
```bash
# Parse 1000 SQL statements
# Measure time and memory
```

---

## Debugging Tips

### 1. Print Tokens
```c
lexer_print_tokens(lexer);
```

### 2. Print AST
```c
ast_print(ast_node);
```

### 3. Enable Debug Output
```bash
make debug
```

### 4. Use GDB
```bash
gdb ./bin/parser_test
(gdb) break parser_parse
(gdb) run
(gdb) print tokens
```

---

## Common Issues

### Issue: "Unexpected token"
**Cause**: Parser encountered unexpected token
**Solution**: 
1. Check SQL syntax
2. Add debug output
3. Verify token sequence

### Issue: "Memory leak"
**Cause**: AST nodes not freed
**Solution**:
1. Use valgrind to find leak
2. Check ast_free() calls
3. Verify cleanup

### Issue: "Segmentation fault"
**Cause**: Null pointer or invalid memory
**Solution**:
1. Use GDB to find line
2. Check pointer initialization
3. Verify bounds

---

## Performance Expectations

### Time Complexity
- **Lexing**: O(n) where n = input length
- **Parsing**: O(n) for most SQL
- **AST Construction**: O(n)

### Space Complexity
- **Tokens**: O(n)
- **AST**: O(n)

### Expected Performance
- **Lexing**: ~1 MB/sec
- **Parsing**: ~100 KB/sec
- **AST Construction**: ~100 KB/sec

---

## Next Steps

After completing Phase 3:
1. ✅ Understand lexical analysis
2. ✅ Understand syntax analysis
3. ✅ Understand AST construction
4. ✅ Move to Phase 4: Buffer Pool

---

## Learning Resources

### Compiler Design
- "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- "Crafting Interpreters"
- Wikipedia: Lexical analysis, Syntax analysis

### SQL Grammar
- SQL standard documentation
- PostgreSQL grammar
- SQLite grammar

### Debugging
- `man gdb` - GNU Debugger
- `man valgrind` - Memory checker

---

## Success Criteria

- [ ] Code compiles without warnings
- [ ] All tests pass
- [ ] No memory leaks (valgrind clean)
- [ ] Lexer tokenizes correctly
- [ ] Parser builds valid AST
- [ ] All SQL commands parsed
- [ ] Error messages helpful
- [ ] Code is well-commented

---

**Happy Learning! 📚**
