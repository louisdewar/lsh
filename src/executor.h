//
// Created by Louis de Wardt
//


#pragma once
#include "shell.h"

typedef enum CommandType { ABSOLUTE, RELATIVE, GLOBAL, INVALID } CommandType;

typedef struct Executor {
  char* command;
  char* args;
  enum CommandType command_type;
} Executor;

Executor* new_executor(CommandType, char*, char*);
int run_executor(Executor*, Shell*);

void print_executor(Executor*);
char* stringify_command_type(CommandType);