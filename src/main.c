//
// Created by Louis de Wardt
//


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <signal.h>

#include "parse.h"
#include "executor.h"
#include "shell.h"

#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define BLUE "\x1b[34m"
#define RESET_COLOUR "\x1b[0m"

int main(void) {
    printf("lsh v0.1\n");

    Shell *shell = new_shell(getenv("PATH"), getenv("HOME"));

    // Main loop
    while (shell->running) {
        if (shell->last_exit_status == 0) {
            printf(GREEN"(lsh)"BLUE" %s "GREEN">> "RESET_COLOUR, get_path_last_segment(shell->working_directory));
        } else {
            printf(RED"(lsh)"BLUE" %s "RED">> "RESET_COLOUR, get_path_last_segment(shell->working_directory));
        }

        // Max line chars is 4096
        char line[4096];

        if (fgets(line, 4096, stdin) == NULL) {
            perror("Couldn't read line");
            break;
        }

        // Replace the newline with a null char (terminate string early)
        line[strcspn(line, "\n")] = '\0';

        Executor *executor = parse_line(line);

        if (executor == NULL) {
            shell->last_exit_status = -1;
            continue;
        }

//    print_executor(executor);
        shell->last_exit_status = run_executor(executor, shell);
    }

    return 0;
}
