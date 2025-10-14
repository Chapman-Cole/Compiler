#include "Strings.h"
#include <stdio.h>

int main(void) {
    string str;
    string_init(str);
    string_copy(&str, &STRING("This is a test"));
    string_insert(&str, &STRING(" is"), str.len);
    string_insert(&str, &STRING("choo"), 3 + string_find(&str, &STRING(" is ")));

    printf("Before:\n%s\n\n", str.str);
    while (string_find_replace(&str, &STRING("is"), &STRING("ahh")));
    printf("After:\n%s\n", str.str);
    return 0;
}
