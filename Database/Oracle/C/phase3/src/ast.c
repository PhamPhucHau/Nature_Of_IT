/*
 * ast.c - Abstract Syntax Tree Implementation
 * 
 * Implements AST node creation, destruction, and utility functions.
 * 
 * Author: Database Learning Project
 * Date: May 2026
 */

#include "ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/*
 * ast_get_type_string - Convert AST node type to string
 * 
 * Returns a human-readable string representation of an AST node type.
 */
const char* ast_get_type_string(ASTNodeType type) {
    switch (type) {
        case AST_SELECT:        return "SELECT";
        case AST_INSERT:        return "INSERT";
        case AST_CREATE_TABLE:  return "CREATE_TABLE";
        case AST_DELETE:        return "DELETE";
        case AST_COLUMN_LIST:   return "COLUMN_LIST";
        case AST_VALUE_LIST:    return "VALUE_LIST";
        case AST_WHERE_CLAUSE:  return "WHERE_CLAUSE";
        case AST_CONDITION:     return "CONDITION";
        case AST_COLUMN_DEF:    return "COLUMN_DEF";
        case AST_IDENTIFIER:    return "IDENTIFIER";
        case AST_LITERAL:       return "LITERAL";
        default:                return "UNKNOWN";
    }
}

/*
 * ast_get_data_type_string - Convert data type to string
 * 
 * Returns a human-readable string representation of a data type.
 */
const char* ast_get_data_type_string(DataType type) {
    switch (type) {
        case DATA_TYPE_INT:     return "INT";
        case DATA_TYPE_VARCHAR: return "VARCHAR";
        case DATA_TYPE_FLOAT:   return "FLOAT";
        case DATA_TYPE_CHAR:    return "CHAR";
        default:                return "UNKNOWN";
    }
}

/* ============================================================================
 * AST NODE CREATION
 * ============================================================================ */

/*
 * ast_create_select - Create a SELECT statement node
 * 
 * Allocates and initializes a SELECT statement AST node.
 */
ASTNode* ast_create_select(const char *table_name, ASTNode **columns,
                           uint32_t column_count, ASTNode *where_clause) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for SELECT node\n");
        return NULL;
    }
    
    node->type = AST_SELECT;
    node->data.select.table_name = (char *)malloc(strlen(table_name) + 1);
    if (!node->data.select.table_name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for table name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.select.table_name, table_name);
    node->data.select.columns = columns;
    node->data.select.column_count = column_count;
    node->data.select.where_clause = where_clause;
    
    return node;
}

/*
 * ast_create_insert - Create an INSERT statement node
 * 
 * Allocates and initializes an INSERT statement AST node.
 */
ASTNode* ast_create_insert(const char *table_name, ASTNode **columns,
                           uint32_t column_count, ASTNode **values,
                           uint32_t value_count) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for INSERT node\n");
        return NULL;
    }
    
    node->type = AST_INSERT;
    node->data.insert.table_name = (char *)malloc(strlen(table_name) + 1);
    if (!node->data.insert.table_name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for table name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.insert.table_name, table_name);
    node->data.insert.columns = columns;
    node->data.insert.column_count = column_count;
    node->data.insert.values = values;
    node->data.insert.value_count = value_count;
    
    return node;
}

/*
 * ast_create_create_table - Create a CREATE TABLE statement node
 * 
 * Allocates and initializes a CREATE TABLE statement AST node.
 */
ASTNode* ast_create_create_table(const char *table_name, ASTNode **columns,
                                 uint32_t column_count) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for CREATE TABLE node\n");
        return NULL;
    }
    
    node->type = AST_CREATE_TABLE;
    node->data.create.table_name = (char *)malloc(strlen(table_name) + 1);
    if (!node->data.create.table_name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for table name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.create.table_name, table_name);
    node->data.create.columns = columns;
    node->data.create.column_count = column_count;
    
    return node;
}

/*
 * ast_create_delete - Create a DELETE statement node
 * 
 * Allocates and initializes a DELETE statement AST node.
 */
