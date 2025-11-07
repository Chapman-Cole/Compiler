#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

// This is required to be able to access these variables
DYNAMIC_ARRAY_REGISTRY_ENABLE

int main(void) {
    dynamic_array_registry_init();

    DynamicArray usrArrays;
    dynamic_array_init(&usrArrays, sizeof(DynamicArray), &STRING("DynamicArray"));

    string input;
    STRING_INIT(input);

    DynamicArray arrayNames;
    dynamic_array_init(&arrayNames, sizeof(string), &STRING("string"));

    bool should_continue = true;
    while (should_continue) {
        printf(
            "\033[2J\033[1;1HPlease Enter One of the Following Options\n"
            "1: Create a New Array\n"
            "2: Delete an Array\n"
            "3: Print Array List\n"
            "q: Quit the program\n");

        string_read_console(&input);

        // Only look at first character of the input
        switch (input.str[0]) {
        case 'q':
            should_continue = false;
            break;
        case '1':
            printf("\033[2J\033[1;1HPlease Enter the name of the array: ");
            // Don't free string since its str pointer will be incorporated into arrayNames dynamic array
            string name;
            STRING_INIT(name);
            string_read_console(&name);
            dynamic_array_append(&arrayNames, &name);
            break;
        case '2':
            printf("\033[2J\033[1;1HArray List (%d elements):\n", arrayNames.len);
            for (int i = 0; i < arrayNames.len; i++) {
                //printf("%s\n", ((string*)(arrayNames.buf))[i].str);
                printf("%s\n", ((string*)dynamic_array_get(&arrayNames, 1, i))->str);
            }

            printf("\nPlease enter the name of the array to be deleted: ");
            string_read_console(&input);

            break;
        case '3':
            printf("\033[2J\033[1;1HArray List (%d elements):\n", arrayNames.len);
            for (int i = 0; i < arrayNames.len; i++) {
                //printf("%s\n", ((string*)(arrayNames.buf))[i].str);
                printf("%s\n", ((string*)dynamic_array_get(&arrayNames, 1, i))->str);
            }

            // Wait for user to type something to move on
            string_read_console(&input);
            break;
        default:
            continue;
        }
    }

    STRING_FREE(input);

    dynamic_array_free(&arrayNames);
    dynamic_array_free(&usrArrays);

    dynamic_array_registry_terminate();
    return 0;
}
