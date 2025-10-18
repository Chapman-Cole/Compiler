#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>

// clang main.c Strings.c DynamicArray.c -o main

// This is required to be able to access these variables
DYNAMIC_ARRAY_REGISTRY_ENABLE

int main(void) {
    dynamic_array_registry_setup();
    dynamic_array_registry_type_append(&STRING("DynamicArray"), false);

    DynamicArray arr, arr2, arr3;
    dynamic_array_init(&arr, sizeof(int), &STRING("int"));
    dynamic_array_init(&arr2, sizeof(float), &STRING("float"));
    dynamic_array_init(&arr3, sizeof(DynamicArray), &STRING("DynamicArray"));

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

    dynamic_array_pop(&arr);
    dynamic_array_insert(&arr, (void*)128, 1);
    dynamic_array_insert(&arr, (void*)799, 2);
    dynamic_array_remove(&arr, 2);
    dynamic_array_remove(&arr, arr.len - 1);
    dynamic_array_remove(&arr, 0);
    dynamic_array_insert(&arr, (void*)69, 0);

    dynamic_array_append(&arr3, &arr);
    dynamic_array_append(&arr3, &arr2);
    
    printf("type: %s\n", DYNAMIC_ARRAY_TYPE(arr3.type).str);
    printf("type: %s | ", DYNAMIC_ARRAY_TYPE(arr.type).str);
    for (int i = 0; i < arr.len; i++) {
        printf("%d, ", ((int*)(((DynamicArray*)(arr3.buf))[0]).buf)[i]);
    }
    printf("\ntype: %s | ", DYNAMIC_ARRAY_TYPE(arr2.type).str);
    for (int i = 0; i < arr2.len; i++) {
        printf("%.2f, ", ((float*)(((DynamicArray*)(arr3.buf))[1]).buf)[i]);
    }
    printf("\n");

    dynamic_array_free(&arr);
    dynamic_array_free(&arr2);
    dynamic_array_free(&arr3);
    return 0;
}
