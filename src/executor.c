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

Executor *new_executor(enum CommandType command_type, char **args) {
    Executor *executor = malloc(sizeof(Executor));

    executor->command = args[0];
    executor->args = args;

    executor->command_type = command_type;

    return executor;
}

ExecutionPlan *new_execution_plan(Executor *executor) {
    ExecutionPlan *plan = malloc(sizeof(ExecutionPlan));

    plan->executor = executor;
    plan->next = NULL;
    plan->connection = NO_CONNECTION;

    return plan;
}

void free_executor(Executor *executor) {
    char **args = executor->args;
    int i = 0;
    while (executor->args[i] != NULL) {
        free(args[i]);
        i++;
    }

    free(args);

    free(executor);
}

void free_execution_plan(ExecutionPlan* plan) {
    do {
        if(plan->executor != NULL) {
            free_executor(plan->executor);
        }

        ExecutionPlan* old_plan = plan;
        plan = plan->next;
        free(old_plan);
    } while(plan != NULL);
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
    }
}

void print_executor(Executor *executor) {
    if (executor == NULL) {
        printf("NULL");
    }

    int len = 0;
    while (executor->args[len] != NULL) {
        len++;
    }

    printf("[");
    if (len > 0) {
        printf("\"%s\"", executor->args[0]);
    }
    for (int i = 1; i < len; i++) {
        printf(", \"%s\"", executor->args[i]);
    }
    printf("]");

}

void print_execution_plan(ExecutionPlan *plan) {
    if (plan == NULL) {
        printf("NULL");
        return;
    }

    if (plan->executor != NULL) {
        printf("executor=>");
        print_executor(plan->executor);
        printf(" ");
    }

    switch (plan->connection) {
        case CONNECTION_PIPE:
            printf("pipe");
            break;
        case CONNECTION_AFTER:
            printf("after");
            break;
        default:
            break;
    }

    if (plan->connection != NO_CONNECTION) {
        printf("=>{ ");
        print_execution_plan(plan->next);
        printf("}");
    }
}

pid_t run_executor(Executor *executor, Shell *shell, int fd_in, int fd_out, int fd_close, int fd_log) {
    CommandLocation *cmd_loc = which(shell, executor->command_type, executor->command);

    int built_in_status = 0;

    // We have to run built ins on this thread
    if (cmd_loc != NULL && cmd_loc->built_in != NULL) {
        built_in_status = execute_built_in(shell, executor);
    }

    int pid = fork();

    if (pid == 0) {
        if (fd_close != -1) {
            close(fd_close);
        }

        if (fd_out != -1) {
            // Replace stdout with fd_out
            dup2(fd_out, 1);
        }

        if (fd_in != -1) {
            // Replace stdin with fd_in
            dup2(fd_in, 0);
        }

        if (cmd_loc == NULL) {
            fd_print(fd_log, 3, "Unknown command `", executor->command, "`\n");
            exit(-1);
        }

        if (cmd_loc->path != NULL) {
            execvp(executor->args[0], executor->args);
            fd_print(fd_log, 3, "Failed to exec process ", executor->args[0], "\n");
            exit(-1);
        } else if (cmd_loc->built_in != NULL) {
            // This just needs to exit with the status from earlier
            exit(built_in_status);
        } else {
            fd_print(fd_log, 1,
                     "INVARIANT reached, cmd was not NULL but both path AND built_in were - should be impossible\n");
            exit(-1);
        }
    } else {
        if (pid == -1) {
            fd_print(fd_log, 1, "Couldn't fork process\n");
            exit(-1);
        }

        return pid;
    }
}

int wait_for_pid_exit(int pid, int fd_log) {
    int status = 0;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    } else {
        fd_print(fd_log, 1, "Process did not exit normally\n");
        return -1;
    }
}

int run_execution_plan(ExecutionPlan *plan, Shell *shell, int fd_out, int fd_log) {
    int fd_in = -1;

    //  TODO: int vec of pids to wait for

    while (plan->next != NULL) {
        switch (plan->connection) {
            case CONNECTION_PIPE: {
                int pipe_fd[2];
                pipe(pipe_fd);

                run_executor(plan->executor, shell, fd_in, pipe_fd[1], pipe_fd[0], fd_log);
                // Close our write end of pipe
                close(pipe_fd[1]);

                // Next fd_in is from the pipe
                fd_in = pipe_fd[0];
                break;
            }
            case CONNECTION_AFTER: {
                int pid = run_executor(plan->executor, shell, fd_in, fd_out, -1, fd_log);
                // Wait until this process has finished before moving on to next
                wait_for_pid_exit(pid, fd_log);
                fd_in = -1;
                break;
            }
            case NO_CONNECTION:
                fd_print(fd_log, 1, "Execution plan had non NULL next but the connection was NO_CONNECTION\n");
                break;
        }

        // Go to next execution plan
        plan = plan->next;
    }

    // Run the final executor
    int last_pid = run_executor(plan->executor, shell, fd_in, fd_out, -1, fd_log);
    if (fd_in != -1) {
        // Close our read end of pipe
        close(fd_in);
    }

    return wait_for_pid_exit(last_pid, fd_log);
}