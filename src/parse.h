//
// Created by Louis de Wardt
//


#pragma once

#include "executor.h"
#include "path.h"

Executor* parse_line(char* line, Path* working_directory);
