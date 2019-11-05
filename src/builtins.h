//
// Created by Louis de Wardt
//

#pragma once


#include "shell.h"
#include "path.h"
#include "executor.h"

typedef struct CommandLocation {
    Path* path;
    char* built_in;
} CommandLocation;

CommandLocation* which(Shell*, CommandType type, char* path);