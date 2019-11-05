//
// Created by Louis de Wardt on 05/11/2019.
//

#pragma once


#include "path.h"

typedef struct Shell {
    char* PATH;
    Path* working_directory;
} Shell;

Shell* new_shell(char* PATH);