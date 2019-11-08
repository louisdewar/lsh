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
    char* c = *args;

    // Remove leading whitespace
    while(*c == ' ') {
        c++;
    }

    char* start = c;

    // The purpose of this isn't to help editing the string and preform the escaping, it's just to decide whether the next char should be escaped
    bool escape_next = false;
    while (*c != '\0') {
        if (*c == '\\') {
            if (escape_next) {
                escape_next = false;
            } else {
                escape_next = true;
            }

            c++;
            continue;
        }

        if(*c == '\'' || *c == '"') {
            char quote = *c;
            c++;

            while(*c != quote) {
                if (*c == '\0') {
                    return "Reached EOF with unmatched quote";
                }

                c++;
            }
        }

        // Check for control chars
        if(!escape_next && (*c == '|' || *c == ';' || *c == '&')) {
            break;
        }

        if (!escape_next && (*c == ' ')) {
            string_vector_append_n(vec, start, c - start);

            // Remove extra whitespace
            while(*c == ' ') {
                c++;
            }

            start = c;
            continue;
        }

        // Increment the char pointer and go around again
        c++;
    }

    // Either we reached a control char or NULL, if there is a string from the start up to (not including) the current char we want to append it
    if(start != c) {
        string_vector_append_n(vec, start, c - start);
    }

    if (vec->len == 0) {
        return "Empty string";
    } else {
        *args = c;
        return NULL;
    }
}

ExecutionPlan *parse_line(char *line) {
    ExecutionPlan *root_execution_plan = NULL;
    ExecutionPlan *prev_execution_plan = NULL;
    char *control_char = "\0";

    while(*line != '\0') {
        StringVector vec = new_string_vector(1);
        char* parse_err = parse_arguments(&vec, &line);

        if (parse_err != NULL) {
            printf("Error parsing statement: %s\n", parse_err);
            return NULL;
        }

        Executor* executor = new_executor(vec.ptr);
        ExecutionPlan* execution_plan = new_execution_plan(executor);

        // Match the control char linking this command to the *previous* one
        switch (*control_char) {
            case '\0':
                root_execution_plan = execution_plan;
                break;
            case ';':
                prev_execution_plan->next = execution_plan;
                prev_execution_plan->connection = CONNECTION_AFTER;
                break;
            case '|':
                prev_execution_plan->next = execution_plan;
                if (*(control_char + 1) == '|') {
                    prev_execution_plan->connection = CONNECTION_ON_FAILURE;
                } else {
                    prev_execution_plan->connection = CONNECTION_PIPE;
                }
                break;
            case '&':
                prev_execution_plan->next = execution_plan;
                if (*(control_char + 1) == '&') {
                    prev_execution_plan->connection = CONNECTION_ON_SUCCESS;
                } else {
                    // TODO: Fix fork syntax so that it works even as the last char of the formula
                    prev_execution_plan->connection = CONNECTION_FORK;
                }
                break;
            default:
                printf("Unhandled control char '%c'\n", *control_char);
                return NULL;
        }

        // Store the pointer to the location of the char in the line
        control_char = line;

        // End of line
        if(*control_char == '\0') {
            break;
        }

        line++;

        // We should skip the next char because it is also a control char
        if (*line == '|' || *line == '&') {
            line++;
        }

        prev_execution_plan = execution_plan;
    }

    return root_execution_plan;

}
