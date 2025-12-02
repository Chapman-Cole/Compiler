#include "lexer.h"
#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int token_deallocator(void* tok) {
    // Only deallocate if it is not a keyword token
    if (((token*)tok)->type == LRES_LITERAL || ((token*)tok)->type == LRES_IDENTIFIER) {
        string_free(&((token*)tok)->literal);
    }
    return 0;
}

int lexer_module_init(void) {
    dynamic_array_registry_type_append(&STRING("token"), token_deallocator, sizeof(token));
    dynamic_array_registry_type_append(&STRING("language_identifier"), NULL, sizeof(language_identifier));
    dynamic_array_init(&LanguageReservedWords, &STRING("language_identifier"));

    // The comments are being appended below. Important to come first to potentially save time on not checking reduantly
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_COMMENT, .id = COMM_DSLASH, .name = STRING("//")});

    // The keywords are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_INT, .name = STRING("int"), .vtag = true});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_FLOAT, .name = STRING("float"), .vtag = true});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_FLOAT, .name = STRING("string"), .vtag = true});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_FOR, .name = STRING("for"), .vtag = false});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_IF, .name = STRING("if"), .vtag = false});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_WHILE, .name = STRING("while"), .vtag = false});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_KEYWORD, .id = KEY_RETURN, .name = STRING("return "), .vtag = false});

    // The punctuators are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_BRACKET_L, .name = STRING("[")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_BRACKET_R, .name = STRING("]")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_CURLY_L, .name = STRING("{")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_CURLY_R, .name = STRING("}")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_PAREN_L, .name = STRING("(")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_PAREN_R, .name = STRING(")")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_PUNCTUATOR, .id = PUNC_SEMICOLON, .name = STRING(";")});

    // The operators are being appended below
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_OPERATOR, .id = OP_PLUS, .name = STRING("+")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_OPERATOR, .id = OP_EQUAL, .name = STRING("=")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_OPERATOR, .id = OP_MINUS, .name = STRING("-")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_OPERATOR, .id = OP_MULT, .name = STRING("*")});
    dynamic_array_append(&LanguageReservedWords, &(language_identifier){.type = LRES_OPERATOR, .id = OP_DIV, .name = STRING("/")});

    return 0;
}

int lexer_module_terminate(void) {
    dynamic_array_free(&LanguageReservedWords);
    return 0;
}

