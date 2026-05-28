/*
 * ast.h - Abstract Syntax Tree (AST) Definitions
 * 
 * Defines the AST node structures that represent parsed SQL statements.
 * The AST is the output of the parser and represents the structure of
 * the SQL command in a tree format.
 * 
 * Key Concepts:
 * - AST Node: Represents a single element in the parse tree
 * - Node Types: Different types for different SQL statements
 * - Tree Structure: Hierarchical representation of SQL
 * - Traversal: Walking the tree to process SQL
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#ifndef AST_H
#define AST_H

#include <stdint.h>     
#include <stddef.h>   

/* ============================================================================
 * AST NODE TYPES
 * ============================================================================
 * 
 * ASTNodeType enumeration defines all possible AST node types.
 * Each type corresponds to a different SQL construct.
 */

typedef enum {
    AST_SELECT,             /* SELECT statement */
    AST_INSERT,             /* INSERT statement */
    AST_CREATE_TABLE,       /* CREATE TABLE statement */
    AST_DELETE,             /* DELETE statement */
    AST_COLUMN_LIST,        /* List of columns */
    AST_VALUE_LIST,         /* List of values */
    AST_WHERE_CLAUSE,       /* WHERE clause */
    AST_CONDITION,          /* Comparison condition */
    AST_COLUMN_DEF,         /* Column definition */
    AST_IDENTIFIER,         /* Identifier (table/column name) */
    AST_LITERAL             /* Literal value (number/string) */
} ASTNodeType;

/* ============================================================================
 * DATA TYPE DEFINITIONS
 * ============================================================================ */

typedef enum {
    DATA_TYPE_INT,
    DATA_TYPE_VARCHAR,
    DATA_TYPE_FLOAT,
    DATA_TYPE_CHAR
} DataType;

typedef enum {
    OPERATOR_EQUALS,
    OPERATOR_NOT_EQUALS,
    OPERATOR_LESS_THAN,
    OPERATOR_GREATER_THAN,
    OPERATOR_LESS_EQUAL,
    OPERATOR_GREATER_EQUAL,
    OPERATOR_AND,
    OPERATOR_OR
} OperatorType;

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

typedef struct ASTNode ASTNode;

/* ============================================================================
 * AST NODE STRUCTURES
 * ============================================================================ */

/*
 * SelectStatement - Represents a SELECT statement
 * 
 * Structure:
 *   SELECT column_list FROM table_name [WHERE condition]
 * 
 * Fields:
 *   - columns: List of column names to select
 *   - table_name: Name of table to select from
 *   - where_clause: Optional WHERE clause
 */
typedef struct {
    ASTNode **columns;              /* Array of column nodes */
    uint32_t column_count;          /* Number of columns */
    char *table_name;               /* Table name */
    ASTNode *where_clause;          /* Optional WHERE clause */
} SelectStatement;

/*
 * InsertStatement - Represents an INSERT statement
 * 
 * Structure:
 *   INSERT INTO table_name (column_list) VALUES (value_list)
 * 
 * Fields:
 *   - table_name: Name of table to insert into
 *   - columns: List of column names
 *   - values: List of values to insert
 */
typedef struct {
    char *table_name;               /* Table name */
    ASTNode **columns;              /* Array of column nodes */
    uint32_t column_count;          /* Number of columns */
    ASTNode **values;               /* Array of value nodes */
    uint32_t value_count;           /* Number of values */
} InsertStatement;

/*
 * CreateTableStatement - Represents a CREATE TABLE statement
 * 
 * Structure:
 *   CREATE TABLE table_name (column_def, ...)
 * 
 * Fields:
 *   - table_name: Name of table to create
 *   - columns: Array of column definitions
 */
typedef struct {
    char *table_name;               /* Table name */
    ASTNode **columns;              /* Array of column definition nodes */
    uint32_t column_count;          /* Number of columns */
} CreateTableStatement;

/*
 * DeleteStatement - Represents a DELETE statement
 * 
 * Structure:
 *   DELETE FROM table_name [WHERE condition]
 * 
 * Fields:
 *   - table_name: Name of table to delete from
 *   - where_clause: Optional WHERE clause
 */
typedef struct {
    char *table_name;               /* Table name */
    ASTNode *where_clause;          /* Optional WHERE clause */
} DeleteStatement;

/*
 * ColumnDefinition - Represents a column definition
 * 
 * Structure:
 *   column_name TYPE [size]
 * 
 * Fields:
 *   - name: Column name
 *   - type: Data type
 *   - size: Size for VARCHAR (0 if not applicable)
 */
typedef struct {
    char *name;                     /* Column name */
    DataType type;                  /* Data type */
    uint32_t size;                  /* Size for VARCHAR */
} ColumnDefinition;

/*
 * Condition - Represents a comparison condition
 * 
 * Structure:
 *   left_operand operator right_operand
 * 
 * Fields:
 *   - left: Left operand (usually identifier)
 *   - operator: Comparison operator
 *   - right: Right operand (usually literal)
 */
typedef struct {
    ASTNode *left;                  /* Left operand */
    OperatorType operator;          /* Operator */
    ASTNode *right;                 /* Right operand */
} Condition;

