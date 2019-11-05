//
// Created by Louis de Wardt
//

#pragma once

#include "executor.h"
#include "shell.h"
#include "path.h"

typedef struct CommandLocation {
    Path* path;
    char* built_in;
};

CommandLocation which(Shell*, enum CommandType type, char* path);