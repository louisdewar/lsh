//
// Created by Louis de Wardt
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

#include "builtins.h"
#include "parse.h"

const char *built_ins[] = {"which", "cd", "pwd", "exit", NULL};

CommandLocation *cmd_loc_from_path(Path *path) {
    CommandLocation *loc = malloc(sizeof(CommandLocation));

    loc->path = path;
    loc->built_in = NULL;

    return loc;
}

// This also checks the path exists and is executable, return NULL if it isn't
CommandLocation *cmd_loc_from_path_checked(Path *path) {
    struct stat statbuf;

    if (stat(path->str, &statbuf) == 0 && (statbuf.st_mode & S_IXUSR) && S_ISREG(statbuf.st_mode)) {
        CommandLocation *loc = cmd_loc_from_path(path);

        return loc;
    }

    if (errno == EACCES) {
        printf("You do not have permission to access path: `%s`", path->str);
    }

    return NULL;
}

CommandLocation *cmd_loc_from_built_in(char *built_in) {
    CommandLocation *loc = malloc(sizeof(CommandLocation));

    loc->built_in = built_in;
    loc->path = NULL;

    return loc;
}

CommandLocation *which(Shell *shell, CommandType type, char *path_str) {
    switch (type) {
        case ABSOLUTE: {
            Path *path = new_path_from_str(path_str);
            path_insert_home(path, shell->home);
            return cmd_loc_from_path_checked(path);
        }

        case RELATIVE: {
            Path *real_path = new_path_from_join(shell->working_directory, path_str);

            CommandLocation *loc = cmd_loc_from_path_checked(real_path);

            if (loc == NULL) {
                return loc;
            } else {
                free_path(real_path);
                return NULL;
            }
        }
        case GLOBAL: {
            // Check to see if it's a built in command
            for (int i = 0; built_ins[i] != NULL; i++) {
                if (strcmp(built_ins[i], path_str) == 0) {
                    return cmd_loc_from_built_in(path_str);
                }
            }

            char *path_start = shell->PATH;

            // Search the path
            while (*path_start != '\0') {
                char *path_end = strchr(path_start, ':');

                int len = 0;
                if (path_end == NULL) {
                    len = strlen(path_start);
                } else {
                    len = path_end - path_start;
                }

                Path *path = new_path_from_str_slice(path_start, len);
                path_join(path, path_str);

                CommandLocation *loc = cmd_loc_from_path_checked(path);

                if (loc != NULL) {
                    return loc;
                }

                free_path(path);

                path_start = path_start + len + 1;
            }

            return NULL;
        }
    }
}

int execute_built_in(Shell *shell, Executor *executor) {
    char *cmd = executor->command;
    char **args = executor->args;

    if (strcmp(cmd, "cd") == 0) {
        char *dir = args[1];

        if (dir != NULL) {
            Path *new_path = NULL;
            CommandType path_type = get_command_type(dir);
            if(path_type == ABSOLUTE) {
                new_path = new_path_from_str(dir);
            } else {
                new_path = new_path_from_join(shell->working_directory, dir);
            }
            path_insert_home(new_path, shell->home);

            struct stat meta;

            if (stat(new_path->str, &meta) == 0 && meta.st_mode & S_IFDIR) {
                free_path(shell->working_directory);
                shell->working_directory = new_path;
                chdir(shell->working_directory->str);
                return 0;
            } else {
                printf("cd: Either you don't have the required permissions or the directory `%s` doesn't exist\n",
                       new_path->str);
                return 1;
            }
        } else {
            printf("cd: path not specified\n");
            return 1;
        };
    } else if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", shell->working_directory->str);
        return 0;
    } else if (strcmp(cmd, "which") == 0) {
        char *path = args[1];
        if (path != NULL) {
            CommandType type = get_command_type(args[0]);

            CommandLocation *loc = which(shell, type, path);

            if (loc == NULL) {
                printf("which: can't find command `%s`\n", path);
                return 1;
            } else if (loc->built_in != NULL) {
                printf("`%s` is a shell built in\n", loc->built_in);
                return 0;
            } else if (loc->path != NULL) {
                printf("%s", loc->path->str);
                return 0;
            } else {
                printf("Internal shell error\n");
                return -1;
            }
        }

        return -1;
    } else if (strcmp(cmd, "exit") == 0) {
        shell->running = false;
        return 0;
    } else {
        printf("Unrecognised built in `%s`, this should not have occurred\n", cmd);
        exit(1);
    }

}