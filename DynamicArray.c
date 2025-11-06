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

int string_deallocator(void* str) {
    STRING_FREE(*((string*)str));
    return 0;
}

int dynamic_array_registry_type_append(string* type, int (*deallocator)(void*)) {
    typeRegistry = (DynamicArrayType*)realloc(typeRegistry, (typeRegistryLen + 1) * sizeof(DynamicArrayType));
    if (typeRegistry == NULL) {
        printf("Failed to initialize/append type registry for DynamicArray type.");
        exit(-1);
    }

    // Can't forget to register string before using it with other functions
    STRING_INIT(typeRegistry[typeRegistryLen].type);
    typeRegistry[typeRegistryLen].typeID = typeRegistryLen;
    string_copy(&typeRegistry[typeRegistryLen].type, type);
    typeRegistry[typeRegistryLen].deallocator = deallocator;

    typeRegistryLen++;
    return 0;
}

int dynamic_array_registry_init(void) {
    dynamic_array_registry_type_append(&STRING("char"), NULL);
    dynamic_array_registry_type_append(&STRING("unsigned char"), NULL);
    dynamic_array_registry_type_append(&STRING("short"), NULL);
    dynamic_array_registry_type_append(&STRING("unsigned short"), NULL);
    dynamic_array_registry_type_append(&STRING("int"), NULL);
    dynamic_array_registry_type_append(&STRING("unsigned int"), NULL);
    dynamic_array_registry_type_append(&STRING("long"), NULL);
    dynamic_array_registry_type_append(&STRING("unsigned long"), NULL);
    dynamic_array_registry_type_append(&STRING("long long"), NULL);
    dynamic_array_registry_type_append(&STRING("unsigned long long"), NULL);
    dynamic_array_registry_type_append(&STRING("bool"), NULL);

    // Since float data is being passed through a union, it actually will be treated like a struct
    dynamic_array_registry_type_append(&STRING("float"), NULL);
    dynamic_array_registry_type_append(&STRING("double"), NULL);
    dynamic_array_registry_type_append(&STRING("long double"), NULL);

    // The function pointer for the deallocation of a dynamic array is already handled by a defined function, so it can
    // just be NULL here
    dynamic_array_registry_type_append(&STRING("DynamicArray"), dynamic_array_deallocator);
    dynamic_array_registry_type_append(&STRING("string"), string_deallocator);

    return 0;
}

int dynamic_array_registry_terminate(void) {
    for (int i = 0; i < typeRegistryLen; i++) {
        STRING_FREE(typeRegistry[i].type);
        typeRegistry[i].deallocator = NULL;
    }

    free(typeRegistry);
    typeRegistry = NULL;
    typeRegistryLen = 0;
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

// If the freeElements variable is set to true, then the function will recursively search the elements of the
// dynamic_array to search for potential sublists, and free those first and then work its way back up the chain
// it also accounts for the possibility of strings, which are a special case
int dynamic_array_free(DynamicArray* arr) {
    if (arr->type == dynamic_array_registry_get_typeID(&STRING("DynamicArray"))) {
        for (int i = 0; i < arr->len; i++) {
            dynamic_array_free(&((DynamicArray*)arr->buf)[i]);
        }
    } else if (typeRegistry[arr->type].deallocator != NULL) {
        // This will only run if the type requires a special deallocation function on each element, mainly if each
        // type has a pointer within itself that needs to be handled
        for (int i = 0; i < arr->len; i++) {
            typeRegistry[arr->type].deallocator((void*)((char*)arr->buf + (i * arr->element_size)));
        }
    }

    free(arr->buf);
    arr->buf = NULL;
    arr->len = 0;
    arr->__memsize = 1;
    arr->element_size = 1;
    return 0;
}

int dynamic_array_append(DynamicArray* arr, void* data) {
    if (arr->len + 1 >= arr->__memsize) {
        arr->__memsize = 2 * arr->__memsize + 1;
        void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

        if (test == NULL) {
            printf("Failed to allocate memory in dynamic_array_append\n");
            exit(-1);
        }

        arr->buf = test;
    }

    // This copies the raw bytes of the struct, union, or enum pointed to by data into the array
    // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
    // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
    memcpy((char*)arr->buf + (arr->len * arr->element_size), data, arr->element_size);
    arr->len++;

    return 0;
}

int dynamic_array_pop(DynamicArray* arr) {
    if (arr->len > 0) {
        if (arr->len - 1 < arr->__memsize / 2) {
            arr->__memsize /= 2;

            void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_pop\n");
                exit(-1);
            }

            arr->buf = test;
        }

        // Arrays of arrays or arrays of structs with pointers may require special deallocation functions.
        // This is here to account for that possibility
        if (typeRegistry[arr->type].deallocator != NULL) {
            typeRegistry[arr->type].deallocator((void*)((char*)arr->buf + ((arr->len - 1) * arr->element_size)));
        }

        arr->len--;
    }

    return 0;
}

