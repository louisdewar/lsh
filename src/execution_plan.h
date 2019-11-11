//
// Created by Louis de Wardt on 10/11/2019.
//

#pragma once

#include "executor.h"

// TODO: Consider how to support both on success and on failure from 1 command, perhaps vector of execution plans, each with a unique connection?
typedef enum ExecutionConnection {
    NO_CONNECTION,
    CONNECTION_PIPE,
    CONNECTION_AFTER,
    CONNECTION_ON_SUCCESS,
    CONNECTION_ON_FAILURE,
    CONNECTION_FORK,
} ExecutionConnection;

typedef struct ExecutionPlan {
    Executor* executor;
    struct ExecutionPlan* next;
    ExecutionConnection connection;
} ExecutionPlan;

void free_execution_plan(ExecutionPlan*);

ExecutionPlan *new_execution_plan(Executor* executor);

void print_execution_plan(ExecutionPlan*);
int run_execution_plan(ExecutionPlan*, Shell*, int, int);