int lexer(DynamicArray* tokens, string* file) {
    // Very important for tokens dynamic array to actually be an array of tokens
    if (tokens->type != dynamic_array_registry_get_typeID(&STRING("token"))) {
        dynamic_array_free(tokens);
        dynamic_array_init(tokens, &STRING("token"));
    }

    // Used for figuring out where quotes are for string literals
    unsigned int quoteCount = 0;
    unsigned int prevQuoteCount = 0;
    unsigned int quoteIndices[2];

    // Keeps track of what known identifiers have been declared in the code while
    // lexing. This allows for the identification of identifiers in expressions.
    // the identifiers themselves will be determined based on variable declaration,
    // like int x, float y, or string str. Thus, after a variable keyword, identifier
    // is expected. This is the primary heuristic for determining identifiers
    DynamicArray knownIdentifiers;
    dynamic_array_init(&knownIdentifiers, &STRING("string"));

    for (int i = 0; i < file->len; i++) {
        // Looks for the string literals
        // The part with file.str[i-1] is to allow for quotes to be included in strings by the following method: \"
        if (file->str[i] == '"' && i > 0 && file->str[i - 1] != '\\') {
            quoteIndices[quoteCount % 2] = i;
            quoteCount++;

            // This means that the body of the quotes has just been exited
            if (prevQuoteCount % 2 == 1 && quoteCount % 2 == 0) {
                string literal;
                string_init(&literal);
                // Add +1 to the from argument below because otherwise quote symbol would be included
                string_substring(&literal, file, quoteIndices[0] + 1, quoteIndices[1]);
                dynamic_array_append(tokens, &(token){.literal = literal, .type = LRES_LITERAL});
            }

            // This prevents wasting time searching for keywords, identifiers, etc later on
            goto loop_exit;
        } else if (file->str[i] == ' ' || file->str[i] == '\n') {
            // Skip redudant checking by passing over newlines and spaces
            goto loop_exit;
        } else if (file->str[i] >= '0' && file->str[i] <= '9') {
            // This is where numerical literals are searched for
            int end = i;
            // The loop basically continues as long as the current character is a number or a period, since there
            // could be decimal values
            for (int j = i; j < file->len && ((file->str[j] >= '0' && file->str[j] <= '9') || file->str[j] == '.'); j++) {
                end++;
            }

            string numerical_literal;
            string_init(&numerical_literal);
            string_substring(&numerical_literal, file, i, end);
            dynamic_array_append(tokens, &(token){.type = LRES_LITERAL, .literal = numerical_literal});
            // the minus one is to account for the iteration of i by one at the end of the loop
            i = end - 1;
            goto loop_exit;
        }

        // Only want to check for keywords and identifiers if not inside a string literal
        if (quoteCount % 2 == 0) {
            for (int j = 0; j < LanguageReservedWords.len; j++) {
                language_identifier* ldent = dynamic_array_get(&LanguageReservedWords, &INDEX(j));
                if (string_compare_with_offset(file, &ldent->name, i)) {
                    //Check for comments first, as those can be skipped
                    if (ldent->type == LRES_COMMENT) {
                        if (ldent->id == COMM_DSLASH) {
                            // Account for length of double slashes by adding the length of the name of ldent
                            int comm_end = i + ldent->name.len;
                            for (int k = comm_end; k < file->len && file->str[k] != '\n'; k++) {
                                comm_end++;
                            }

                            // The minus one is to account for the fact that i is incremented by one after this
                            i = comm_end - 1;
                            goto loop_exit;
                        } 
                    }
                    // For debugging purposes, the literal part of the token will contain the string name of the keyword
                    dynamic_array_append(tokens, &(token){.type = ldent->type, .id = ldent->id, .literal = ldent->name});

                    // If the current token is a variable keyword (like int or float), then immediately begin search for the expected
                    // declaration of the identifier
                    if (ldent->type == LRES_KEYWORD && ldent->vtag == true) {
                        // Add 1 to account for expected space after variable keyword declaration: int x = 2; <- note the space between int and x
                        int start = i + ldent->name.len + 1;
                        int ends[3];
                        // The three possible characters that could delimate the end of a variable declaration are a space, an equal sign,
                        // or a semicolon. Unfortunately, these have to be hard coded in here
                        ends[0] = string_find_with_offset(file, &STRING(" "), start);
                        ends[1] = string_find_with_offset(file, &STRING("="), start);
                        ends[2] = string_find_with_offset(file, &STRING(";"), start);
                        // Whichever comes first is what will delimate the identifier
                        int end = MIN(MIN(ends[0], ends[1]), ends[2]);

                        string s1, s2;
                        string_init(&s1);
                        string_init(&s2);
                        string_substring(&s1, file, start, end);
                        string_copy(&s2, &s1);

                        dynamic_array_append(tokens, &(token){.type = LRES_IDENTIFIER, .literal = s1});
                        dynamic_array_append(&knownIdentifiers, &s2);

                        // Allows for skipping of redudant checks
                        // the minus one is to account for the iteration of i by one at the end of the loop
                        i = end - 1;
                        goto loop_exit;
                    }

                    // This allows for the skipping of redudant checks for keywords when one has already been found at the current location
                    // The minus one is to account for the fact that i will be iterated by one at the end of this loop
                    i += ldent->name.len - 1;
                    goto loop_exit;
                }
            }

            // Searches for the known identifiers (the ones that have already been declared)
            for (int j = 0; j < knownIdentifiers.len; j++) {
                string* identifier = dynamic_array_get(&knownIdentifiers, &INDEX(j));
                if (string_compare_with_offset(file, identifier, i)) {
                    string s3;
                    string_init(&s3);
                    string_copy(&s3, identifier);
                    dynamic_array_append(tokens, &(token){.type = LRES_IDENTIFIER, .literal = s3});

                    // This allows for the lexer to skip over redudant checks
                    // the minus one is to account for the iteration of i by one at the end of the loop
                    i += s3.len - 1;
                    goto loop_exit;
                }
            }
        }

    loop_exit:
        prevQuoteCount = quoteCount;
    }

    dynamic_array_free(&knownIdentifiers);
    return 0;
}
