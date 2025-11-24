#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include "Strings.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct DynamicArray {
    // The buffer itself
    void* buf;
    // The current number of elements in use
    unsigned int len;
    // The actual number of elements allocated (number of bytes would be __memsize * __datsize)
    unsigned int __memsize;
    // Used to know the constant scaling of memory size for each element
    size_t element_size;
    // This is used to help keep track of what type the different buffers you create are
    // It is also used for the fundamental types in c (like float, int, and so on) for the
    // append function, as those types of literals require special handling
    // Note: If you want to add your own type that is a typedef struct for example
    // you must first add it to the type registry using the associated function
    unsigned int type;
} DynamicArray;

// This is used to make passing in indices to the dynamic_array_get and dynamic_array_set functions
// nicer. Also should be used for the dyunamic_array_init_nDimensions function
#define INDEX(...) \
    (DynamicArray){.buf = (int[]){__VA_ARGS__}, .len = sizeof((int[]){__VA_ARGS__}) / sizeof(int), .element_size = sizeof(int), .__memsize = 0, .type = 4}

int dynamic_array_init(DynamicArray* arr, string* type);

int dynamic_array_free(DynamicArray* arr);

// This is specifically for use in the typeRegistry to make certain things easier
int dynamic_array_deallocator(void* arr);

// In order to make this function work with any type of data, you have to provide
// a pointer to that data so that the raw bytes can be shuffled into the array
// Note: it is assumed that the size of the data that the buffer points to
// is equal to element_size value of the struct
int dynamic_array_append(DynamicArray* arr, void* data);

// Removes the final element in the array
int dynamic_array_pop(DynamicArray* arr);

// Inserts the given data at specified index, shifting everything else after it right
int dynamic_array_insert(DynamicArray* arr, void* data, unsigned int index);

// Removes the data in the arrray at the specified index, and shifts everything else left
int dynamic_array_remove(DynamicArray* arr, unsigned int index);

// The dest dynamic array recieves the sub array from the parameter from (inclusive) to the parameter to (exclusive)
// as bounds of the src array
int dynamic_array_subset(DynamicArray* dest, DynamicArray* src, unsigned int from, unsigned int to);

// Adds all of the elements to the dest array at the specified index, pushing the elements at and after that index to the right
// It is very important for this function that the dest and src arrays have the same type
int dynamic_array_insert_array(DynamicArray* dest, DynamicArray* src, unsigned int index);

// Removes the specified selection, with from being inclusive bottom range, and to being the exclusive
// top range
int dynamic_array_remove_selection(DynamicArray* arr, unsigned int from, unsigned int to);

// Returns a pointer to the data in the specified dynamic array at the specified indices.
// Note: size should match the number of elements in the indices pointer list
// The INDEX macro makes passing in the index nicer for this function
void* dynamic_array_get(DynamicArray* arr, DynamicArray* indices);

// Takes in an index array of the specified size (the index macro should be used for the indices parameter for ease of use)
// and then a pointer to the data itself should be passed as well
int dynamic_array_set(DynamicArray* arr, DynamicArray* indices, void* data);

// This will create an n dimensional array with the specified number of dimensions
// Note: the INDEX macro can be used to make passing in dimensions easier
int dynamic_array_init_nDimensions(DynamicArray* arr, string* type, DynamicArray* dimensions);

// Resizes the array to the specified size in memory, and also updating the length of the dynamic_array
// if that is desired
int dynamic_array_resize(DynamicArray* arr, unsigned int size, bool updateLen);

typedef struct DynamicArrayType {
    string type;
    unsigned int typeID;
    // This is the function pointer to the deallocation function of the type in the dynamic array, if it needs one
    // if this function pointer is NULL, then that just means that the type doesn't have any pointers that need freeing
    // specifically, this is used for structs that might contain pointers that need to be freed when freeing the entire dynamic
    // array
    int (*deallocator)(void*);
    // Stores the memory size of the type
    unsigned int size;
} DynamicArrayType;

// This union is used to pass fundamental types in c into the dynamic_array_function, which simplifies the inner workings
// of the dynamic array functions
typedef union FundamentalType {
    char c;
    unsigned char uc;
    short s;
    unsigned short us;
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    bool b;
    float f;
    double d;
    long double ld;
} FundamentalType;

#define CHAR(x) \
    (FundamentalType) { .c = x }
#define UCHAR(x) \
    (FundamentalType) { .uc = x }
#define SHORT(x) \
    (FundamentalType) { .s = x }
#define USHORT(x) \
    (FundamentalType) { .us = x }
#define INT(x) \
    (FundamentalType) { .i = x }
#define UINT(x) \
    (FundamentalType) { .ui = x }
#define LONG(x) \
    (FundamentalType) { .l = x }
#define ULONG(x) \
    (FundamentalType) { .ul = x }
#define LONG_LONG(x) \
    (FundamentalType) { .ll = x }
#define ULONG_LONG(x) \
    (FundamentalType) { .ull = x }
#define BOOL(x) \
    (FundamentalType) { .b = x }
#define FLOAT(x) \
    (FundamentalType) { .f = x }
#define DOUBLE(x) \
    (FundamentalType) { .d = x }
#define LONG_DOUBLE(x) \
    (FundamentalType) { .ld = x }

// This macro takes in a DynamicArray typeID (unsigned int) and allows
// you to get the string version for potential use in print debugging
#define DYNAMIC_ARRAY_TYPE(x) \
    typeRegistry[x].type

#define DYNAMIC_ARRAY_TYPE_SIZE(x) \
    typeRegistry[x].size

// This is only called once at startup to initialize the type registry that is
// necessary for the dynamic_array functions to work
int dynamic_array_registry_init(void);

// Frees the type registry. Should be used at the very end of the life cycle of a program
int dynamic_array_registry_terminate(void);

// If the type being appended is a basic type, then you can simply pass in NULL for
// function pointer
int dynamic_array_registry_type_append(string* type, int (*deallocator)(void*), unsigned int size);

// Pass in a string of the types name, and it returns the id of that type, if it finds it
unsigned int dynamic_array_registry_get_typeID(string* type);

// The string deallocation function that will be passed to dynamic_array_registry_type_append
int string_deallocator(void* str);

#endif
