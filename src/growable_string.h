//
// Created by Louis de Wardt on 07/11/2019.
//

#pragma once

typedef struct String {
    char* str;
    int len;
    int capacity;
} String;

String new_string(char* str);
void push_str(String* s, char* str);
void push_str_n(String* s, char* str, int n);
void push_char(String* s, char c);