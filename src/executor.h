//
// Created by Louis de Wardt
//


#pragma once
#include "shell.h"

#include <stdlib.h>

typedef enum CommandType { ABSOLUTE, RELATIVE, GLOBAL } CommandType;

typedef struct Executor {
  char** args;
} Executor;

Executor* new_executor(char**);

void free_executor(Executor*);

pid_t run_executor(Executor *executor, Shell *shell, int fd_in, int fd_out, int fd_close, int fd_log);

void fd_print(int fd, int count, ...);

void print_executor(Executor *executor);