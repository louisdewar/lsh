//
// Created by Louis de Wardt
//

#include "executor.h"

#include <stdlib.h>
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

int run_executor(Executor* executor) {
    if (executor)
    return 0;
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
    printf("Executor { type: %s, path: '%s', args: '%s' }", stringify_command_type(executor->command_type), executor->path, executor->args);
}