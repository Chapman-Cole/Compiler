#include "DynamicArray.h"
#include "Strings.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// There is no need to free this memory because it is intended to last for the
// entire lifetime of the program
unsigned int typeRegistryLen = 0;
unsigned int typeRegistryMemsize = 1;
DynamicArrayType* typeRegistry = NULL;

int string_deallocator(void* str) {
    string_free((string*)str);
    return 0;
}

int dynamic_array_registry_type_append(string* type, int (*deallocator)(void*), unsigned int size) {
    if (typeRegistryLen + 1 >= typeRegistryMemsize) {
        // The typeRegistryMemsize will increase by 5 each time since I only expect the number of times to grow roughly linearly 
        typeRegistryMemsize += 5;
        typeRegistry = (DynamicArrayType*)realloc(typeRegistry, typeRegistryMemsize * sizeof(DynamicArrayType));
        if (typeRegistry == NULL) {
            printf("Failed to initialize/append type registry for DynamicArray type.");
            exit(-1);
        }
    }

    // Can't forget to register string before using it with other functions
    string_init(&typeRegistry[typeRegistryLen].type);
    typeRegistry[typeRegistryLen].typeID = typeRegistryLen;
    string_copy(&typeRegistry[typeRegistryLen].type, type);
    typeRegistry[typeRegistryLen].deallocator = deallocator;
    typeRegistry[typeRegistryLen].size = size;

    typeRegistryLen++;
    return 0;
}

int dynamic_array_registry_init(void) {
    dynamic_array_registry_type_append(&STRING("char"), NULL, sizeof(char));
    dynamic_array_registry_type_append(&STRING("unsigned char"), NULL, sizeof(unsigned char));
    dynamic_array_registry_type_append(&STRING("short"), NULL, sizeof(short));
    dynamic_array_registry_type_append(&STRING("unsigned short"), NULL, sizeof(unsigned short));
    dynamic_array_registry_type_append(&STRING("int"), NULL, sizeof(int));
    dynamic_array_registry_type_append(&STRING("unsigned int"), NULL, sizeof(unsigned int));
    dynamic_array_registry_type_append(&STRING("long"), NULL, sizeof(long));
    dynamic_array_registry_type_append(&STRING("unsigned long"), NULL, sizeof(unsigned long));
    dynamic_array_registry_type_append(&STRING("long long"), NULL, sizeof(long long));
    dynamic_array_registry_type_append(&STRING("unsigned long long"), NULL, sizeof(unsigned long long));
    dynamic_array_registry_type_append(&STRING("bool"), NULL, sizeof(bool));

    // Since float data is being passed through a union, it actually will be treated like a struct
    dynamic_array_registry_type_append(&STRING("float"), NULL, sizeof(float));
    dynamic_array_registry_type_append(&STRING("double"), NULL, sizeof(double));
    dynamic_array_registry_type_append(&STRING("long double"), NULL, sizeof(long double));

    // The function pointer for the deallocation of a dynamic array is already handled by a defined function, so it can
    // just be NULL here
    dynamic_array_registry_type_append(&STRING("DynamicArray"), dynamic_array_deallocator, sizeof(DynamicArray));
    dynamic_array_registry_type_append(&STRING("string"), string_deallocator, sizeof(string));

    return 0;
}

int dynamic_array_registry_terminate(void) {
    for (int i = 0; i < typeRegistryLen; i++) {
        string_free(&typeRegistry[i].type);
        typeRegistry[i].deallocator = NULL;
    }

    free(typeRegistry);
    typeRegistry = NULL;
    typeRegistryLen = 0;
    return 0;
}

