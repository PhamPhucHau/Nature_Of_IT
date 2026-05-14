# JavaScript Compiler Implementation Guide

## Getting Started

### Prerequisites
- C++17 or later
- GCC or Clang
- Make
- Basic C++ programming knowledge

### Project Structure
```
JavaScriptCompiler/
├── src/
│   ├── lexer/
│   │   ├── lexer.cc
│   │   ├── lexer.h
│   │   ├── token.cc
│   │   └── token.h
│   ├── parser/
│   │   ├── parser.cc
│   │   ├── parser.h
│   │   ├── ast.cc
│   │   └── ast.h
│   ├── bytecode/
│   │   ├── generator.cc
│   │   ├── generator.h
│   │   ├── interpreter.cc
│   │   └── interpreter.h
│   ├── jit/
│   │   ├── compiler.cc
│   │   └── compiler.h
│   └── main.cc
├── examples/
├── test/
└── makefile
```

---

## Phase 1: Token Definitions

Create `src/lexer/token.h`:

```cpp
#ifndef JS_COMPILER_TOKEN_H
#define JS_COMPILER_TOKEN_H

#include <string>
#include <vector>

// Token types
enum class TokenType {
    EOF_TOKEN,
    ILLEGAL,
    
    // Keywords
    VAR, LET, CONST, FUNCTION, RETURN, IF, ELSE, FOR, WHILE,
    TRUE, FALSE, NULL_TYPE, THIS,
    
    // Identifiers and literals
    IDENTIFIER,
    NUMBER,
    STRING,
    BOOLEAN,
    
    // Operators
    ASSIGN, ADD, SUB, MUL, DIV, MOD,
    EQ, NEQ, STRICT_EQ, STRICT_NEQ,
    LT, GT, LE, GE,
    AND, OR, NOT, XOR,
    LSHIFT, RSHIFT, RSHIFT_UNSIGNED,
    
    // Delimiters
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    LBRACKET, RBRACKET,
    COMMA, SEMICOLON, DOT, COLON,
    ARROW, QUESTION, COLON_COLON,
    
    // Comments
    COMMENT_LINE,
    COMMENT_BLOCK
};

// Token structure
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType t, const std::string& l, int ln, int col)
        : type(t), lexeme(l), line(ln), column(col) {}
};

// Function declarations
const char* token_type_name(TokenType type);
std::vector<Token> tokenize(const std::string& input);

#endif
```

Create `src/lexer/token.cc`:

