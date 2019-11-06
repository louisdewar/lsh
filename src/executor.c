//
// Created by Louis de Wardt
//

#include "executor.h"
#include "builtins.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

Executor *new_executor(enum CommandType command_type, char *path, char **args) {
    Executor *executor = malloc(sizeof(Executor));

    // Allocate the values on the heap
    executor->command = malloc(sizeof(char) * (strlen(path) + 1));
    strcpy(executor->command, path);

    executor->args = args;

    executor->command_type = command_type;

    return executor;
}

// Helper function for printing to file descriptors
void fd_print(int fd, int count, ...) {
    va_list arg_list;

    va_start(arg_list, count);

    for (int i = 0; i < count; i++) {
        char *str = va_arg(arg_list, char*);
        write(fd, str, strlen(str));
    }
}

char *stringify_command_type(enum CommandType type) {
    switch (type) {
        case ABSOLUTE:
            return "ABSOLUTE";
        case RELATIVE:
            return "RELATIVE";
        case GLOBAL:
            return "GLOBAL";
        case INVALID:
            return "INVALID";
    }
}

int exec_wait_status(char *args[], int in_fd, int out_fd, int log_fd) {
    int pid = exec_process(args, in_fd, out_fd, log_fd);

    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        fd_print(log_fd, 1, "Process did not exit normally\n");
        return -1;
    }
}

pid_t exec_process(char *args[], int in_fd, int out_fd, int log_fd) {
    int pid = fork();

    if (pid == 0) {
        if (in_fd == -1) {
            close(0);
        } else {
            // Replace stdin with in_fd
            dup2(in_fd, 0);
        }

        if (out_fd == -1) {
            close(1);
        } else {
            // Replace stdout with out_fd
            dup2(out_fd, 1);
        }

        execvp(args[0], args);

        fd_print(log_fd, 3, "Failed to exec process ", args[0], "\n");
        exit(-1);
    } else {
        if (pid == -1) {
            fd_print(log_fd, 1, "Couldn't fork process\n");
            exit(-1);
        }

        return pid;
    }

}

// Pipe process a to b
// TODO: change to return PID
int plumb(char *args_a[], char *args_b[], int fd_out, int log_fd) {
    int pipefd[2];
    pipe(pipefd);

    pid_t pid_a = fork();

    if (pid_a == 0) {
        close(pipefd[0]);

        dup2(pipefd[1], 1);
        execvp(args_a[0], args_a);

        fd_print(log_fd, 3, "Failed to exec process ", args_a[0], "\n");
        exit(1);
    } else {
        if (pid_a == -1) {
            fd_print(log_fd, 1, "Couldn't fork process\n");
            exit(-1);
        }

        close(pipefd[1]);
        pid_t pid = fork();

        if (pid == 0) {
            dup2(pipefd[0], 0);
            dup2(fd_out, 1);
            execvp(args_b[0], args_b);

            fd_print(log_fd, 3, "Failed to exec process ", args_a[0], "\n");
            exit(1);
        } else {
            close(pipefd[0]);

            int status = 0;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            } else {
                fd_print(log_fd, 1, "Process did not exit normally\n");
                return -1;
            }
        }
    }
}

int run_executor(Executor *executor, Shell *shell) {
    CommandLocation *cmd_loc = which(shell, executor->command_type, executor->command);

    if (cmd_loc == NULL) {
        printf("Unknown command `%s`\n", executor->command);
        return 1;
    } else if (cmd_loc->path != NULL) {
        int status = exec_wait_status(executor->args, -1, 1, 2);
//        printf("Exit status: %i\n", status);
        return status;
    } else if (cmd_loc->built_in != NULL) {
        return execute_built_in(shell, executor);
    } else {
        printf("INVARIANT reached, cmd was not NULL but both path AND built_in were - should be impossible\n");
        exit(1);
    }
}