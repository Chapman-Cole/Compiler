#include "DynamicArray.h"
#include "Strings.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    dynamic_array_registry_init();

    DynamicArray test;
    dynamic_array_init_nDimensions(&test, &STRING("int"), &INDEX(3, 10, 10));

    for (int i = 0; i < test.len; i++) {
        int s1 = ((DynamicArray*)dynamic_array_get(&test, &INDEX(i)))->len;
        for (int j = 0; j < s1; j++) {
            int s2 = ((DynamicArray*)dynamic_array_get(&test, &INDEX(i, j)))->len;
            for (int k = 0; k < s2; k++) {
                dynamic_array_set(&test, &INDEX(i, j, k), &INT((i + 1) * 100 + (j + 1) * (k + 1)));
            }
        }
    }

    for (int i = 0; i < test.len; i++) {
        int s1 = ((DynamicArray*)dynamic_array_get(&test, &INDEX(i)))->len;
        for (int j = 0; j < s1; j++) {
            int s2 = ((DynamicArray*)dynamic_array_get(&test, &INDEX(i, j)))->len;
            for (int k = 0; k < s2; k++) {
                printf("%5d", *(int*)dynamic_array_get(&test, &INDEX(i, j, k)));
            }
            printf("\n");
        }
        printf("\n------------------------------------------------------------------\n\n");
    }

    dynamic_array_free(&test);
    dynamic_array_registry_terminate();
    return 0;
}
