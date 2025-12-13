#include "DynamicArray.h"
#include "Strings.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc <= 1) {
        return -1;
    }

    dynamic_array_registry_init();
    lexer_module_init();
    
    DynamicArray tokens;
    dynamic_array_init(&tokens, &STRING("token"));

    DynamicArray identifiers;
    dynamic_array_init(&identifiers, &STRING("string"));

    string file;
    string_init(&file);
    string_read_file(&file, &(string){.str = argv[1], .len = strlen(argv[1]), .__memsize = 0});
    lexer(&tokens, &identifiers, &file);

    for (int i = 0; i < tokens.len; i++) {
        token* tok = dynamic_array_get(&tokens, &INDEX(i));
        if (tok->type == LRES_LITERAL) {
            printf("TOKEN %d:\ntype: literal\nval: %s\n\n", i, tok->literal.str);
        } else if (tok->type == LRES_KEYWORD) {
             printf("TOKEN %d:\ntype: keyword\nval: %s\n\n", i, tok->literal.str);
        } else if (tok->type == LRES_PUNCTUATOR) {
            printf("TOKEN %d:\ntype: punctuator\nval: %s\n\n", i, tok->literal.str);
        } else if (tok->type == LRES_OPERATOR) {
            printf("TOKEN %d:\ntype: operator\nval: %s\n\n", i, tok->literal.str);
        } else if (tok->type == LRES_IDENTIFIER) {
            printf("TOKEN %d:\ntype: identifier\nval: %s\n\n", i, tok->literal.str);
        }
    }
    
    dynamic_array_free(&tokens);
    dynamic_array_free(&identifiers);
    string_free(&file);
    lexer_module_terminate();
    dynamic_array_registry_terminate();
    return 0;
}
