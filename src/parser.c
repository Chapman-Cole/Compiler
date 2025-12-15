#include "parser.h"
#include "DynamicArray.h"
#include "Strings.h"
#include "lexer.h"

int ast_module_init(void) {
    dynamic_array_registry_type_append(&STRING("AST"), ast_deallocator, sizeof(AST));
    return 0;
}

int ast_deallocator(void* ast) {
    dynamic_array_free(&((AST*)ast)->branches);
    string_free(&((AST*)ast)->name);
    return 0;
}

int ast_init(AST* ast) {
    dynamic_array_init(&ast->branches, &STRING("AST"));
    return 0;
}

// Takes in an array of tokens and the string for the original source file for debugging purposes
int ast_generate(AST* ast, DynamicArray *tokens, string *file) {
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
