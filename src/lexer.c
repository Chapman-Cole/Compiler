#include "lexer.h"
#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>

int token_deallocator(void* tok) {
    // Only deallocate if it is a literal type
    if (((token*)tok)->type == LRES_LITERAL) {
        string_free(&((token*)tok)->literal);
    }
    return 0;
}

int lexer_module_init(void) {
    dynamic_array_registry_type_append(&STRING("token"), token_deallocator, sizeof(token));
    dynamic_array_registry_type_append(&STRING("language_identifier"), NULL, sizeof(language_identifier));
    dynamic_array_init(&LanguageReservedWords, &STRING("language_identifier"));

    // The keywords are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_INT, .name = STRING("int")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_FLOAT, .name = STRING("float")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_FOR, .name = STRING("for")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_IF, .name = STRING("if")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_WHILE, .name = STRING("while")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_RETURN, .name = STRING("return")});

    // The punctuators are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_BRACKET_L, .name = STRING("[")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_BRACKET_R, .name = STRING("]")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_CURLY_L, .name = STRING("{")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_CURLY_R, .name = STRING("}")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_PAREN_L, .name = STRING("(")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_PAREN_R, .name = STRING(")")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_SEMICOLON, .name = STRING(";")});

    // The comments are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_COMMENT, .id = COMM_DSLASH, .name = STRING("//")});

    return 0;
}

int lexer_module_terminate(void) {
    dynamic_array_free(&LanguageReservedWords);
    return 0;
}

DynamicArray lexer(string* path) {
    string file;
    string_init(&file);
    string_read_file(&file, path);

    DynamicArray tokens;
    dynamic_array_init(&tokens, &STRING("token"));

    unsigned int quoteCount = 0;
    unsigned int prevQuoteCount = 0;
    unsigned int quoteIndices[2];
    for (int i = 0; i < file.len; i++) {
        // Looks for the string literals
        if (file.str[i] == '"') {
            quoteIndices[quoteCount % 2] = i;
            quoteCount++;

            // This means that the body of the quotes has just been exited
            if (prevQuoteCount % 2 == 1 && quoteCount % 2 == 0) {
                string literal;
                string_init(&literal);
                // Add +1 to the from argument below because otherwise quote symbol would be included
                string_substring(&literal, &file, quoteIndices[0] + 1, quoteIndices[1]);
                dynamic_array_append(&tokens, &(token){.literal = literal, .type = LRES_LITERAL});
            }
        }

        for (int j = 0; j < LanguageReservedWords.len; j++) {
            language_identifier* ldent = dynamic_array_get(&LanguageReservedWords, &INDEX(j));
            if (string_compare_with_offset(&file, &ldent->name, i)) {
                // For debugging purposes, the literal part of the token will contain the string name of the keyword
                dynamic_array_append(&tokens, &(token){.type = ldent->type, .id = ldent->id, .literal = ldent->name});
            }
        }

        prevQuoteCount = quoteCount;
    }

    string_free(&file);
    return tokens;
}