```cpp
#include "token.h"
#include <cctype>
#include <iostream>

const char* token_type_name(TokenType type) {
    switch (type) {
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::ILLEGAL: return "ILLEGAL";
        case TokenType::VAR: return "VAR";
        case TokenType::LET: return "LET";
        case TokenType::CONST: return "CONST";
        case TokenType::FUNCTION: return "FUNCTION";
        case TokenType::RETURN: return "RETURN";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::FOR: return "FOR";
        case TokenType::WHILE: return "WHILE";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NULL_TYPE: return "NULL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::BOOLEAN: return "BOOLEAN";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::ADD: return "ADD";
        case TokenType::SUB: return "SUB";
        case TokenType::MUL: return "MUL";
        case TokenType::DIV: return "DIV";
        case TokenType::EQ: return "EQ";
        case TokenType::NEQ: return "NEQ";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::LE: return "LE";
        case TokenType::GE: return "GE";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::DOT: return "DOT";
        case TokenType::COLON: return "COLON";
        default: return "UNKNOWN";
    }
}

static bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' ||
           c == '!' || c == '<' || c == '>' || c == '&' || c == '|' ||
           c == '^' || c == '~';
}

static bool is_delimiter(char c) {
    return c == '(' || c == ')' || c == '{' || c == '}' ||
           c == '[' || c == ']' || c == ';' || c == ',' || c == '.';
}

static std::string keywords[] = {
    "var", "let", "const", "function", "return", "if", "else", "for", "while",
    "true", "false", "null", "this"
};

static TokenType keyword_types[] = {
    TokenType::VAR, TokenType::LET, TokenType::CONST, TokenType::FUNCTION,
    TokenType::RETURN, TokenType::IF, TokenType::ELSE, TokenType::FOR,
    TokenType::WHILE, TokenType::TRUE, TokenType::FALSE, TokenType::NULL_TYPE,
    TokenType::THIS
};

class Lexer {
private:
    const std::string& input;
    int position;
    int read_position;
    int line;
    int column;
    char ch;
    
    void read_char() {
        if (read_position >= input.length()) {
            ch = '\0';
        } else {
            ch = input[read_position];
        }
        position = read_position++;
        if (ch == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
    }
    
    char peek_char() {
        if (read_position >= input.length()) return '\0';
        return input[read_position];
    }
    
    void skip_whitespace() {
        while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            read_char();
        }
    }
    
    Token read_identifier() {
        int start = position;
        while (is_letter(ch) || is_digit(ch)) {
            read_char();
        }
        int length = position - start;
        std::string identifier = input.substr(start, length);
        
        // Check if it's a keyword
        for (int i = 0; i < 13; i++) {
            if (keywords[i] == identifier) {
                return Token(keyword_types[i], identifier, line, column - length);
            }
        }
        
        return Token(TokenType::IDENTIFIER, identifier, line, column - length);
    }
    
    Token read_number() {
        int start = position;
        while (is_digit(ch)) {
            read_char();
        }
        
        bool is_float = false;
        if (ch == '.') {
            is_float = true;
            read_char();
            while (is_digit(ch)) {
                read_char();
            }
        }
        
        int length = position - start;
        std::string literal = input.substr(start, length);
        
        return Token(is_float ? TokenType::NUMBER : TokenType::NUMBER, 
                    literal, line, column - length);
    }
    
    Token read_string() {
        char quote = ch;
        int start = position + 1;
        while (ch != quote && ch != '\0') {
            if (ch == '\\' && read_position < input.length()) {
                read_char();  // Skip escaped character
            }
            read_char();
        }
        
        int length = position - start;
        std::string literal = input.substr(start, length);
        
        Token token(TokenType::STRING, literal, line, column - length - 1);
        if (ch == quote) {
            read_char();  // Skip closing quote
        }
        return token;
    }
    
    Token read_operator() {
        char first = ch;
        read_char();
        
        // Check for two-character operators
        if (ch == '=') {
            read_char();
            switch (first) {
                case '=': return Token(TokenType::EQ, "==", line, column - 2);
                case '!': return Token(TokenType::NEQ, "!=", line, column - 2);
                case '<': return Token(TokenType::LE, "<=", line, column - 2);
                case '>': return Token(TokenType::GE, ">=", line, column - 2);
            }
        }
        
        if (ch == '=' && first == '=') {
            read_char();
            return Token(TokenType::STRICT_EQ, "===", line, column - 2);
        }
        
        if (ch == '!' && first == '=') {
            read_char();
            return Token(TokenType::STRICT_NEQ, "!==", line, column - 2);
        }
        
        if (ch == '&' && first == '&') {
            read_char();
            return Token(TokenType::AND, "&&", line, column - 2);
        }
        
        if (ch == '|' && first == '|') {
            read_char();
            return Token(TokenType::OR, "||", line, column - 2);
        }
        
        if (ch == '<' && first == '<') {
            read_char();
            return Token(TokenType::LSHIFT, "<<", line, column - 2);
        }
        
        if (ch == '>' && first == '>') {
            read_char();
            return Token(TokenType::RSHIFT, ">>", line, column - 2);
        }
        
        switch (first) {
            case '+': return Token(TokenType::ADD, "+", line, column - 1);
            case '-': return Token(TokenType::SUB, "-", line, column - 1);
            case '*': return Token(TokenType::MUL, "*", line, column - 1);
            case '/': return Token(TokenType::DIV, "/", line, column - 1);
            case '%': return Token(TokenType::MOD, "%", line, column - 1);
            case '=': return Token(TokenType::ASSIGN, "=", line, column - 1);
            case '!': return Token(TokenType::NOT, "!", line, column - 1);
            case '&': return Token(TokenType::AND, "&", line, column - 1);
            case '|': return Token(TokenType::OR, "|", line, column - 1);
            case '^': return Token(TokenType::XOR, "^", line, column - 1);
            case '<': return Token(TokenType::LT, "<", line, column - 1);
            case '>': return Token(TokenType::GT, ">", line, column - 1);
            case '.': return Token(TokenType::DOT, ".", line, column - 1);
            case ':': return Token(TokenType::COLON, ":", line, column - 1);
            case ';': return Token(TokenType::SEMICOLON, ";", line, column - 1);
            case '(': return Token(TokenType::LPAREN, "(", line, column - 1);
            case ')': return Token(TokenType::RPAREN, ")", line, column - 1);
            case '{': return Token(TokenType::LBRACE, "{", line, column - 1);
            case '}': return Token(TokenType::RBRACE, "}", line, column - 1);
            case '[': return Token(TokenType::LBRACKET, "[", line, column - 1);
            case ']': return Token(TokenType::RBRACKET, "]", line, column - 1);
            case ',': return Token(TokenType::COMMA, ",", line, column - 1);
        }
        
        return Token(TokenType::ILLEGAL, std::string(1, first), line, column - 1);
    }
    
    void skip_line_comment() {
        while (ch != '\n' && ch != '\0') {
            read_char();
        }
    }
    
    void skip_block_comment() {
        read_char();  // Skip *
        read_char();  // Skip /
        while (!(ch == '*' && peek_char() == '/')) {
            if (ch == '\0') break;
            read_char();
        }
        if (ch == '*') {
            read_char();  // Skip *
            read_char();  // Skip /
        }
    }

public:
    Lexer(const std::string& input) : input(input), position(0), 
        read_position(0), line(1), column(0) {
        read_char();
    }
    
    Token next_token() {
        skip_whitespace();
        
        if (is_letter(ch)) {
            return read_identifier();
        } else if (is_digit(ch)) {
            return read_number();
        } else if (ch == '"' || ch == '\'') {
            return read_string();
        } else if (ch == '/') {
            if (peek_char() == '/') {
                skip_line_comment();
                return next_token();
            } else if (peek_char() == '*') {
                skip_block_comment();
                return next_token();
            }
            return read_operator();
        } else if (is_operator_char(ch)) {
            return read_operator();
        } else if (is_delimiter(ch)) {
            return read_delimiter();
        }
        
        return Token(TokenType::ILLEGAL, std::string(1, ch), line, column);
    }
    
    Token read_delimiter() {
        char c = ch;
        read_char();
        switch (c) {
            case '(': return Token(TokenType::LPAREN, "(", line, column - 1);
            case ')': return Token(TokenType::RPAREN, ")", line, column - 1);
            case '{': return Token(TokenType::LBRACE, "{", line, column - 1);
            case '}': return Token(TokenType::RBRACE, "}", line, column - 1);
            case '[': return Token(TokenType::LBRACKET, "[", line, column - 1);
            case ']': return Token(TokenType::RBRACKET, "]", line, column - 1);
            case ';': return Token(TokenType::SEMICOLON, ";", line, column - 1);
            case ',': return Token(TokenType::COMMA, ",", line, column - 1);
            case '.': return Token(TokenType::DOT, ".", line, column - 1);
            case ':': return Token(TokenType::COLON, ":", line, column - 1);
        }
        return Token(TokenType::ILLEGAL, std::string(1, c), line, column);
    }
};

std::vector<Token> tokenize(const std::string& input) {
    std::vector<Token> tokens;
    Lexer lexer(input);
    
    Token token;
    while ((token = lexer.next_token()).type != TokenType::EOF_TOKEN) {
        tokens.push_back(token);
    }
    tokens.push_back(token);  // Add EOF token
    
    return tokens;
}
```

