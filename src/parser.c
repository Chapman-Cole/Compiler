#include "parser.h"
#include "DynamicArray.h"
#include "lexer.h"

// Takes in an array of tokens and the string for the original source file for debugging purposes
int ast_generate(DynamicArray *tokens, string *file) {
    // List of general heuristics used to figure out the structure of the AST
    // Should include things like figuring out variable declaration from a list of tokens like keyword + identifier + keyword + literal
    for (int i = 0; i < tokens->len; i++) {
        token curr_token = *(token*)dynamic_array_get(tokens, &INDEX(i));
        
        if (curr_token.type == LRES_KEYWORD && i < tokens->len - 1) {
            token next_token = *(token*)dynamic_array_get(tokens, &INDEX(i + 1));
            switch (curr_token.type) {
                case KEY_INT:
                    break;

                case KEY_FLOAT:
                    break;

                case KEY_STRING:
                    break;
            }
        }
    }
    return 0;
}
