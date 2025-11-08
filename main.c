#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

// This is required to be able to access these variables
DYNAMIC_ARRAY_REGISTRY_ENABLE

int main(void) {
    dynamic_array_registry_init();

    DynamicArray test;
    dynamic_array_init(&test, &STRING("DynamicArray"));
    dynamic_array_create_nDimensions(&test, &STRING("int"), 1, 10);

    for (int i = 0; i < 10; i++) {
        dynamic_array_append(&test, &INT(i));
    }

    for (int i = 0; i < test.len; i++) {
        printf("%d ", *(int*)dynamic_array_get(&test, 1, i));
    }

    dynamic_array_free(&test);
    dynamic_array_registry_terminate();
    return 0;
}
