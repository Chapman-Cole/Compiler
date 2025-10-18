#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

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

int dynamic_array_pop(DynamicArray* arr) {
    if (arr->len - 1 < arr->__memsize / 2) {
        arr->__memsize /= 2;
        void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

        if (test == NULL) {
            printf("Failed to allocate memory in dynamic_array_pop\n");
            exit(-1);
        }

        arr->buf = test;
    }

    arr->len--;

    return 0;
}

int dynamic_array_insert(DynamicArray* arr, void* data, unsigned int index) {
    if (index < arr->len) {
        if (arr->len + 1 >= arr->__memsize) {
            arr->__memsize *= 2;
            void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_insert\n");
                exit(-1);
            }

            arr->buf = test;
        }

        DynamicArray end;
        dynamic_array_init(&end, arr->element_size, &typeRegistry[arr->type].type);
        dynamic_array_subset(&end, arr, index, arr->len);
        
        if (typeRegistry[arr->type].passByValue == true) {
            memcpy(arr->buf + (index * arr->element_size), &data, arr->element_size);
            memcpy(arr->buf + ((index + 1) * arr->element_size), end.buf, end.len * end.element_size);
        } else {
            memcpy(arr->buf + (index * arr->element_size), data, arr->element_size);
            memcpy(arr->buf + ((index + 1) * arr->element_size), end.buf, end.len * end.element_size);
        }
        
        arr->len++;

        dynamic_array_free(&end);

        return 0;
    } else {
        printf("Index Out of Bounds warning in dynamic_array_insert\n");
        return -1;
    }
}

int dynamic_array_remove(DynamicArray* arr, unsigned int index) {
    if (index < arr->len) {
        if (arr->len - 1 < arr->__memsize / 2) {
            arr->__memsize /= 2;
            void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_remove\n");
                exit(-1);
            }

            arr->buf = test;
        }

        // If the index is the last index in the list, then simply decrement the length of the array
        if (index == arr->len - 1) {
            arr->len--;
            return 0;
        }

        DynamicArray end;
        dynamic_array_init(&end, arr->element_size, &typeRegistry[arr->type].type);
        dynamic_array_subset(&end, arr, index + 1, arr->len);

        memcpy(arr->buf + (index * arr->element_size), end.buf, end.len * end.element_size);

        arr->len--;

        dynamic_array_free(&end);

        return 0;
    } else {
        printf("Index Out of Bounds warning in dynamic_array_insert\n");
        return -1;
    }
}

// Note: this function assumes that the dynamic arrays both have the same types, so it is important
// that you verify that before calling this function, otherwise memory leaks could happen
int dynamic_array_subset(DynamicArray* dest, DynamicArray* src, unsigned int from, unsigned int to) {
    if (from < to && to <= src->len) {
        if (dest->__memsize < to - from) {
            dest->__memsize = to - from + 1;
            void* test = (void*)realloc(dest->buf, dest->__memsize * dest->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_subset\n");
                exit(-1);
            }

            dest->buf = test;
        }

        memcpy(dest->buf, src->buf + (from * src->element_size), (to - from) * src->element_size);
        dest->len = to - from;

        return 0;
    } else if (to - from == 0) {
        return 0;
    } else {
        printf("dynamic_array_subset indices were incorrect, so function did nothing\nfrom = %d | to = %d | src->len = %d\n", from, to, src->len);
        return -1;
    }
}
