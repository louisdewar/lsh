//
// Created by Louis de Wardt
//

#include "executor.h"
#include "builtins.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Executor* new_executor(enum CommandType command_type, char* path, char* args) {
    Executor* executor = malloc(sizeof(Executor));

    // Allocate the values on the heap
    executor->path = malloc(sizeof(char) * (strlen(path) + 1));
    strcpy(executor->path, path);

    executor->args = malloc(sizeof(char) * (strlen(args) + 1));
    strcpy(executor->args, args);

    executor->command_type = command_type;

    return executor;
}

int run_executor(Executor* executor, Shell* shell) {
    CommandLocation* cmd_loc = which(shell, executor->command_type, executor->path);

    if (cmd_loc == NULL) {
         printf("Unknown command `%s`", executor->path);
         return 1;
    } else if (cmd_loc->path != NULL) {
        printf("Running file %s", cmd_loc->path->str);
        return 0;
    } else if (cmd_loc->built_in != NULL) {
        printf("Running built in %s", cmd_loc->built_in);
        return 0;
    } else {
        printf("INVARIANT reached, cmd was not NULL but both path AND built_in were should be impossible");
        exit(1);
    }
}

char* stringify_command_type(enum CommandType type) {
    switch (type) {
        case ABSOLUTE:
            return "ABSOLUTE";
        case RELATIVE:
            return "RELATIVE";
        case GLOBAL:
            return "GLOBAL";
        case INVALID:
            return "INVALID";
    }
}

void print_executor(Executor* executor) {
    printf("Executor { type: %s, path: '%s', args: '%s' }\n", stringify_command_type(executor->command_type), executor->path, executor->args);
}