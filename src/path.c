//
// Created by Louis de Wardt on 05/11/2019.
//

#include "path.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

Path* new_path_from_cwd() {
    int capacity = 2048;

    char* str = (char*) malloc(sizeof(char) * capacity);

    if(getcwd(str, capacity) == NULL) {
        return NULL;
    }

    Path* path = malloc(sizeof(Path));

    path->cap=capacity;
    path->len=strlen(str);
    path->str=str;

    return path;
}

Path* path_append_raw(Path* path, char* str) {
    int str_len = strlen(str);

    // Combined length plus the separator (/) and null char
    int combined_len = str_len + path->len + 2;

    if (path->cap < combined_len) {
        char* new_str = malloc(sizeof(char) * combined_len);

        strcpy(new_str, path->str);

        if (path->len > 0) {
            // Put the seperator between the strings if there was already a string in the path
            strcat(new_str, '/');
        }

        strcat(new_str, str);

        path->str = new_str;
        path->cap = combined_len;
        path->len = strlen(path->str);
    } else {
        strcat(path->str, str);
        path->len = strlen(path->str);
    }
}

Path* new_empty_path() {
    Path* path = malloc(sizeof(Path));

    path->cap = 1;
    path->len = 0;
    path->str = '\0';

    return path;
}

Path* path_delete_last_segment(Path* path) {
    char* last_segment = strrchr(path->str, '/');

    // Entire string is last segment
    if (last_segment == NULL) {
        // Preserve allocation for current string just set first char to NULL (don't need to change capacity)
        path->str[0] = '\0';
        path->len = 0;
    } else {
        // Reduce string length without changing capacity
        *last_segment = '\0';
        path->len = strlen(path->str);
    }
}

Path* new_path_from_str(char* str) {
    Path* path = new_empty_path();

    // Path is absolute so ensure this char gets added to start of string
    if (*str == '/') {
        path_append_raw(path, '/');
        str++;
    }

    if (strlen(str) > 0) {
        char* segment = strtok(str, "/");

        while(segment != '\0') {
            if (segment == '..') {
                path_delete_last_segment(path);
            } else if (segment != '.') {
                path_append_raw(path, segment);
            }

            segment = strtok(str, "/");
        }
    }
}

Path* new_path_from_join(Path* path, char* str) {
    Path* new_path = new_empty_path();

    path_append_raw(new_path, path->str);
    path_append_raw(new_path, str);

    return new_path;
}

void free_path(Path* path) {
    free(path->str);
    free(path);
}