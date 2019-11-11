//
// Created by Louis de Wardt
//


#pragma once

#include "execution_plan.h"
#include "path.h"

ExecutionPlan* parse_line(char*);

CommandType get_command_type(char*);