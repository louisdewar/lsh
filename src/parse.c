//
// Created by Louis de Wardt
//

#include "parse.h"

#include <stdio.h>
#include <string.h>

#include "executor.h"
#include "str_vec.h"

CommandType get_command_type(char *word) {
    PathType p_type = get_path_type(word);

    if (p_type == P_ABSOLUTE) {
        return ABSOLUTE;
    }

    // If string contains / then it must be relative and not global
    if (strchr(word, '/') != NULL) {
        return RELATIVE;
    }

    return GLOBAL;
}

void parse_arguments(StringVector *vec, char *args) {
    char *start = strtok(args, " ");

    while (start != NULL) {
        string_vector_append(vec, start);

        start = strtok(NULL, " ");
    }


//    printf("Arg vec[%i]: [", vec.len);
//    if (vec.len > 0) {
//        printf("\"%s\"", vec.ptr[0]);
//    }
//    for(int i = 1; i < vec.len; i++) {
//        printf(", \"%s\"", vec.ptr[i]);
//    }
//    printf("]\n");

}

Executor *parse_line(char *line) {
    // Remove leading whitespace
    while (*line == ' ') {
        line++;
    }

    if (*line == '\0') {
        return NULL;
    }

    char *first_word = line;

    // Find the start of args
    char *args = strchr(line, ' ');
    if (args != NULL) {
        // Terminate first word string
        *args = '\0';
        // Args should start after the null char;
        args++;
    } else {
        // Set args to be an empty string since there are none (NULL ptr would cause errors later on)
        args = "";
    }

    // Get the type of path
    enum CommandType type = get_command_type(first_word);

    StringVector vec = new_string_vector(1);
    string_vector_append(&vec, first_word);
    parse_arguments(&vec, args);

    return new_executor(type, first_word, vec.ptr);

}
