#include "DynamicArray.h"
#include "Strings.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    dynamic_array_registry_init();
    lexer_module_init();
    
    DynamicArray tokens;
    tokens = lexer(&STRING("test.txt"));

    for (int i = 0; i < tokens.len; i++) {
        token* tok = dynamic_array_get(&tokens, &INDEX(i));
        if (tok->type == LRES_LITERAL) {
            printf("TOKEN %d:\ntype = literal\nval = %s\n\n", i, tok->literal.str);
        } else if (tok->type == LRES_KEYWORD) {
             printf("TOKEN %d:\ntype = keyword\nval = %s\n\n", i, tok->literal.str);
        }
    }
    
    dynamic_array_free(&tokens);
    lexer_module_terminate();
    dynamic_array_registry_terminate();
    return 0;
}
