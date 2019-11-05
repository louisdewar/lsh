//
// Created by Louis de Wardt
//

#include <unistd.h>

#include "builtins.h"

const char* built_ins[] = { "which", "cd", NULL };

CommandLocation cmd_loc_from_path(Path* path) {
    CommandLocation location = {

    };
}

CommandLocation which(Shell* shell, enum CommandType type, char* path) {
    switch (type) {
        case INVALID:
            return NULL;
        case ABSOLUTE:
            if(access(path, X_OK) != -1) {
                return new_path_from_str(path);
            } else {
                return NULL;
            }
        case RELATIVE: {
            Path *real_path = new_path_from_join(shell->working_directory, path);

            if (access(real_path->str, X_OK) != -1) {
                return real_path;
            } else {
                free_path(real_path);
                return NULL;
            }
        }
        case GLOBAL: {
            char* built_in = built_ins[0];

            do {

            } while(built_in++ != NULL);
        }
    }
}