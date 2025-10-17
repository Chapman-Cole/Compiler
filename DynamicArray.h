#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include "Strings.h"
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

int dynamic_array_init(DynamicArray* arr, size_t element_size, string* type);

int dynamic_array_free(DynamicArray* arr);

// In order to make this function work with any type of data, you have to provide
// a pointer to that data so that the raw bytes can be shuffled into the array
// Note: it is assumed that the size of the data that the buffer points to
// is equal to element_size value of the struct
int dynamic_array_append(DynamicArray* arr, void* data);

typedef struct DynamicArrayType {
    string type;
    unsigned int typeID;
} DynamicArrayType;

// This is needed to get floating point numbers to pass properly into the append function
typedef union FloatingPointData {
    float f;
    double d;
    long double ld;
} FloatingPointData;

#define FLOAT(x) \
    (FloatingPointData) { .f = x }
#define DOUBLE(x) \
    (FloatingPointData) { .d = x }
#define LONG_DOUBLE(x) \
    (FloatingPointData) { .ld = x }

// Use this define at the top of the file near the include headers
// in order to get access to the variables below
#define DYNAMIC_ARRAY_REGISTRY_ENABLE    \
    extern unsigned int typeRegistryLen; \
    extern DynamicArrayType* typeRegistry;

// This is only called once at startup to initialize the type registry that is
// necessary for the dynamic_array functions to work
int dynamic_array_registry_setup(void);

int dynamic_array_registry_type_append(string* type);

// Pass in a string of the types name, and it returns the id of that type, if it finds it
unsigned int dynamic_array_registry_get_typeID(string* type);

#endif
