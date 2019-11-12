//
// Created by Louis de Wardt on 05/11/2019.
//

#include "shell.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "growable_string.h"

/// Returns a pointer to the value of the environment variable (NULL if doesn't exist)
char *shell_get_env_var(Shell *shell, char *key, bool required) {
    char *value = hashmap_get_value(shell->env_vars, key);

    if (required && value == NULL) {
        fprintf(stderr, "Internal shell error: `%s` was expected to be a key in ENV VAR but it didn't exist\n", key);
        exit(1);
    }

    return value;
}

// Ended up not being needed because execvpe isn't defined on my setup (just use setenv() whenever there's a change instead)
StringVector get_all_env_vars(Shell* shell) {
    HashMap* map = shell->env_vars;
    StringVector vec = new_string_vector(map->element_count);

    int max_unique_keys = pow_2(map->table_ptr_size);

    for (int i = 0; i < max_unique_keys; i++) {
        HashMapEntry* entry = map->table[i];
        while (entry != NULL) {
            String env_var = new_string(NULL);
            // All these appends are allocating
            push_str(&env_var, entry->key);
            push_char(&env_var, '=');
            push_str(&env_var, entry->value);
            string_vector_append(&vec, env_var.str);

            // Move to next entry in linked list
            entry = entry->next;
        }
    }

    return vec;
}

Shell *new_shell(char **envp) {
    Shell *shell = malloc(sizeof(Shell));

    // Initialise with 4 bit mask (16 unique keys)
    HashMap *env_vars = new_hashmap(4);

    while (*envp != NULL) {
        char *start = *envp;
        char *end = strchr(start, '=');

        // Shouldn't happen but just in case
        if (end == NULL) {
            continue;
        }

        int key_len = end - start;
        char *value = end + 1;

        hashmap_insert_or_update(env_vars, new_hashmap_entry_slice(start, key_len, value, strlen(value)));

        envp++;
    }

    fprintf(stderr, "Hashmap element/occupied key ratio = %.4f\n",
            (double) env_vars->occupancy / (double) env_vars->element_count);


    shell->working_directory = new_path_from_cwd();
    shell->running = true;
    shell->last_exit_status = 0;
    shell->env_vars = env_vars;

    printf("Home %s\n", shell_get_env_var(shell, "HOME", true));

    // Ensure PATH is set
    shell_get_env_var(shell, "PATH", true);


    return shell;
}