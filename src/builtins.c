//
// Created by Louis de Wardt
//

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "builtins.h"

const char* built_ins[] = { "which", "cd", NULL };

CommandLocation* cmd_loc_from_path(Path* path) {
    CommandLocation* loc = malloc(sizeof(CommandLocation));

    loc->path = path;
    loc->built_in = NULL;

    return loc;
}

CommandLocation* cmd_loc_from_built_in(char* built_in) {
    CommandLocation* loc = malloc(sizeof(CommandLocation));

    loc->built_in = built_in;
    loc->path = NULL;

    return loc;
}

CommandLocation* which(Shell* shell, CommandType type, char* path_str) {
    switch (type) {
        case INVALID:
            return NULL;
        case ABSOLUTE:
            if(access(path_str, X_OK) != -1) {
                return cmd_loc_from_path(new_path_from_str(path_str));
            } else {
                return NULL;
            }
        case RELATIVE: {
            Path *real_path = new_path_from_join(shell->working_directory, path_str);

            if (access(real_path->str, X_OK) != -1) {
                return cmd_loc_from_path(real_path);
            } else {
                free_path(real_path);
                return NULL;
            }
        }
        case GLOBAL: {
            // Check to see if it's a built in command
            for(int i = 0; built_ins[i] != NULL; i++) {
                if (strcmp(built_ins[i], path_str) == 0) {
                    return cmd_loc_from_built_in(path_str);
                }
            }

            char* path_start = shell->PATH;

            // Search the path
            while(*path_start != '\0') {
                char* path_end = strchr(path_start, ':');

                int len;
                if (path_end == NULL) {
                    len = strlen(path_start);
                } else {
                    len = path_end - path_start;
                }


                Path* path = new_path_from_str_slice(path_start, len);
                path_join(path, path_str);

                if (access(path->str, F_OK | X_OK) == 0) {
                    return cmd_loc_from_path(path);
                }

                path_start = path_start + len + 1;
            }

            return NULL;
        }
    }
}