//
// Created by Louis de Wardt on 07/11/2019.
//

#include "growable_string.h"

#include <stdlib.h>
#include <string.h>

// Doesn't re-allocate str (if str is NULL it creates a new empty string)
String new_string(char* str) {
    int capacity = 1;
    int len = 0;

    if (str != NULL) {
        len = strlen(str);
        capacity = len + 1;
    } else {
        str = malloc(sizeof(char));
        *str = '\0';
    }

    String string = {
        .capacity = capacity,
        .len = len,
        .str = str
    };

    return string;
}

/// Allocates str onto current string
void push_str(String* s, char* str) {
    int str_len = strlen(str);

    // Not enough room for new string and NULL terminator
    if(s->len + str_len >= s->capacity) {
        char* new_str = calloc(sizeof(char), s->len + str_len + 1);
        strcpy(new_str, s->str);
        free(s->str);
        s->str = new_str;

        s->capacity = s->len + str_len + 1;
    }

    strcat(s->str, str);
    s->len += str_len;
}

/// Allocates n chars from str onto current string, does NOT free old str
void push_str_n(String* s, char* str, int n) {
    // Not enough room for new string and NULL terminator
    if(s->len + n >= s->capacity) {
        char* new_str = calloc(sizeof(char), s->len + n + 1);
        strcpy(new_str, s->str);
        free(s->str);
        s->str = new_str;

        s->capacity = s->len + n + 1;
    }

    strncat(s->str, str, n);
    s->len += n;
    s->str[s->len] = '\0';
}

void push_char(String* s, char c) {
    if (s->len + 1 >= s->capacity) {
        // Double previous capacity
        int new_capacity = s->capacity * 2;

        char* new_str = calloc(sizeof(char), new_capacity);
        strcpy(new_str, s->str);
        free(s->str);
        s->str = new_str;

        s->capacity = new_capacity;
    }

    s->str[s->len] = c;
    s->str[++s->len] = '\0';
}