ASTNode* ast_create_delete(const char *table_name, ASTNode *where_clause) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for DELETE node\n");
        return NULL;
    }
    
    node->type = AST_DELETE;
    node->data.delete.table_name = (char *)malloc(strlen(table_name) + 1);
    if (!node->data.delete.table_name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for table name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.delete.table_name, table_name);
    node->data.delete.where_clause = where_clause;
    
    return node;
}

/*
 * ast_create_column_def - Create a column definition node
 * 
 * Allocates and initializes a column definition AST node.
 */
ASTNode* ast_create_column_def(const char *name, DataType type, uint32_t size) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for column definition node\n");
        return NULL;
    }
    
    node->type = AST_COLUMN_DEF;
    node->data.column_def.name = (char *)malloc(strlen(name) + 1);
    if (!node->data.column_def.name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for column name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.column_def.name, name);
    node->data.column_def.type = type;
    node->data.column_def.size = size;
    
    return node;
}

/*
 * ast_create_condition - Create a condition node
 * 
 * Allocates and initializes a condition AST node.
 */
ASTNode* ast_create_condition(ASTNode *left, OperatorType operator,
                              ASTNode *right) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for condition node\n");
        return NULL;
    }
    
    node->type = AST_CONDITION;
    node->data.condition.left = left;
    node->data.condition.operator = operator;
    node->data.condition.right = right;
    
    return node;
}

/*
 * ast_create_identifier - Create an identifier node
 * 
 * Allocates and initializes an identifier AST node.
 */
ASTNode* ast_create_identifier(const char *name) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for identifier node\n");
        return NULL;
    }
    
    node->type = AST_IDENTIFIER;
    node->data.identifier.name = (char *)malloc(strlen(name) + 1);
    if (!node->data.identifier.name) {
        fprintf(stderr, "ERROR: Failed to allocate memory for identifier name\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.identifier.name, name);
    
    return node;
}

/*
 * ast_create_literal - Create a literal node
 * 
 * Allocates and initializes a literal AST node.
 */
ASTNode* ast_create_literal(const char *value, int is_string) {
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "ERROR: Failed to allocate memory for literal node\n");
        return NULL;
    }
    
    node->type = AST_LITERAL;
    node->data.literal.value = (char *)malloc(strlen(value) + 1);
    if (!node->data.literal.value) {
        fprintf(stderr, "ERROR: Failed to allocate memory for literal value\n");
        free(node);
        return NULL;
    }
    
    strcpy(node->data.literal.value, value);
    node->data.literal.is_string = is_string;
    
    return node;
}

/* ============================================================================
 * AST NODE DESTRUCTION
 * ============================================================================ */

/*
 * ast_destroy - Free AST node and all children
 * 
 * Recursively frees all memory used by an AST node and its children.
 */
void ast_destroy(ASTNode *node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_SELECT: {
            SelectStatement *select = &node->data.select;
            if (select->table_name) free(select->table_name);
            if (select->columns) {
                for (uint32_t i = 0; i < select->column_count; i++) {
                    ast_destroy(select->columns[i]);
                }
                free(select->columns);
            }
            if (select->where_clause) {
                ast_destroy(select->where_clause);
            }
            break;
        }
        case AST_INSERT: {
            InsertStatement *insert = &node->data.insert;
            if (insert->table_name) free(insert->table_name);
            if (insert->columns) {
                for (uint32_t i = 0; i < insert->column_count; i++) {
                    ast_destroy(insert->columns[i]);
                }
                free(insert->columns);
            }
            if (insert->values) {
                for (uint32_t i = 0; i < insert->value_count; i++) {
                    ast_destroy(insert->values[i]);
                }
                free(insert->values);
            }
            break;
        }
        case AST_CREATE_TABLE: {
            CreateTableStatement *create = &node->data.create;
            if (create->table_name) free(create->table_name);
            if (create->columns) {
                for (uint32_t i = 0; i < create->column_count; i++) {
                    ast_destroy(create->columns[i]);
                }
                free(create->columns);
            }
            break;
        }
        case AST_DELETE: {
            DeleteStatement *delete = &node->data.delete;
            if (delete->table_name) free(delete->table_name);
            if (delete->where_clause) {
                ast_destroy(delete->where_clause);
            }
            break;
        }
        case AST_COLUMN_DEF: {
            ColumnDefinition *col_def = &node->data.column_def;
            if (col_def->name) free(col_def->name);
            break;
        }
        case AST_CONDITION: {
            Condition *cond = &node->data.condition;
            if (cond->left) ast_destroy(cond->left);
            if (cond->right) ast_destroy(cond->right);
            break;
        }
        case AST_IDENTIFIER: {
            Identifier *id = &node->data.identifier;
            if (id->name) free(id->name);
            break;
        }
        case AST_LITERAL: {
            Literal *lit = &node->data.literal;
            if (lit->value) free(lit->value);
            break;
        }
        default:
            break;
    }
    
    free(node);
}

