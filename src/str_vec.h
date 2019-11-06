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

void string_vector_append(StringVector*, char*);