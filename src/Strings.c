#include "Strings.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


extern void string_init(string* str);
extern void string_free(string* str);

// This implementation could definitely be better, but for now simply
// exactly fitting the size of the buffer to the string is probably just fine
int string_resize(string* str, unsigned int size) {
    str->len = size;
    str->__memsize = size + 1;

    char* temp = (char*)realloc(str->str, str->__memsize * sizeof(char));

    if (temp == NULL) {
        printf("ERROR: Failed to allocate memory for string resizing\n");
        exit(-1);
    } else {
        str->str = temp;
        // Make sure to prevent possible buffer overflows due to missing
        // null terminator. Essentially, this gets added automatically
        str->str[str->len] = '\0';
    }

    return 0;
}

int string_copy(string* dest, string* src) {
    string_resize(dest, src->len);
    memcpy(dest->str, src->str, src->len);
    return 0;
}

int string_concat(string* dest, string* base, string* add) {
    string_resize(dest, base->len + add->len);
    memcpy(dest->str, base->str, base->len);
    memcpy(dest->str + base->len, add->str, add->len);
    return 0;
}

int string_substring(string* dest, string* src, unsigned int from, unsigned int to) {
    if (to - from < 0) {
        printf("string_substring::Range Error::upper bound is less than lower bound\n");
        return -1;
    } else if (to == from) {
        // If the bounds are the same, it will just return an empty string,
        // which is signfified by the string pointer being NULL
        string_free(dest);
        return 0;
    } else if (to > src->len) {
        printf("string_substring::Range Error::Index Out of Bounds would have occurred\n");
        return -1;
    }

    string_resize(dest, to - from);
    memcpy(dest->str, src->str + from, dest->len);
    return 0;
}

int string_find(string* src, string* find) {
    // It is necessary to add 1 to src->len - find->len since we want to make sure that i actually reaches
    // the value of src->len - find->len, because the range of the final possible check that could be made in
    // the buffer is [src->len - find->len, src->len).
    for (int i = 0; i < src->len - find->len + 1; i++) {
        if (memcmp(src->str + i, find->str, find->len) == 0) {
            return i;
        }
    }

    // Return -1 if the find string isn't found in the src string
    return -1;
}

int string_compare(string* str1, string* str2) {
    if (str1->len == str2->len) {
        if (memcmp(str1->str, str2->str, str1->len) == 0) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

int string_insert(string* dest, string* insert, unsigned int from) {
    string lastThird;
    string_init(&lastThird);


    string_substring(&lastThird, dest, from, dest->len);
    string_resize(dest, dest->len + insert->len);
    memcpy(dest->str + from, insert->str, insert->len);
    memcpy(dest->str + from + insert->len, lastThird.str, lastThird.len);

    string_free(&lastThird);
    return 0;
}

int string_find_replace(string* src, string* find, string* replace) {
    int index = string_find(src, find);

    if (index == -1) {
        return false;
    } else {
        string lastThird;
        string_init(&lastThird);

        string_substring(&lastThird, src, index + find->len, src->len);
        string_resize(src, src->len - find->len + replace->len);

        memcpy(src->str + index, replace->str, replace->len);
        memcpy(src->str + index + replace->len, lastThird.str, lastThird.len);

        string_free(&lastThird);
        return true;
    }
}

int string_read_console(string* str) {
    char buf[MAX_LINE_READ];
    // fgets always returns a null terminated string
    fgets(buf, MAX_LINE_READ, stdin);

    int bufLen = strnlen(buf, MAX_LINE_READ);

    // This function will automatically ensure the null terminator is there
    // Also, subtract one to make sure the newline character isn't included, because otherwise that happens
    // by default
    if (bufLen - 1 > 0) {
        string_resize(str, bufLen - 1);
        memcpy(str->str, buf, bufLen - 1);
    } else {
        string_resize(str, 1);
        str->str[0] = '\0';
    }

    return 0;
}

int string_read_file(string* str, string* path) {
    // Read as a binary file
    FILE* fptr = fopen(path->str, "rb");
    if (fptr == NULL) {
        printf("string_read_file could not find the specified file\n");
        exit(-1);
    }

    fseek(fptr, 0, SEEK_END);
    int len = ftell(fptr) / sizeof(char);
    fseek(fptr, 0, SEEK_SET);

    string_resize(str, len);
    fread(str->str, sizeof(char), str->len, fptr);

    fclose(fptr);
    return 0;
}

int string_compare_with_offset(string* strOffset, string* str2, unsigned int offset) {
    if (str2->len + offset > strOffset->len) {
        return false;
    } else if (offset > strOffset->len) {
        return false;
    }

    if (memcmp(strOffset->str + offset, str2->str, str2->len) == 0) {
        return true;
    } else {
        return false;
    }
}
