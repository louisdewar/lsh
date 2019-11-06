//
// Created by Louis de Wardt
//


#pragma once

#include "executor.h"
#include "path.h"

Executor* parse_line(char*);

CommandType get_command_type(char*);