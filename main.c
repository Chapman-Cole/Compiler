#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

// This is required to be able to access these variables
DYNAMIC_ARRAY_REGISTRY_ENABLE

int main(void) {
    dynamic_array_registry_setup();

    DynamicArray usrArrays;
    dynamic_array_init(&usrArrays, sizeof(DynamicArray), &STRING("DynamicArray"));

    string input;
    STRING_INIT(input);
    
    bool should_continue = true;
    while (should_continue) {
        printf(
            "\n\nPlease Enter One of the Following Options\n"
            "1: Create a New Array\n"
            "2: Delete an Array\n"
            "3: Print Array List\n"
            "q: Quit the program\n"
        );
        
        string_read_console(&input);
        
        //Only look at first character of the input
        switch (input.str[0]) {
            case 'q':
                should_continue = false;
                break;
            case '1':
                printf("Please Enter the name of the array: ");
                string name;
                STRING_INIT(name);
                string_read_console(&name);
                printf("\nInput: %s\n", name.str);
                STRING_FREE(name);
                break;
            case '2':
                break;
            case '3':
                break;
            default:
                continue;
        }

        
    }

    STRING_FREE(input);

    dynamic_array_free(&usrArrays);
    return 0;
}
