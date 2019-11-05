//
// Created by Louis de Wardt
//

#include "parse.h"

#include <stdio.h>
#include <string.h>

#include "executor.h"

CommandType get_command_type(char *word) {
    // This is guaranteed to be at least one
    int len = strlen(word);

    // it is only possible for this to be GLOBAL or RELATIVE
    if (len == 1) {
        if (word[0] == '/') {
            printf("'/' is not a valid command\n");
            return INVALID;
        } else {
            return GLOBAL;
        }
    }


    // This can not be RELATIVE - need './{path}'
    if (len == 2) {
        if (word[0] == '/') {
            return ABSOLUTE;
        } else if (word[0] == '.') {
            printf("Invalid path '%s'\n", word);
            return INVALID;
        } else {
            return GLOBAL;
        }
    }

    if (word[0] == '.' && word[1] == '/') {
        return RELATIVE;
    }

    if (word[0] == '/') {
        return ABSOLUTE;
    }

    // If string contains / then it must be relative and not global
    if (strchr(word, '/') != NULL) {
        return RELATIVE;
    }

    return GLOBAL;
}

Executor *parse_line(char *line, Path *working_directory) {
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

    if (type == INVALID) {
        return NULL;
    } else {
        return new_executor(type, first_word, args);
    }
}