---

## Phase 2: AST Node Definitions

Create `src/parser/ast.h`:

```cpp
#ifndef JS_COMPILER_AST_H
#define JS_COMPILER_AST_H

#include <string>
#include <vector>
#include <memory>

// AST node types
enum class NodeType {
    PROGRAM,
    FUNCTION_DECLARATION,
    VARIABLE_DECLARATION,
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    IDENTIFIER,
    LITERAL,
    IF_STATEMENT,
    FOR_STATEMENT,
    WHILE_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION_STATEMENT,
    BLOCK_STATEMENT,
    VARIABLE_DELEGATION
};

// AST node structure
struct ASTNode {
    NodeType type;
    ASTNode* parent;
    std::vector<std::unique_ptr<ASTNode>> children;
    
    union {
        std::string identifier;
        int int_value;
        double float_value;
        std::string string_value;
        bool bool_value;
    } value;
    
    int line;
    int column;
    
    ASTNode(NodeType t, int ln = 0, int col = 0) 
        : type(t), parent(nullptr), line(ln), column(col) {}
    
    ~ASTNode() = default;
    
    void add_child(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
        if (!children.empty()) {
            children.back()->parent = this;
        }
    }
};

// Function declarations
std::unique_ptr<ASTNode> create_node(NodeType type, int line = 0, int col = 0);
void print_ast(const ASTNode* node, int indent = 0);

#endif
```

