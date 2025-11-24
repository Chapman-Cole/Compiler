#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    dynamic_array_registry_init();

    string str;
    string_init(&str);

    string_read_file(&str, &STRING("test.txt"));
    printf("%s\n", str.str);

    string_free(&str);
    dynamic_array_registry_terminate();
    return 0;
}
