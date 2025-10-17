#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>

// clang main.c Strings.c DynamicArray.c -o main

// This is required to be able to access these variables
DYNAMIC_ARRAY_REGISTRY_ENABLE

int main(void) {
    dynamic_array_registry_setup();
    dynamic_array_registry_type_append(&STRING("DynamicArray"), false);

    printf("Registry Table:\n");
    for (int i = 0; i < typeRegistryLen; i++) {
        printf("Type: %20s | ID: %d\n", typeRegistry[i].type.str, typeRegistry[i].typeID);
    }
    printf("\n");

    DynamicArray arr, arr2;
    dynamic_array_init(&arr, sizeof(int), &STRING("int"));
    dynamic_array_init(&arr2, sizeof(float), &STRING("float"));

    dynamic_array_append(&arr, (void*)67);
    dynamic_array_append(&arr, (void*)82);
    dynamic_array_append(&arr, (void*)23);
    dynamic_array_append(&arr, (void*)22);

    dynamic_array_append(&arr2, &FLOAT(2.31f));
    dynamic_array_append(&arr2, &FLOAT(4.67f));
    dynamic_array_append(&arr2, &FLOAT(3.26f));
    dynamic_array_append(&arr2, &FLOAT(3.14f));
    dynamic_array_append(&arr2, &FLOAT(9.81f));
    dynamic_array_append(&arr2, &FLOAT(7.77f));
    dynamic_array_append(&arr2, &FLOAT(5.78f));

    for (int i = 0; i < arr.len; i++) {
        printf("%d, ", ((int*)(arr.buf))[i]);
    }
    printf("\n\n");

    for (int i = 0; i < arr2.len; i++) {
        printf("%0.2f, ", ((float*)(arr2.buf))[i]);
    }
    printf("\n");

    dynamic_array_free(&arr);
    dynamic_array_free(&arr2);
    return 0;
}
