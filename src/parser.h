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
    AST_FOR
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

int ast_generate(DynamicArray* tokens, string* file);

#endif
