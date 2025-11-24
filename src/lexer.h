#ifndef LEXER_H
#define LEXER_H

#include "Strings.h"
#include "DynamicArray.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// To Do...
// create a lexer that tokenizes identifiers, keywords, punctuators, operators, literals, and comments
// lexer will create a dynamic array of the tokens in the order they appear in the file so that the 
// parser can do its job in creating the abstract syntax tree later on

typedef struct token {
    unsigned int type; // The actual internal id for that keyword, punctuator, etc.
    unsigned int id; // The id assigned to that specific keyword, punctuator, etc.

    // Used to store a string representation of the literal to be converted to its respective type
    // at a later point
    // This is also used by identifiers to store the identifier itself as a string
    // keywords use this for debugging purposes to keep the string version of the keyword available
    // for print debugging
    string literal;
} token;

// Used to specify the type field in the token parameter
// A LITERAL isn't a reserved language word, but it fits
// same as above for IDENTIFIER 
// well in here for how it is used in the token struct (the type would be LRES_LITERAL to indicate)
// a token literal
enum Languge_Reserved {
    LRES_KEYWORD,
    LRES_PUNCTUATOR,
    LRES_COMMENT,
    LRES_LITERAL,
    LRES_OPERATOR,
    LRES_IDENTIFIER
};

enum Keywords {
    KEY_INT,
    KEY_FLOAT,
    KEY_IF,
    KEY_WHILE,
    KEY_FOR,
    KEY_RETURN
};

enum Punctuators {
    PUNC_SEMICOLON,
    PUNC_BRACKET_L,
    PUNC_BRACKET_R,
    PUNC_CURLY_L,
    PUNC_CURLY_R,
    PUNC_PAREN_L,
    PUNC_PAREN_R
};

enum Operators {
    OP_EQUAL,
    OP_PLUS,
    OP_MINUS,
    OP_MULT,
    OP_DIV
};

enum Comments {
    COMM_DSLASH
};

// Used specifically to create key, value pairing between string and unsigned integer identifier for keywords, 
// punctuators, operators, and comments
typedef struct language_identifier {
    string name; // The string identifier itself, such as int or float.
    unsigned int type; // The actual internal id for a keyword, punctuator, etc.
    unsigned int id; // The id assigned to that specific keyword, punctuator, etc.
    bool vtag; // Used to specify whether or not the keyword is a variable type, if the keyword is a variable type at all
} language_identifier;

// A dynamic array containing a list of language_identifiers, each containing the string, type, and val of a keyword, punctuator, or comment
static DynamicArray LanguageReservedWords;

// For use specifically with the dynamic array type registry
int token_deallocator(void* tok);

// Must be called before using the lexer function 
// purpose is to initialize list of keywords, punctuators, etc.
int lexer_module_init(void);

// Must be called once done using the lexer
// frees up some memory that was previously allocated.
int lexer_module_terminate(void);

// This will perform the actual lexical analysis and generate the dynamic array of tokens
// The tokens dynamic array needs to be properly initialized before calling this function,
// which should look something like dynamic_array_init(&tokens, &STRING("tokens"));
// The file string also needs to be initialized so that references to strings within
// the file can be made later on if needed (this is mostly for debugging)
// the string containing the code should already be loaded into the file string before being passed into the lexer
int lexer(DynamicArray* tokens, string* file);

#endif
