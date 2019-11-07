//
// Created by Louis de Wardt
//

#include "parse.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "executor.h"
#include "str_vec.h"

CommandType get_command_type(char *word) {
    PathType p_type = get_path_type(word);

    if (p_type == P_ABSOLUTE) {
        return ABSOLUTE;
    }

    // If string contains / then it must be relative and not global
    if (strchr(word, '/') != NULL) {
        return RELATIVE;
    }

    return GLOBAL;
}

// Parse all the arguments including the actual command (returns error message or NULL pointer for no error)
// It sets the char pointer to the end of the command (either '\0' or the control char e.g. |)
char* parse_arguments(StringVector *vec, char **args) {
    char *start = *args;
    char *end = *args;

    while (*start != '\0') {
        // Remove leading whitespace
        while (*start == ' ') {
            start++;
        }

        end = start;

        // Find end of this argument
        while(*end != '\0' && *end != ' ' && *end != '|' && *end != ';') {
            if (*end == '"' || *end == '\'') {
                char quote = *start;

                do {
                    end++;

                    // Reached EOF with unescaped quote
                    if (*end == '\0') {
                        return "Reached EOF with unmatched quote";
                    }
                } while(*end != quote || *(end - 1) == '\\');
            }

            end++;
        }

        int len = end - start;

        if(len > 0) {
            // Vec should include from start not including the end char
            string_vector_append_n(vec, start, len);
            start = end;
        } else {
            // We must have reached a control char
            break;
        }
    }

    if (vec->len == 0) {
        return "Empty string";
    } else {
        *args = end;
        return NULL;
    }
}

ExecutionPlan *parse_line(char *line) {
    ExecutionPlan *root_execution_plan = NULL;
    ExecutionPlan *prev_execution_plan = NULL;
    char control_char = '\0';

    while(*line != '\0') {
        StringVector vec = new_string_vector(1);
        char* parse_err = parse_arguments(&vec, &line);

        if (parse_err != NULL) {
            printf("Error parsing statement: %s\n", parse_err);
            return NULL;
        }

        // Get the type of command path
        enum CommandType type = get_command_type(vec.ptr[0]);


        Executor* executor = new_executor(type, vec.ptr);
        ExecutionPlan* execution_plan = new_execution_plan(executor);

        switch (control_char) {
            case '\0':
                root_execution_plan = execution_plan;
                break;
            case '|':
                prev_execution_plan->next = execution_plan;
                prev_execution_plan->connection = CONNECTION_PIPE;
                break;
            case ';':
                prev_execution_plan->next = execution_plan;
                prev_execution_plan->connection = CONNECTION_AFTER;
                break;
            default:
                printf("Unhandled control char '%c'\n", control_char);
                return NULL;
        }

        control_char = *line;

        // End of line
        if(control_char == '\0') {
            break;
        }

        line++;

        prev_execution_plan = execution_plan;
    }

    return root_execution_plan;

}
