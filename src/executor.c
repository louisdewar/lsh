//
// Created by Louis de Wardt
//

#include "executor.h"

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "builtins.h"
#include "growable_string.h"
#include "parse.h"

Executor *new_executor(char **args) {
    Executor *executor = malloc(sizeof(Executor));

    executor->args = args;

    return executor;
}

ExecutionPlan *new_execution_plan(Executor *executor) {
    ExecutionPlan *plan = malloc(sizeof(ExecutionPlan));

    plan->executor = executor;
    plan->next = NULL;
    // This is the default
    plan->connection = CONNECTION_AFTER;

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



char *sanitise_argument(char *arg, Shell *shell) {
    String s = new_string(NULL);

    char quote_char = '\0';
    bool escape_next = false;

    while (*arg != '\0') {
        // Handle \ char (if escaped then treat as normal char - will get appended at the end)
        if(*arg == '\\' && !escape_next) {
            // Escape the next char
            escape_next = true;
            // Skip to next char
            arg++;
            continue;
        }


        // Expand a variable if we are not in `'` and we're not escaped
        if(*arg == '$' && !escape_next && quote_char != '\'') {
            // This is a regular variable name
            if (isalnum(*(arg + 1)) || *(arg + 1) == '_') {
                // Start at the first char of variable name
                char *start = ++arg;

                arg++;

                // Move arg to the first char after the variable name
                while (isalnum(*arg) || *arg == '_') {
                    arg++;
                }

                int name_len = arg - start;
                char *var_name = calloc(sizeof(char), name_len + 1);
                strncpy(var_name, start, name_len);
                var_name[name_len] = '\0';

                char *value = shell_get_env_var(shell, var_name, false);

                if (value == NULL) {
                    // Don't append so variable acts as empty string
                    continue;
                }

                push_str(&s, value);

                continue;
            }

            // Last exit status
            if (*(arg + 1) == '?') {
                char buf[255];
                sprintf(buf, "%d", shell->last_exit_status);
                push_str(&s, buf);

                arg += 2;
                continue;
            }

            // Otherwise this is just a normal solitary $ which should be added as a char
        }

        // Replace unescaped ~ with HOME
        if (*arg == '~' && !escape_next && quote_char != '\'') {
            push_str(&s, shell_get_env_var(shell, "HOME", true));
            arg++;
            continue;
        }

        // If there is a `'` and we're not escaped and we're not in "
        if(*arg == '\'' && !escape_next && quote_char != '"') {
            // This single quote close the previous one
            if (quote_char == '\'') {
                quote_char = '\0';
            } else {
                // This is the start of a new quote
                quote_char = '\'';
            }

            arg++;
            continue;
        }

        // If there is a `"` and we're not escaped and we're not in '
        if (*arg == '"' && !escape_next && quote_char != '\'') {
            // This double quote close the previous one
            if (quote_char == '\"') {
                quote_char = '\0';
            } else {
                // This is the start of a new quote
                quote_char = '\"';
            }

            arg++;
            continue;
        }

        // Expand new lines
        if (*arg == 'n' && escape_next) {
            push_char(&s, '\n');

            arg++;
            escape_next = false;
            continue;
        }

        // Expand tabs
        if (*arg == 't' && escape_next) {
            push_char(&s, '\t');

            arg++;
            escape_next = false;
            continue;
        }

        // Add the current char and then move on to the next
        push_char(&s, *arg);
        arg++;

        // Escape wasn't used
        escape_next = false;
    }

    // Unescaped quote
    if(quote_char != '\0') {
        fprintf(stderr, "Syntax error unescaped `%c` in `%s`\n", quote_char, arg);
        return NULL;
    }

    return s.str;
}

/// Expands shell variables and removes quotes (context aware) - returns true if syntactically correct and false otherwise
bool sanitise_arguments(char **args, Shell *shell) {
    for(int i = 0; args[i] != NULL; i++) {
        char* arg = sanitise_argument(args[i], shell);
        free(args[i]);

        if (arg == NULL) {
            return false;
        }

        args[i] = arg;
    }

    return true;
}

pid_t run_executor(Executor *executor, Shell *shell, int fd_in, int fd_out, int fd_close, int fd_log) {
    char** args = executor->args;

    // Removes outer ", ' and inserts values into unescaped shell variables
    if(!sanitise_arguments(args, shell)) {
        fprintf(stderr, "Internal shell error: the previous error should have been caught at parse stage\n");
        exit(1);
    }

    CommandType command_type = get_command_type(args[0]);

    CommandLocation *cmd_loc = which(shell, command_type, args[0]);

    int built_in_status = 0;

    // We have to run built-ins on this thread and then in the fork we simply exit with this status
    if (cmd_loc != NULL && cmd_loc->built_in != NULL) {
        built_in_status = execute_built_in(shell, executor->args, fd_log);
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

        // Sleep for a very short amount of time to ensure that parent is ready to start listening for this PID
        struct timespec t;
        t.tv_sec = 0;
        // 1 ms
        t.tv_nsec = 1 * 1000000;
        nanosleep(&t, NULL);

        if (cmd_loc == NULL) {
            fd_print(fd_log, 3, "Unknown command `", executor->args[0], "`\n");
            exit(1);
        }

        if (cmd_loc->path != NULL) {
            execvp(executor->args[0], executor->args);

            fd_print(fd_log, 3, "Failed to exec process ", executor->args[0], "\n");
            exit(1);
        } else if (cmd_loc->built_in != NULL) {
            // This just needs to exit with the status from earlier
            exit(built_in_status);
        } else {
            fd_print(fd_log, 1,
                     "INVARIANT reached, cmd was not NULL but both path AND built_in were - should be impossible\n");
            exit(1);
        }
    } else {
        if (pid == -1) {
            fd_print(fd_log, 1, "Couldn't fork process\n");
            exit(1);
        }

        return pid;
    }
}


