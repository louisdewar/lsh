//
// Created by Louis de Wardt on 10/11/2019.
//

#include "execution_plan.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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
    bool should_skip_next = false;

    //  TODO: int vec of pids to wait for

    while (plan->next != NULL) {
        if (!should_skip_next) {
            // The connection to the *next* command
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
                    shell->last_exit_status = wait_for_pid_exit(pid, fd_log);
                    fd_in = -1;
                    break;
                }
                case CONNECTION_ON_SUCCESS: {
                    int pid = run_executor(plan->executor, shell, fd_in, fd_out, -1, fd_log);
                    shell->last_exit_status = wait_for_pid_exit(pid, fd_log);

                    if (shell->last_exit_status != 0) {
                        should_skip_next = true;
                    }
                    break;
                }
                case CONNECTION_FORK: {
                    int pid = run_executor(plan->executor, shell, fd_in, fd_out, -1, fd_log);
                    char buf[255];
                    sprintf(buf, "[forked %i]", pid);
                    fd_print(fd_log, 1, buf);
                    break;
                }
                case CONNECTION_ON_FAILURE: {
                    int pid = run_executor(plan->executor, shell, fd_in, fd_out, -1, fd_log);
                    shell->last_exit_status = wait_for_pid_exit(pid, fd_log);

                    if (shell->last_exit_status != 0) {
                        should_skip_next = true;
                    }
                    break;
                }
                case NO_CONNECTION:
                    fd_print(fd_log, 1, "Execution plan had non NULL next but the connection was NO_CONNECTION\n");
                    break;
            }
        } else {
            should_skip_next = false;
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

void free_execution_plan(ExecutionPlan *plan) {
    do {
        if (plan->executor != NULL) {
            free_executor(plan->executor);
        }

        ExecutionPlan *old_plan = plan;
        plan = plan->next;
        free(old_plan);
    } while (plan != NULL);
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
        case NO_CONNECTION:
            break;
        case CONNECTION_FORK:
            printf("fork");
            break;
        case CONNECTION_ON_SUCCESS:
            printf("on success");
            break;
        case CONNECTION_ON_FAILURE:
            printf("on failure");
            break;
        default:
            printf("__unhandled__");
            break;
    }

    if (plan->connection != NO_CONNECTION) {
        printf("=>{ ");
        print_execution_plan(plan->next);
        printf("}");
    }
}