/*
 * Identifier - Represents an identifier (table/column name)
 * 
 * Fields:
 *   - name: The identifier string
 */
typedef struct {
    char *name;                     /* Identifier name */
} Identifier;

/*
 * Literal - Represents a literal value
 * 
 * Fields:
 *   - value: String representation of value
 *   - is_string: 1 if string literal, 0 if numeric
 */
typedef struct {
    char *value;                    /* Literal value */
    int is_string;                  /* 1 if string, 0 if number */
} Literal;

/*
 * ASTNode - Generic AST node
 * 
 * This is the main AST node structure that can represent any SQL construct.
 * It uses a union to store different types of data depending on the node type.
 * 
 * Fields:
 *   - type: Type of this node
 *   - data: Union containing node-specific data
 * 
 * Example:
 *   ASTNode *node = ast_create_select(...);
 *   if (node->type == AST_SELECT) {
 *       SelectStatement *select = &node->data.select;
 *       printf("Table: %s\n", select->table_name);
 *   }
 */
typedef struct ASTNode {
    ASTNodeType type;               /* Type of this node */
    union {
        SelectStatement select;
        InsertStatement insert;
        CreateTableStatement create;
        DeleteStatement delete;
        ColumnDefinition column_def;
        Condition condition;
        Identifier identifier;
        Literal literal;
    } data;                         /* Node-specific data */
} ASTNode;

/* ============================================================================
 * FUNCTION DECLARATIONS
 * ============================================================================ */

/*
 * ast_create_select - Create a SELECT statement node
 * 
 * Parameters:
 *   table_name: Name of table to select from
 *   columns: Array of column nodes
 *   column_count: Number of columns
 *   where_clause: Optional WHERE clause (can be NULL)
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_select(const char *table_name, ASTNode **columns,
                           uint32_t column_count, ASTNode *where_clause);

/*
 * ast_create_insert - Create an INSERT statement node
 * 
 * Parameters:
 *   table_name: Name of table to insert into
 *   columns: Array of column nodes
 *   column_count: Number of columns
 *   values: Array of value nodes
 *   value_count: Number of values
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_insert(const char *table_name, ASTNode **columns,
                           uint32_t column_count, ASTNode **values,
                           uint32_t value_count);

/*
 * ast_create_create_table - Create a CREATE TABLE statement node
 * 
 * Parameters:
 *   table_name: Name of table to create
 *   columns: Array of column definition nodes
 *   column_count: Number of columns
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_create_table(const char *table_name, ASTNode **columns,
                                 uint32_t column_count);

/*
 * ast_create_delete - Create a DELETE statement node
 * 
 * Parameters:
 *   table_name: Name of table to delete from
 *   where_clause: Optional WHERE clause (can be NULL)
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_delete(const char *table_name, ASTNode *where_clause);

/*
 * ast_create_column_def - Create a column definition node
 * 
 * Parameters:
 *   name: Column name
 *   type: Data type
 *   size: Size for VARCHAR (0 if not applicable)
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_column_def(const char *name, DataType type, uint32_t size);

/*
 * ast_create_condition - Create a condition node
 * 
 * Parameters:
 *   left: Left operand
 *   operator: Comparison operator
 *   right: Right operand
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_condition(ASTNode *left, OperatorType operator,
                              ASTNode *right);

/*
 * ast_create_identifier - Create an identifier node
 * 
 * Parameters:
 *   name: Identifier name
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_identifier(const char *name);

/*
 * ast_create_literal - Create a literal node
 * 
 * Parameters:
 *   value: Literal value
 *   is_string: 1 if string, 0 if numeric
 * 
 * Returns:
 *   Pointer to new ASTNode, or NULL on error
 */
ASTNode* ast_create_literal(const char *value, int is_string);

/*
 * ast_destroy - Free AST node and all children
 * 
 * Recursively frees all memory used by an AST node and its children.
 * 
 * Parameters:
 *   node: Pointer to ASTNode
 * 
 * Returns:
 *   void
 */
void ast_destroy(ASTNode *node);

/*
 * ast_print - Print AST in human-readable format
 * 
 * Prints the AST structure with indentation showing the tree hierarchy.
 * Useful for debugging the parser.
 * 
 * Parameters:
 *   node: Pointer to ASTNode
 *   indent: Indentation level (usually 0 for root)
 * 
 * Returns:
 *   void
 */
void ast_print(ASTNode *node, int indent);

/*
 * ast_get_type_string - Convert AST node type to string
 * 
 * Returns a human-readable string representation of an AST node type.
 * 
 * Parameters:
 *   type: ASTNodeType enum value
 * 
 * Returns:
 *   String representation of node type
 */
const char* ast_get_type_string(ASTNodeType type);

/*
 * ast_get_data_type_string - Convert data type to string
 * 
 * Returns a human-readable string representation of a data type.
 * 
 * Parameters:
 *   type: DataType enum value
 * 
 * Returns:
 *   String representation of data type
 */
const char* ast_get_data_type_string(DataType type);

#endif // AST_H
