//
// Created by Louis de Wardt
//


#pragma once
#include <stdio.h>

enum CommandType { ABSOLUTE, RELATIVE, GLOBAL, INVALID };

typedef struct Executor {
  char* path;
  char* args;
  enum CommandType command_type;
} Executor;

Executor* new_executor(enum CommandType command_type, char* path, char* args);
void print_executor(Executor* executor);