#ifndef DYNAMICARRAY_H
#define DYNAMICARRAY_H

#include "Strings.h"
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

int dynamic_array_init(DynamicArray* arr, size_t element_size, string* type);

int dynamic_array_free(DynamicArray* arr);

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

typedef struct DynamicArrayType {
    string type;
    unsigned int typeID;

    // If true, simply cast your data to a void pointer to pass it directly
    // If false, pass a reference to your union, struct, or enum into the append function
    bool passByValue;
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

// This macro takes in a DynamicArray typeID (unsigned int) and allows
// you to get the string version for potential use in print debugging
#define DYNAMIC_ARRAY_TYPE(x) \
    typeRegistry[x].type

// This is only called once at startup to initialize the type registry that is
// necessary for the dynamic_array functions to work
int dynamic_array_registry_setup(void);

int dynamic_array_registry_type_append(string* type, bool passByValue);

// Pass in a string of the types name, and it returns the id of that type, if it finds it
unsigned int dynamic_array_registry_get_typeID(string* type);

#endif
