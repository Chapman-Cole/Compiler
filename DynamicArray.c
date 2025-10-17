#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The max index (inclusive) of the fundamental base data types in c in the typeRegistry array
// Note: this number may need to be updated if there is a fundamental data type I forgot to consider
#define FUNDAMENTAL_DATA_TYPE_MAX_INDEX 10

// There is no need to free this memory because it is intended to last for the
// entire lifetime of the program
DynamicArrayType* typeRegistry = NULL;
unsigned int typeRegistryLen = 0;

int dynamic_array_registry_type_append(string* type) {
    typeRegistry = (DynamicArrayType*)realloc(typeRegistry, (typeRegistryLen + 1) * sizeof(DynamicArrayType));
    if (typeRegistry == NULL) {
        printf("Failed to initialize/append type registry for DynamicArray type.");
        exit(-1);
    }

    // Can't forget to register string before using it with other functions
    STRING_INIT(typeRegistry[typeRegistryLen].type);
    typeRegistry[typeRegistryLen].typeID = typeRegistryLen;
    string_copy(&typeRegistry[typeRegistryLen].type, type);

    typeRegistryLen++;
    return 0;
}

int dynamic_array_registry_setup(void) {
    dynamic_array_registry_type_append(&STRING("char"));
    dynamic_array_registry_type_append(&STRING("unsigned char"));
    dynamic_array_registry_type_append(&STRING("short"));
    dynamic_array_registry_type_append(&STRING("unsigned short"));
    dynamic_array_registry_type_append(&STRING("int"));
    dynamic_array_registry_type_append(&STRING("unsigned int"));
    dynamic_array_registry_type_append(&STRING("long"));
    dynamic_array_registry_type_append(&STRING("unsigned long"));
    dynamic_array_registry_type_append(&STRING("long long"));
    dynamic_array_registry_type_append(&STRING("unsigned long long"));
    dynamic_array_registry_type_append(&STRING("bool"));

    // Since float data is being passed through a union, it actually will be treated like a struct
    dynamic_array_registry_type_append(&STRING("float"));
    dynamic_array_registry_type_append(&STRING("double"));
    dynamic_array_registry_type_append(&STRING("long double"));

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

    // If the data type is fundamental, then simply load into the array by the value of the bits stored in data parameter
    // else use memcmp to transfer data stored at the pointer into the array
    if (arr->type <= FUNDAMENTAL_DATA_TYPE_MAX_INDEX) {
        memcpy(arr->buf + (arr->len * arr->element_size), &data, arr->element_size);
        arr->len++;
    } else {
        memcpy(arr->buf + (arr->len * arr->element_size), data, arr->element_size);
        arr->len++;
    }

    return 0;
}
