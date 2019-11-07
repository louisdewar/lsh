//
// Created by Louis de Wardt
//


#pragma once
#include "shell.h"

typedef enum CommandType { ABSOLUTE, RELATIVE, GLOBAL } CommandType;

typedef struct Executor {
  char** args;
} Executor;

typedef enum ExecutionConnection {
    NO_CONNECTION,
    CONNECTION_PIPE,
    CONNECTION_AFTER,
} ExecutionConnection;

typedef struct ExecutionPlan {
    Executor* executor;
    struct ExecutionPlan* next;
    ExecutionConnection connection;
} ExecutionPlan;

Executor* new_executor(char**);
int run_execution_plan(ExecutionPlan*, Shell*, int, int);

void free_executor(Executor*);
void free_execution_plan(ExecutionPlan*);

void print_execution_plan(ExecutionPlan*);