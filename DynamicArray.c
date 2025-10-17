#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// There is no need to free this memory because it is intended to last for the
// entire lifetime of the program
DynamicArrayType* typeRegistry = NULL;
unsigned int typeRegistryLen = 0;

int dynamic_array_registry_type_append(string* type, bool passByValue) {
    typeRegistry = (DynamicArrayType*)realloc(typeRegistry, (typeRegistryLen + 1) * sizeof(DynamicArrayType));
    if (typeRegistry == NULL) {
        printf("Failed to initialize/append type registry for DynamicArray type.");
        exit(-1);
    }

    // Can't forget to register string before using it with other functions
    STRING_INIT(typeRegistry[typeRegistryLen].type);
    typeRegistry[typeRegistryLen].typeID = typeRegistryLen;
    typeRegistry[typeRegistryLen].passByValue = passByValue;
    string_copy(&typeRegistry[typeRegistryLen].type, type);

    typeRegistryLen++;
    return 0;
}

int dynamic_array_registry_setup(void) {
    dynamic_array_registry_type_append(&STRING("char"), true);
    dynamic_array_registry_type_append(&STRING("unsigned char"), true);
    dynamic_array_registry_type_append(&STRING("short"), true);
    dynamic_array_registry_type_append(&STRING("unsigned short"), true);
    dynamic_array_registry_type_append(&STRING("int"), true);
    dynamic_array_registry_type_append(&STRING("unsigned int"), true);
    dynamic_array_registry_type_append(&STRING("long"), true);
    dynamic_array_registry_type_append(&STRING("unsigned long"), true);
    dynamic_array_registry_type_append(&STRING("long long"), true);
    dynamic_array_registry_type_append(&STRING("unsigned long long"), true);
    dynamic_array_registry_type_append(&STRING("bool"), true);

    // Since float data is being passed through a union, it actually will be treated like a struct
    dynamic_array_registry_type_append(&STRING("float"), false);
    dynamic_array_registry_type_append(&STRING("double"), false);
    dynamic_array_registry_type_append(&STRING("long double"), false);

    return 0;
}

int dynamic_array_init(DynamicArray* arr, size_t element_size, string* type) {
    arr->buf = NULL;
    arr->len = 0;
    arr->__memsize = 1;
    arr->element_size = element_size;
    arr->type = dynamic_array_registry_get_typeID(type);
    return 0;
}

unsigned int dynamic_array_registry_get_typeID(string* type) {
    // Linear search should be just fine since I can't imagine their being enough types added to the registry
    // that linear search becomes significantly inefficient
    for (int i = 0; i < typeRegistryLen; i++) {
        if (string_compare(&typeRegistry[i].type, type) == true) {
            return typeRegistry[i].typeID;
        }
    }

    // This indicates that the given type string was not found in the type
    // registry
    return -1;
}

int dynamic_array_free(DynamicArray* arr) {
    free(arr->buf);
    arr->buf = NULL;
    arr->len = 0;
    arr->__memsize = 1;
    arr->element_size = 1;
    return 0;
}

int dynamic_array_append(DynamicArray* arr, void* data) {
    if (arr->len + 1 >= arr->__memsize) {
        arr->__memsize *= 2;
        void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

        if (test == NULL) {
            printf("Failed to allocate memory in dynamic_array_append\n");
            exit(-1);
        }

        arr->buf = test;
    }

    if (typeRegistry[arr->type].passByValue == true) {
        // Since in this case the value of the void* itself is the data we want, we pass a pointer to the void* that is then
        // used by the memcpy function to copy the specified number of bytes from the void* into the array
        memcpy(arr->buf + (arr->len * arr->element_size), &data, arr->element_size);
        arr->len++;
    } else {
        // This copies the raw bytes of the struct, union, or enum pointed to by data into the array
        memcpy(arr->buf + (arr->len * arr->element_size), data, arr->element_size);
        arr->len++;
    }

    return 0;
}