Create `src/parser/ast.cc`:

```cpp
#include "ast.h"
#include <iostream>

std::unique_ptr<ASTNode> create_node(NodeType type, int line, int col) {
    return std::make_unique<ASTNode>(type, line, col);
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

void print_ast(const ASTNode* node, int indent) {
    if (!node) return;
    
    print_indent(indent);
    
    switch (node->type) {
        case NodeType::PROGRAM:
            std::cout << "Program\n";
            break;
        case NodeType::FUNCTION_DECLARATION:
            std::cout << "FunctionDecl: " << node->value.identifier << "\n";
            break;
        case NodeType::VARIABLE_DECLARATION:
            std::cout << "VariableDecl: " << node->value.identifier << "\n";
            break;
        case NodeType::IDENTIFIER:
            std::cout << "Ident: " << node->value.identifier << "\n";
            break;
        case NodeType::LITERAL:
            if (node->children.empty()) {
                std::cout << "Literal: " << node->value.string_value << "\n";
            } else {
                std::cout << "Literal\n";
            }
            break;
        case NodeType::BINARY_EXPRESSION:
            std::cout << "BinaryExpr\n";
            break;
        case NodeType::UNARY_EXPRESSION:
            std::cout << "UnaryExpr\n";
            break;
        case NodeType::CALL_EXPRESSION:
            std::cout << "CallExpr\n";
            break;
        case NodeType::IF_STATEMENT:
            std::cout << "IfStmt\n";
            break;
        case NodeType::FOR_STATEMENT:
            std::cout << "ForStmt\n";
            break;
        case NodeType::WHILE_STATEMENT:
            std::cout << "WhileStmt\n";
            break;
        case NodeType::RETURN_STATEMENT:
            std::cout << "ReturnStmt\n";
            break;
        case NodeType::BLOCK_STATEMENT:
            std::cout << "BlockStmt\n";
            break;
        default:
            std::cout << "Unknown\n";
            break;
    }
    
    for (const auto& child : node->children) {
        print_ast(child.get(), indent + 1);
    }
}
```

---

## Phase 3: Parser Implementation

Create `src/parser/parser.h`:

```cpp
#ifndef JS_COMPILER_PARSER_H
#define JS_COMPILER_PARSER_H

#include "../lexer/token.h"
#include "../parser/ast.h"

class Parser {
private:
    std::vector<Token>& tokens;
    int position;
    
    Token* current_token() {
        if (position >= tokens.size()) return nullptr;
        return &tokens[position];
    }
    
    Token* peek_token() {
        if (position + 1 >= tokens.size()) return nullptr;
        return &tokens[position + 1];
    }
    
    void next_token() { position++; }
    
    bool match(TokenType type) {
        return current_token() && current_token()->type == type;
    }
    
    bool expect(TokenType type);
    bool expect_keyword(const std::string& keyword);

public:
    Parser(std::vector<Token>& t) : tokens(t), position(0) {}
    
    std::unique_ptr<ASTNode> parse_program();
    std::unique_ptr<ASTNode> parse_function_declaration();
    std::unique_ptr<ASTNode> parse_variable_declaration();
    std::unique_ptr<ASTNode> parse_expression();
    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_block_statement();
};

#endif
```

