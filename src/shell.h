//
// Created by Louis de Wardt on 05/11/2019.
//

#pragma once


#include "path.h"
#include "hashmap.h"
#include <stdbool.h>

typedef struct Shell {
    Path* working_directory;
    int last_exit_status;
    bool running;
    HashMap* env_vars;
} Shell;

Shell* new_shell(char**);

char* shell_get_env_var(Shell*, char*, bool);