#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "DynamicArray.h"
#include "Strings.h"

// This defines the different possible types for an AST node
enum AST_types {
    AST_VARIABLE,
    AST_ASSIGNMENT,
    AST_EXPRESSION,
    AST_COMPARISON,
    AST_INT_CONSTANT,
    AST_FLOAT_CONSTANT,
    AST_STRING_CONSTANT,
    AST_BRANCH,
    AST_WHILE,
    AST_FOR,
    AST_ROOT
};

typedef struct AST {
    // The "branches" of the abstract syntax tree, as in the sub nodes for each node
    // Specifically, it will be a dynamic array of type AST
    DynamicArray branches;
    // The type of the specific node: variable, expression, assignment, comparison, constant, etc.
    unsigned int type;
    // This is used to store the name of a variable, if applicable. Could also be another identifier
    string name;
    // This is used for storing 
    unsigned int data;
} AST;

// Should be called only once in the lifetime of a program before using the parser functions
// It basically just initializes some critical values for this module to work properly
int ast_module_init(void);

// For use within the initialization of the AST struct into the dynamic array type registry
int ast_deallocator(void* ast);

// Should be called before using any functions that involve an abstract syntax tree
int ast_init(AST* ast);

// Generates the actual abstract syntax tree 
int ast_generate(AST* ast, DynamicArray* tokens, DynamicArray* identifiers, string* file);

#endif