Create `src/parser/parser.cc`:

```cpp
#include "parser.h"
#include <iostream>

bool Parser::expect(TokenType type) {
    if (match(type)) {
        next_token();
        return true;
    }
    std::cerr << "Expected token type " << token_type_name(type) << ", got " 
              << token_type_name(current_token()->type) << "\n";
    return false;
}

bool Parser::expect_keyword(const std::string& keyword) {
    if (current_token() && current_token()->type == TokenType::IDENTIFIER 
        && current_token()->lexeme == keyword) {
        next_token();
        return true;
    }
    std::cerr << "Expected keyword '" << keyword << "', got '" 
              << current_token()->lexeme << "'\n";
    return false;
}

std::unique_ptr<ASTNode> Parser::parse_program() {
    auto program = create_node(NodeType::PROGRAM, current_token()->line, current_token()->column);
    
    while (current_token() && current_token()->type != TokenType::EOF_TOKEN) {
        if (match(TokenType::VAR) || match(TokenType::LET) || match(TokenType::CONST)) {
            program->add_child(parse_variable_declaration());
        } else if (match(TokenType::FUNCTION)) {
            program->add_child(parse_function_declaration());
        } else {
            program->add_child(parse_statement());
        }
    }
    
    return program;
}

std::unique_ptr<ASTNode> Parser::parse_function_declaration() {
    expect(TokenType::FUNCTION);
    
    auto func = create_node(NodeType::FUNCTION_DECLARATION, 
                           current_token()->line, current_token()->column);
    
    // Parse function name
    if (match(TokenType::IDENTIFIER)) {
        func->value.identifier = current_token()->lexeme;
        next_token();
    }
    
    // Parse parameters
    expect(TokenType::LPAREN);
    while (!match(TokenType::RPAREN)) {
        if (match(TokenType::IDENTIFIER)) {
            auto param = create_node(NodeType::IDENTIFIER,
                                    current_token()->line, current_token()->column);
            param->value.identifier = current_token()->lexeme;
            func->add_child(std::move(param));
            next_token();
        }
        if (match(TokenType::COMMA)) next_token();
    }
    expect(TokenType::RPAREN);
    
    // Parse body
    expect(TokenType::LBRACE);
    func->add_child(parse_block_statement());
    expect(TokenType::RBRACE);
    
    return func;
}

std::unique_ptr<ASTNode> Parser::parse_variable_declaration() {
    TokenType kind = current_token()->type;
    next_token();  // Skip var/let/const
    
    auto var = create_node(NodeType::VARIABLE_DECLARATION,
                          current_token()->line, current_token()->column);
    var->value.int_value = kind;  // Store kind in int_value
    
    // Parse variable name
    if (match(TokenType::IDENTIFIER)) {
        var->value.identifier = current_token()->lexeme;
        next_token();
    }
    
    // Parse value
    if (match(TokenType::ASSIGN)) {
        next_token();
        var->add_child(parse_expression());
    }
    
    return var;
}

std::unique_ptr<ASTNode> Parser::parse_expression() {
    // Simple expression parser for now
    auto left = parse_primary_expression();
    
    // Handle binary expressions
    if (match(TokenType::ADD) || match(TokenType::SUB) ||
        match(TokenType::MUL) || match(TokenType::DIV)) {
        TokenType op = current_token()->type;
        next_token();
        auto right = parse_expression();
        
        auto expr = create_node(NodeType::BINARY_EXPRESSION,
                               current_token()->line, current_token()->column);
        expr->value.int_value = op;
        expr->add_child(std::move(left));
        expr->add_child(std::move(right));
        return expr;
    }
    
    return left;
}

std::unique_ptr<ASTNode> Parser::parse_primary_expression() {
    if (match(TokenType::NUMBER)) {
        auto lit = create_node(NodeType::LITERAL,
                              current_token()->line, current_token()->column);
        lit->value.float_value = std::stod(current_token()->lexeme);
        next_token();
        return lit;
    }
    
    if (match(TokenType::STRING)) {
        auto lit = create_node(NodeType::LITERAL,
                              current_token()->line, current_token()->column);
        lit->value.string_value = current_token()->lexeme;
        next_token();
        return lit;
    }
    
    if (match(TokenType::IDENTIFIER)) {
        auto ident = create_node(NodeType::IDENTIFIER,
                                current_token()->line, current_token()->column);
        ident->value.identifier = current_token()->lexeme;
        next_token();
        return ident;
    }
    
    if (match(TokenType::LPAREN)) {
        next_token();
        auto expr = parse_expression();
        expect(TokenType::RPAREN);
        return expr;
    }
    
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parse_statement() {
    if (match(TokenType::RETURN)) {
        next_token();
        auto ret = create_node(NodeType::RETURN_STATEMENT,
                              current_token()->line, current_token()->column);
        ret->add_child(parse_expression());
        return ret;
    }
    
    return parse_expression();
}

std::unique_ptr<ASTNode> Parser::parse_block_statement() {
    auto block = create_node(NodeType::BLOCK_STATEMENT,
                            current_token()->line, current_token()->column);
    
    while (!match(TokenType::RBRACE) && current_token() && 
           current_token()->type != TokenType::EOF_TOKEN) {
        block->add_child(parse_statement());
    }
    
    return block;
}
```

