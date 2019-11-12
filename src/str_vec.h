//
// Created by Louis de Wardt on 05/11/2019.
//

#pragma once

typedef struct StringVector {
    char** ptr;
    int len;
    int capacity;
} StringVector;

StringVector new_string_vector(int capacity);

void string_vector_append_n(StringVector*, char*, int);

void string_vector_append(StringVector* vec, char* str);

void free_string_vector(StringVector* vec);