/* ============================================================================
 * AST PRINTING
 * ============================================================================ */

/*
 * ast_print - Print AST in human-readable format
 * 
 * Prints the AST structure with indentation showing the tree hierarchy.
 */
void ast_print(ASTNode *node, int indent) {
    if (!node) return;
    
    char indent_str[256] = "";
    for (int i = 0; i < indent; i++) {
        strcat(indent_str, "  ");
    }
    
    printf("%s[%s]\n", indent_str, ast_get_type_string(node->type));
    
    switch (node->type) {
        case AST_SELECT: {
            SelectStatement *select = &node->data.select;
            printf("%s  table: %s\n", indent_str, select->table_name);
            printf("%s  columns:\n", indent_str);
            for (uint32_t i = 0; i < select->column_count; i++) {
                ast_print(select->columns[i], indent + 2);
            }
            if (select->where_clause) {
                printf("%s  where:\n", indent_str);
                ast_print(select->where_clause, indent + 2);
            }
            break;
        }
        case AST_INSERT: {
            InsertStatement *insert = &node->data.insert;
            printf("%s  table: %s\n", indent_str, insert->table_name);
            printf("%s  columns:\n", indent_str);
            for (uint32_t i = 0; i < insert->column_count; i++) {
                ast_print(insert->columns[i], indent + 2);
            }
            printf("%s  values:\n", indent_str);
            for (uint32_t i = 0; i < insert->value_count; i++) {
                ast_print(insert->values[i], indent + 2);
            }
            break;
        }
        case AST_CREATE_TABLE: {
            CreateTableStatement *create = &node->data.create;
            printf("%s  table: %s\n", indent_str, create->table_name);
            printf("%s  columns:\n", indent_str);
            for (uint32_t i = 0; i < create->column_count; i++) {
                ast_print(create->columns[i], indent + 2);
            }
            break;
        }
        case AST_DELETE: {
            DeleteStatement *delete = &node->data.delete;
            printf("%s  table: %s\n", indent_str, delete->table_name);
            if (delete->where_clause) {
                printf("%s  where:\n", indent_str);
                ast_print(delete->where_clause, indent + 2);
            }
            break;
        }
        case AST_COLUMN_DEF: {
            ColumnDefinition *col_def = &node->data.column_def;
            printf("%s  name: %s\n", indent_str, col_def->name);
            printf("%s  type: %s\n", indent_str, ast_get_data_type_string(col_def->type));
            if (col_def->size > 0) {
                printf("%s  size: %u\n", indent_str, col_def->size);
            }
            break;
        }
        case AST_CONDITION: {
            Condition *cond = &node->data.condition;
            printf("%s  left:\n", indent_str);
            ast_print(cond->left, indent + 2);
            printf("%s  operator: %d\n", indent_str, cond->operator);
            printf("%s  right:\n", indent_str);
            ast_print(cond->right, indent + 2);
            break;
        }
        case AST_IDENTIFIER: {
            Identifier *id = &node->data.identifier;
            printf("%s  name: %s\n", indent_str, id->name);
            break;
        }
        case AST_LITERAL: {
            Literal *lit = &node->data.literal;
            printf("%s  value: %s\n", indent_str, lit->value);
            printf("%s  is_string: %d\n", indent_str, lit->is_string);
            break;
        }
        default:
            break;
    }
}
