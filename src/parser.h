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
    AST_CONSTANT
};

typedef struct AST {
    // The "branches" of the abstract syntax tree, as in the sub nodes for each node
    // Specifically, it will be a dynamic array of type AST
    DynamicArray branches;
    // The type of the specific node: variable, expression, assignment, comparison, constant, etc.
    unsigned int type; 
} AST;

#endif