int dynamic_array_insert(DynamicArray* arr, void* data, unsigned int index) {
    if (index < arr->len) {
        if (arr->len + 1 >= arr->__memsize) {
            arr->__memsize = 2 * arr->__memsize + 1;
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

        // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
        // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
        memcpy((char*)arr->buf + (index * arr->element_size), data, arr->element_size);
        memcpy((char*)arr->buf + ((index + 1) * arr->element_size), end.buf, end.len * end.element_size);

        arr->len++;

        dynamic_array_free(&end);

        return 0;
    } else {
        printf("Index Out of Bounds warning in dynamic_array_insert\n");
        return -1;
    }
}

int dynamic_array_remove(DynamicArray* arr, unsigned int index) {
    if (arr->len > 0) {
        if (index < arr->len && index >= 0) {
            if (arr->len - 1 < arr->__memsize / 2) {
                arr->__memsize /= 2;
                void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

                if (test == NULL) {
                    printf("Failed to allocate memory in dynamic_array_remove\n");
                    exit(-1);
                }

                arr->buf = test;
            }

            // Arrays of arrays or arrays of structs with pointers may require special deallocation functions.
            // This is here to account for that possibility
            if (typeRegistry[arr->type].deallocator != NULL) {
                typeRegistry[arr->type].deallocator((void*)((char*)arr->buf + (index * arr->element_size)));
            }

            // If the index is the last index in the list, then simply decrement the length of the array
            if (index == arr->len - 1) {
                arr->len--;
                return 0;
            }

            DynamicArray end;
            dynamic_array_init(&end, arr->element_size, &typeRegistry[arr->type].type);
            dynamic_array_subset(&end, arr, index + 1, arr->len);

            // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
            // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
            memcpy((char*)arr->buf + (index * arr->element_size), end.buf, end.len * end.element_size);

            arr->len--;

            dynamic_array_free(&end);

            return 0;
        } else {
            printf("Index Out of Bounds warning in dynamic_array_insert\n");
            return -1;
        }
    } else {
        return 0;
    }
}

// Note: this function assumes that the dynamic arrays both have the same types, so it is important
// that you verify that before calling this function, otherwise memory leaks could happen
int dynamic_array_subset(DynamicArray* dest, DynamicArray* src, unsigned int from, unsigned int to) {
    if (from < to && to <= src->len) {
        if (dest->__memsize <= to - from) {
            dest->__memsize = to - from + 1;
            void* test = (void*)realloc(dest->buf, dest->__memsize * dest->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_subset\n");
                exit(-1);
            }

            dest->buf = test;
        }

        // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
        // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
        memcpy(dest->buf, (char*)src->buf + (from * src->element_size), (to - from) * src->element_size);
        dest->len = to - from;

        return 0;
    } else if (to - from == 0) {
        return 0;
    } else {
        printf("dynamic_array_subset indices were incorrect, so function did nothing\nfrom = %d | to = %d | src->len = %d\n", from, to, src->len);
        return -1;
    }
}

// Note: for this function it is ok if the index variable is equal to the legnth of the dest array
int dynamic_array_insert_array(DynamicArray* dest, DynamicArray* src, unsigned int index) {
    if (index <= dest->len) {
        if (dest->__memsize < dest->len + src->len) {
            dest->__memsize = dest->len + src->len;

            void* test = (void*)realloc(dest->buf, dest->__memsize * dest->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_insert_array\n");
                exit(-1);
            }

            dest->buf = test;
        }

        DynamicArray end;
        dynamic_array_init(&end, dest->element_size, &DYNAMIC_ARRAY_TYPE(dest->type));
        dynamic_array_subset(&end, dest, index, dest->len);

        // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
        // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
        memcpy((char*)dest->buf + (index * dest->element_size), src->buf, src->len * src->element_size);
        memcpy((char*)dest->buf + ((index + src->len) * dest->element_size), end.buf, end.len * end.element_size);

        dynamic_array_free(&end);
        dest->len += src->len;

        return 0;
    } else {
        printf("Index out of bounds error in dynamic_array_insert_array\n");
        return -1;
    }
}

int dynamic_array_remove_selection(DynamicArray* arr, unsigned int from, unsigned int to) {
    if (from < to && to <= arr->len && from >= 0) {
        // First, we have to make sure that all of the data being removed is safely deallocated if the special
        // deallocation function is required

        if (typeRegistry[arr->type].deallocator != NULL) {
            // Arrays of arrays or arrays of structs with pointers may require special deallocation functions.
            // This is here to account for that possibility
            for (int i = from; i < to; i++) {
                typeRegistry[arr->type].deallocator((void*)((char*)arr->buf + (i * arr->element_size)));
            }
        }

        DynamicArray end;
        dynamic_array_init(&end, arr->element_size, &DYNAMIC_ARRAY_TYPE(arr->type));
        dynamic_array_subset(&end, arr, to, arr->len);

        // Cast the void* to a char* in order to get around pointer arithmetic being disallowed with void*
        // The casting to a char* also allows for byte scaling, which is what is desired when using the memcpy function
        memcpy((char*)arr->buf + (from * arr->element_size), end.buf, end.len * end.element_size);

        arr->len -= to - from;

        if (arr->len < arr->__memsize / 2) {
            arr->__memsize /= 2;

            void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

            if (test == NULL) {
                printf("Failed to allocate memory in dynamic_array_remove_selection\n");
                exit(-1);
            }

            arr->buf = test;
        }

        dynamic_array_free(&end);

        return 0;
    } else if (from == to) {
        // Do nothing since the range is 0
        return 0;
    } else {
        printf("dynamic_array_remove_selection index range error\n");
        return -1;
    }
}

// Finish later
void* dynamic_array_get(DynamicArray* arr, int dimensions, ...) {
    va_list args;
    va_start(args, dimensions);

    for (int i = 0; i < dimensions; i++) {
    }

    va_end(args);

    return NULL;
}

int dynamic_array_deallocator(void* arr) {
    dynamic_array_free((DynamicArray*)arr);
    return 0;
}