---

## Phase 4: Main Program

Create `src/main.cc`:

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include "lexer/token.h"
#include "parser/parser.h"

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return "";
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    
    return content;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <js_file>\n";
        return 1;
    }
    
    std::cout << "=== JavaScript Compiler ===\n";
    std::cout << "Compiling: " << argv[1] << "\n\n";
    
    // Read source file
    std::string source = read_file(argv[1]);
    if (source.empty()) {
        return 1;
    }
    
    // Lexical analysis
    std::cout << "Phase 1: Lexical Analysis\n";
    std::vector<Token> tokens = tokenize(source);
    std::cout << "  Found " << tokens.size() << " tokens\n\n";
    
    // Parse
    std::cout << "Phase 2: Syntax Analysis\n";
    Parser parser(tokens);
    auto ast = parser.parse_program();
    
    std::cout << "AST:\n";
    print_ast(ast.get());
    std::cout << "\n";
    
    std::cout << "Compilation completed!\n";
    return 0;
}
```

---

## Phase 5: Makefile

Create `makefile`:

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
LDFLAGS = 

TARGET = jsc
SRC_DIR = src
BUILD_DIR = build

SOURCES = $(SRC_DIR)/main.cc \
          $(SRC_DIR)/lexer/token.cc \
          $(SRC_DIR)/parser/ast.cc \
          $(SRC_DIR)/parser/parser.cc

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)

.PHONY: all clean run test setup

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET) examples/hello.js

test: $(TARGET)
	@echo "=== Running Tests ==="
	@./$(TARGET) examples/hello.js
	@./$(TARGET) examples/factorial.js

setup:
	@echo "Creating example JavaScript files..."
	@mkdir -p examples
	@echo 'console.log("Hello, JavaScript Compiler!");' > examples/hello.js
	@echo 'function factorial(n) {' > examples/factorial.js
	@echo '    if (n <= 1) {' >> examples/factorial.js
	@echo '        return 1;' >> examples/factorial.js
	@echo '    }' >> examples/factorial.js
	@echo '    return n * factorial(n - 1);' >> examples/factorial.js
	@echo '}' >> examples/factorial.js
	@echo 'console.log("5! =", factorial(5));' >> examples/factorial.js

rebuild: clean all
```

---

## Build and Run

```bash
cd JavaScriptCompiler
make setup      # Create example files
make            # Build
./jsc examples/hello.js
```

---

## Next Steps

1. Implement more expression types
2. Add semantic analysis
3. Implement bytecode generation
4. Add interpreter
5. Implement JIT compilation