int dynamic_array_init(DynamicArray* arr, string* type) {
    arr->buf = NULL;
    arr->len = 0;
    arr->__memsize = 1;
    arr->type = dynamic_array_registry_get_typeID(type);
    arr->element_size = DYNAMIC_ARRAY_TYPE_SIZE(arr->type);
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
        dynamic_array_init(&end, &typeRegistry[arr->type].type);
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
            dynamic_array_init(&end, &typeRegistry[arr->type].type);
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
        dynamic_array_init(&end, &DYNAMIC_ARRAY_TYPE(dest->type));
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
        dynamic_array_init(&end, &DYNAMIC_ARRAY_TYPE(arr->type));
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

void* dynamic_array_get(DynamicArray* arr, DynamicArray* indices) {
    unsigned int dynamic_array_type = dynamic_array_registry_get_typeID(&STRING("DynamicArray"));

    DynamicArray* temp = arr;
    for (int i = 0; i < indices->len - 1; i++) {
        if (temp->type == dynamic_array_type) {
            int index = ((int*)indices->buf)[i];
            if (index >= 0 && index < temp->len) {
                temp = (DynamicArray*)((char*)temp->buf + (index * temp->element_size));
            } else {
                printf("Index Out of Bounds error in dynamic_array_type");
                return NULL;
            }
        } else {
            printf("Number of dimensions specified is too large for dynamic_array_get\n");
            return NULL;
        }
    }

    int index = ((int*)indices->buf)[indices->len - 1];
    void* answer = NULL;
    if (index >= 0 && index < temp->len) {
        answer = (void*)((char*)temp->buf + (index * temp->element_size));
    } else {
        printf("Index Out of Bounds error in dynamic_array_get\n");
        return NULL;
    }

    return answer;
}

int dynamic_array_set(DynamicArray* arr, DynamicArray* indices, void* data) {
    unsigned int dynamic_array_type = dynamic_array_registry_get_typeID(&STRING("DynamicArray"));

    DynamicArray* temp = arr;
    for (int i = 0; i < indices->len - 1; i++) {
        if (temp->type == dynamic_array_type) {
            int index = ((int*)indices->buf)[i];
            if (index >= 0 && index < temp->len) {
                temp = (DynamicArray*)((char*)temp->buf + (index * temp->element_size));
            } else {
                printf("Index out of bounds error in dynamic_array_set\n");
                return -1;
            }
        } else {
            printf("The number of dimensions specified is too large for dynamic_array_set\n");
            return -1;
        }
    }

    int index = ((int*)indices->buf)[indices->len - 1];
    if (index >= 0 && index < temp->len) {
        memcpy((char*)temp->buf + (index * temp->element_size), data, temp->element_size);
    } else {
        printf("Index out of bounds error in dynamic_array_set\n");
        return -1;
    }

    return 0;
}

int dynamic_array_deallocator(void* arr) {
    dynamic_array_free((DynamicArray*)arr);
    return 0;
}

int dynamic_array_init_nDimensions(DynamicArray* arr, string* type, DynamicArray* dimensions) {
    if (dimensions->len == 1) {
        dynamic_array_init(arr, type);
        dynamic_array_resize(arr, ((int*)dimensions->buf)[0], true);
    } else {
        dynamic_array_init(arr, &STRING("DynamicArray"));
        dynamic_array_resize(arr, ((int*)dimensions->buf)[0], true);

        for (int i = 0; i < arr->len; i++) {
            DynamicArray* temp = (DynamicArray*)dynamic_array_get(arr, &INDEX(i));
            if (temp == NULL) {
                printf("Failed to initialize ndimensional array\n");
                exit(-1);
            }

            // This will recursively initialize the arrays until the final escape condition is met.
            // While doing so, it increments the dimensions pointer by 1 and decrements the dimensionsLen
            // by one as well
            dynamic_array_init_nDimensions(temp, type, &(DynamicArray){.len = dimensions->len - 1, .buf = ((int*)dimensions->buf) + 1, .element_size = sizeof(int), .type = 4, .__memsize = 0});
        }
    }
    return 0;
}

int dynamic_array_resize(DynamicArray* arr, unsigned int size, bool updateLen) {
    if (size < arr->len) {
        while (arr->len < size) {
            dynamic_array_pop(arr);
        }
        arr->__memsize = size;
        void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

        if (test == NULL) {
            printf("Failed to allocate memory in dynamic_array_resize\n");
            exit(-1);
        }

        arr->buf = test;
        if (updateLen == true) {
            arr->len = size;
        }
    } else {
        arr->__memsize = size;
        void* test = (void*)realloc(arr->buf, arr->__memsize * arr->element_size);

        if (test == NULL) {
            printf("Failed to allocate memory in dynamic_array_resize\n");
            exit(-1);
        }

        arr->buf = test;
        if (updateLen == true) {
            arr->len = size;
        }
    }

    return 0;
}
