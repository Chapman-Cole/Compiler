#include <stdio.h>
#include <stdlib.h>
#include "Strings.h"
#include "DynamicArray.h"
#include <stdint.h>

// Instructions will be one byte (8 bits long)

// This is helpful for easily accessing the 8 bits that define the operation, of which there are 256 possible operations
// but not all of those possiblities will be used
typedef union {
    uint64_t full;
    // The operation is specified by the instruction
    uint8_t op;
} instruction;

int main(void) {
    dynamic_array_registry_init();
    dynamic_array_registry_type_append(&STRING("uint64_t"), NULL, sizeof(uint64_t));

    DynamicArray heap;
    dynamic_array_init(&heap, &STRING("uint64_t"));

    printf("Hello World\n");

    dynamic_array_free(&heap);
    dynamic_array_registry_terminate();
    return 0;
}