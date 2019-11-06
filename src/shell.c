//
// Created by Louis de Wardt on 05/11/2019.
//

#include "shell.h"

#include <stdlib.h>
#include <string.h>

Shell* new_shell(char* PATH) {
    Shell* shell = malloc(sizeof(Shell));

    // Allocate on heap
    shell->PATH = malloc(sizeof(char) * (strlen(PATH) + 1));
    strcpy(shell->PATH, PATH);

    shell->working_directory = new_path_from_cwd();
    shell->running = true;

    return shell;
}