//
// Created by Louis de Wardt on 05/11/2019.
//

#pragma once


#include "path.h"
#include <stdbool.h>

typedef struct Shell {
    char* PATH;
    Path* working_directory;
    int last_exit_status;
    bool running;
    char* home;
} Shell;

Shell* new_shell(char *PATH, char *home);