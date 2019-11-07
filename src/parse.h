//
// Created by Louis de Wardt
//


#pragma once

#include "executor.h"
#include "path.h"

ExecutionPlan* parse_line(char*);

ExecutionPlan *new_execution_plan(Executor* executor);

CommandType get_command_type(char*);