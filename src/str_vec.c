//
// Created by Louis de Wardt on 05/11/2019.
//

#include "str_vec.h"

#include <stdlib.h>
#include <string.h>

StringVector new_string_vector(int capacity) {
    if (capacity < 0) {
        capacity = 0;
    }

    // Null terminated
    capacity++;

    char** ptr = malloc(sizeof(char*) * capacity);

    *ptr = NULL;

    StringVector vec = {
        .capacity = capacity,
        .ptr = ptr,
        .len = 0,
    };

    return vec;
}

void string_vector_append(StringVector* vec, char* str) {
    // If old vec + new string is over capacity allocate more (use >= because we have to also include a NULL ptr)
    if (vec->len + 1 >= vec->capacity) {
        char** old_ptr = vec->ptr;

        vec->capacity = vec->capacity * 2;
        vec->ptr = calloc(sizeof(char*), vec->capacity);
        // Copy old pointers
        memcpy(vec->ptr, old_ptr, sizeof(char*) * vec->len);
    }

    char* str_copy = calloc(sizeof(char), strlen(str) + 1);
    strcpy(str_copy, str);

    vec->ptr[vec->len] = str_copy;
    vec->len++;

    vec->ptr[vec->len] = NULL;
}