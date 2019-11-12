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
#include "execution_plan.h"

// Define the list of built ins so we know whether it's a built in or if we have to search the PATH
const char *built_ins[] = {"which", "cd", "pwd", "exit", "export", NULL};

CommandLocation *cmd_loc_from_path(Path *path) {
    CommandLocation *loc = malloc(sizeof(CommandLocation));

    loc->path = path;
    loc->built_in = NULL;

    return loc;
}

// This also checks the path exists and is executable, return NULL if it isn't
CommandLocation *cmd_loc_from_path_checked(Path *path) {
    struct stat stat_buf;

    if (stat(path->str, &stat_buf) == 0 && (stat_buf.st_mode & S_IXUSR) && S_ISREG(stat_buf.st_mode)) {
        CommandLocation *loc = cmd_loc_from_path(path);

        return loc;
    }

    if (errno == EACCES) {
        printf("You do not have permission to access path: `%s`\n", path->str);
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
            path_insert_home(path, shell_get_env_var(shell, "HOME", true));
            return cmd_loc_from_path_checked(path);
        }

        case RELATIVE: {
            Path *real_path = new_path_from_join(shell->working_directory, path_str);

            CommandLocation *loc = cmd_loc_from_path_checked(real_path);


            if (loc != NULL) {
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

            char *path_start = shell_get_env_var(shell, "PATH", true);

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
        default: {
            fprintf(stderr, "which: invalid code path\n");
            exit(1);
        }
    }
}

// Expects str in the form name=value
int export(Shell* shell, char* str, int fd_log) {
    char* name_end = strchr(str, '=');

    if (name_end == NULL) {
        fd_print(fd_log, 1, "export: argument must be key=value\n");
        return 1;
    }

    HashMapEntry* entry = new_hashmap_entry_slice(str, name_end - str, name_end + 1, strlen(name_end + 1));

    setenv(entry->key, entry->value, 1);
    hashmap_insert_or_update(shell->env_vars, entry);

    return 0;
}

int execute_built_in(Shell *shell, char** args, int fd_log) {
    char *cmd = args[0];

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
            path_insert_home(new_path, shell_get_env_var(shell, "HOME", true));

            struct stat meta;

            if (stat(new_path->str, &meta) == 0 && meta.st_mode & S_IFDIR) {
                free_path(shell->working_directory);
                shell->working_directory = new_path;
                chdir(shell->working_directory->str);
                return 0;
            } else {
                fd_print(fd_log, 3, "cd: Either you don't have the required permissions or the directory `", new_path->str, "` doesn't exist\n");
                return 1;
            }
        } else {
            fd_print(fd_log, 1, "cd: path not specified\n");
            return 1;
        };
    } else if (strcmp(cmd, "pwd") == 0) {
        printf("%s\n", shell->working_directory->str);
        return 0;
    } else if (strcmp(cmd, "which") == 0) {
        char *path = args[1];
        if (path != NULL) {
            // Get the command type of the first argument
            CommandType type = get_command_type(args[1]);

            CommandLocation *loc = which(shell, type, path);

            if (loc == NULL) {
                fd_print(fd_log,3, "which: can't find command `", path, "`\n");
                return 1;
            } else if (loc->built_in != NULL) {
                printf("`%s` is a shell built in\n", loc->built_in);
                return 0;
            } else if (loc->path != NULL) {
                printf("%s\n", loc->path->str);
                return 0;
            } else {
                fd_print(fd_log, 1, "Internal shell error\n");
                return 1;
            }
        }

        return 1;
    } else if (strcmp(cmd, "exit") == 0) {
        // TODO: parse status and exit with it
        shell->running = false;
        return 0;
    } else if (strcmp(cmd, "export") == 0) {
        if (args[1] != NULL) {
            return export(shell, args[1], fd_log);
        } else {
            fd_print(fd_log, 1, "export: variable not specified\n");
        }
    }
    /*else if (strchr(cmd, '=') != NULL) {
        char** next_cmd = args+1;

        if (*next_cmd != NULL) {
            char* name_end = strchr(cmd, '=');
            HashMapEntry* entry = new_hashmap_entry_slice(cmd, name_end - cmd, name_end + 1, strlen(name_end + 1));

            // This is just a pointer to the value which we're about to overwrite so we need to make a copy so we can set it back to the original value after
            char* prev_value_ptr = hashmap_get_value(shell->env_vars, entry->key);
            char *prev_value;
            if (prev_value_ptr != NULL) {
                prev_value = malloc(sizeof(char) * (strlen(prev_value_ptr) + 1));
                strcpy(prev_value, prev_value_ptr);
            } else {
                // Set to empty string
                prev_value = malloc(sizeof(char) * 1);
                *prev_value = '\0';
            }

            setenv(entry->key, entry->value, 1);
            hashmap_insert_or_update(shell->env_vars, entry);

            // Run the rest of the commands after this as a command then reset env var after it finishes
            // Fd in and out are just default since this process should have already been redirected properly
            pid_t pid = run_executor(new_executor(next_cmd), shell, 0, 1, -1, fd_log);
            int status = wait_for_pid_exit(pid, fd_log);

            // Reset env var back to original value, because we already have a pointer to the new entry we can just reset it
            free(entry->value);
            entry->value = prev_value;
            setenv(entry->key, entry->value,  1);
            return status;
        }

        return 0;
    }*/ else {
        printf("Unrecognised built in `%s`, this should not have occurred\n", cmd);
        exit(1);
    }

}