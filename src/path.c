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

void path_append_raw(Path* path, char* str) {
    int str_len = strlen(str);

    // Don't do anything (exit early helps with some assumptions further down)
    if(str_len == 0) {
        return;
    }

    // This path is absolute so str should replace the current path
    if (str[0] == '/') {
        path->str[0] = '\0';
        path->len = 0;
    }


    // Remove ending slash if there are chars before it (allow '/')
    if (str_len > 1 && str[str_len - 1] == '/') {
        str[str_len - 1] = '\0';
        str_len -= 1;
    }

    // Combined length plus the separator (/) and null char (doesn't matter if we don't actually use the /, all we are doing is reserving capacity)
    int combined_len = str_len + path->len + 2;

    if (path->cap < combined_len) {
        char *new_str = malloc(sizeof(char) * combined_len);

        strcpy(new_str, path->str);

        free(path->str);

        path->str = new_str;

        path->cap = combined_len;
        path->len = strlen(path->str);
    }

    // Put the separator between the strings if there was already a string in the path and don't allow //
    // (done by default since path can't end in / and if str starts with / then str is treated as absolute and path->str has already been set to ""
    if (path->len > 0) {
        strcat(path->str, "/");
    }

    strcat(path->str, str);
    path->len = strlen(path->str);
}

Path* new_empty_path(int capacity) {
    if (capacity < 0) {
        capacity = 0;
    }

    // Add 1 for NULL char
    capacity += 1;

    Path* path = malloc(sizeof(Path));

    path->cap = capacity;
    path->len = 0;
    path->str = malloc(sizeof(char) * capacity);
    // Set the first char to be null pointer
    *path->str = '\0';

    return path;
}

void path_delete_last_segment(Path* path) {
    char* last_segment = strrchr(path->str, '/');

    // Entire string is last segment
    if (last_segment == NULL) {
        // Preserve allocation for current string just set first char to NULL (don't need to change capacity)
        *path->str = '\0';
        path->len = 0;
    } else {
        // Reduce string length without changing capacity
        *last_segment = '\0';
        path->len = strlen(path->str);
    }
}

Path* new_path_from_str(char* str) {
    Path* path = new_empty_path(strlen(str));

    // Path is absolute so ensure this char gets added to start of string
    if (*str == '/') {
        path_append_raw(path, "/");
        str++;
    }

    if (strlen(str) > 0) {
        char* segment = strtok(str, "/");

        while(segment != NULL) {
            if (strcmp(segment, "..") == 0) {
                path_delete_last_segment(path);
            } else if (strcmp(segment, ".") != 0) {
                path_append_raw(path, segment);
            }

            segment = strtok(NULL, "/");
        }
    }

    return path;
}

void sanitise_append(Path* path, char* str) {
    int len = strlen(str);
    if (len > 0) {
        char* segment_start = str;
        char* segment_end = strchr(segment_start, '/');

        while (segment_end != NULL) {
            int segment_len = segment_end - segment_start;
            char *segment = (char *) calloc(sizeof(char), 1 + segment_len);
            strncpy(segment, segment_start, segment_end - segment_start);
            segment[segment_len] = '\0';

            if (strcmp(segment, "..") == 0) {
                path_delete_last_segment(path);
            } else if (strcmp(segment, ".") != 0) {
                path_append_raw(path, segment);
            }

            segment_start = segment_end + 1;
            segment_end = strchr(segment_start, '/');
        }

        if(strcmp(segment_start, "..") == 0) {
            path_delete_last_segment(path);
        } else if(strcmp(segment_start, ".") != 0) {
            // Append the last segment
            path_append_raw(path, segment_start);
        }
    }
}

void path_join(Path* path, char* str) {
    sanitise_append(path, str);
}

Path* new_path_from_join(Path* path, char* str) {
    Path* new_path = new_empty_path(1);

    path_append_raw(new_path, path->str);
    sanitise_append(new_path, str);

    return new_path;
}

Path* new_path_from_str_slice(char* start, int len) {
     char* sub_str = (char*) malloc(sizeof(char) * (len + 1));
     strncpy(sub_str, start, len);
     sub_str[len] = '\0';

     Path* path = new_path_from_str(sub_str);

     free(sub_str);

     return path;
}

char* get_path_last_segment(Path* path) {
    char* last_segment = strrchr(path->str, '/');

    // No / therefore the whole path is the last segment
    if (last_segment == NULL) {
        return path->str;
    } else {
        // The string is just /
        if (path->len == 1) {
            return path->str;
        } else {
            // Return the last segment excluding the /
            return ++last_segment;
        }
    }
}

void insert_home(Path* path, char* home) {
    // If the first char is tilde
    if(*path->str == '~') {
        char* old_str = calloc(sizeof(char), path->len + 1);
        strcpy(old_str, path->str);

        int cap = strlen(home) + path->len + 2;
        path->str = calloc(sizeof(char), cap);
        path->cap = cap;
        strcpy(path->str, home);

        path_join(path, old_str);

        free(old_str);
    }
}

void free_path(Path* path) {
    free(path->str);
    free(path);
}