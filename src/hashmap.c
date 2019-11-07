//
// Created by Louis de Wardt on 07/11/2019.
//

#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "util.h"

#define LARGE_PRIME 12007849013

HashMapEntry **new_empty_hashmap_table(int table_size) {
    HashMapEntry **table = calloc(sizeof(HashMapEntry *), table_size);
    if (table == NULL) {
        fprintf(stderr, "Couldn't allocate hashmap table");
        exit(1);
    }

    // Initialise memory to null (we need this to be true for hashmap logic to work)
    for (int i = 0; i < table_size; i++) {
        table[i] = 0;
    }

    return table;
}

HashMap *new_hashmap(short table_ptr_size) {
    if (table_ptr_size < 1) {
        table_ptr_size = 1;
    }

    HashMap *map = malloc(sizeof(HashMap));

    if (map == NULL) {
        fprintf(stderr, "Couldn't allocate hashmap");
        exit(1);
    }

    map->table_ptr_size = table_ptr_size;

    map->table = new_empty_hashmap_table(pow_2(table_ptr_size));

    map->occupancy = 0;
    map->element_count = 0;
    map->m = LARGE_PRIME;

    return map;
}

int hashmap_key_to_index(HashMap* map, char* key) {
    // 2^n - 1 = 1111.. n times
    int key_mask = pow_2(map->table_ptr_size) - 1;

    return hash_str(key, map->m) & key_mask;
}

void table_insert_or_update(HashMapEntry **table, int key_hash, HashMapEntry *new_entry, int *occupancy,
                            int *element_count) {
    HashMapEntry *entry = table[key_hash];

    if (entry == NULL) {
        table[key_hash] = new_entry;
        *occupancy += 1;
        *element_count += 1;
    } else {
        *element_count += hashmap_entry_insert_or_update(entry, new_entry);
    }
}

void double_table_size(HashMap* map) {
    int prev_max_unique_keys = pow_2(map->table_ptr_size);

    // This doubles the table size
    map->table_ptr_size += 1;

    int new_max_unique_keys = pow_2(map->table_ptr_size);

    HashMapEntry **new_table = new_empty_hashmap_table(new_max_unique_keys);

    int occupancy = 0;
    int element_count = 0;

    // Loop through all the old memory slots
    for(int key = 0; key < prev_max_unique_keys; key++) {
        HashMapEntry* entry = map->table[key];
        while(entry != NULL) {
            HashMapEntry* next_entry = entry->next;
            // Rehash and insert into new map
            table_insert_or_update(new_table, hashmap_key_to_index(map, entry->key), entry, &occupancy, &element_count);
            entry->next = NULL;

            entry = next_entry;
        }
    }

    // We should only free the actual table of pointers not the entries themselves since we've reused them
    free(map->table);

    map->table = new_table;
    map->occupancy = occupancy;
    map->element_count = element_count;
}

/// inserts/updates the value at key to entry if it exists or creates entry if it doesn't (updates occupancy rate)
void hashmap_insert_or_update(HashMap *map, HashMapEntry *new_entry) {
    // The number of different possible keys
    int max_unique_keys = pow_2(map->table_ptr_size);

    table_insert_or_update(map->table, hashmap_key_to_index(map, new_entry->key), new_entry, &map->occupancy, &map->element_count);

    // We're getting close to capacity there are probably quite a few collisions, so we should increase table size (double) and re-hash
    if (map->element_count >= .75 * max_unique_keys) {
        double_table_size(map);
    }

    // TODO: If we want we can add a check for element_count / occupancy (the closer this is to 0 the worse our collisions so we should change magic number and re-hash)
}

/// Prev_entry and new_entry cannot be NULL
/// Returns 0 if it updated and 1 if it inserted (insert count)
int hashmap_entry_insert_or_update(HashMapEntry *prev_entry, HashMapEntry *new_entry) {
    // Keep going until we find a pre-existing entry with the key or we get to end (so must append new one)
    while (1) {
        // Key already exists so update
        if (strcmp(prev_entry->key, new_entry->key) == 0) {
            // Free the previous value memory
            free(prev_entry->value);

            prev_entry->value = new_entry->value;

            // We don't need this memory any more (only value is kept)
            free(new_entry);
            free(new_entry->key);

            // Updated
            return 0;
        }

        if (prev_entry->next != NULL) {
            prev_entry = prev_entry->next;
        } else {
            // We have reached end without finding our key so we should just append
            break;
        }
    };

    prev_entry->next = new_entry;
    // Inserted
    return 1;
}

HashMapEntry *new_hashmap_entry(char *key, char *value) {
    HashMapEntry *entry = malloc(sizeof(HashMapEntry));

    entry->key = calloc(sizeof(char), strlen(key) + 1);
    strcpy(entry->key, key);

    entry->value = calloc(sizeof(char), strlen(value) + 1);
    strcpy(entry->value, value);

    entry->next = NULL;

    return entry;
}

HashMapEntry *new_hashmap_entry_slice(char *key, int key_len, char *value, int value_len) {
    HashMapEntry *entry = malloc(sizeof(HashMapEntry));

    entry->key = calloc(sizeof(char), key_len + 1);
    strncpy(entry->key, key, key_len);
    entry->key[key_len] = '\0';

    entry->value = calloc(sizeof(char), value_len + 1);
    strncpy(entry->value, value, value_len);
    entry->value[value_len] = '\0';

    entry->next = NULL;

    return entry;
}

/// Returns a pointer to the value of the string (NULL if doesn't exist)
char* hashmap_get_value(HashMap* map, char* key) {
    int key_hash = hashmap_key_to_index(map, key);

    HashMapEntry* entry = map->table[key_hash];

    if (entry == NULL) {
        return NULL;
    }

    while(entry->next != NULL) {
        // Check if this is the actual entry
        if (strcmp(key, entry->key) == 0) {
            return entry->value;
        }

        entry = entry->next;
    }

    // Check last entry
    if (strcmp(key, entry->key) == 0) {
        return entry->value;
    }

    // Although the hash was the same there was no entry with the actual key
    return NULL;
}

// The golden ratio fraction part (excluding the  1.)
#define GOLDEN_RATIO_FRACTIONAL 61803398875;

// Attribution: Algorithm adapted from ch 43 of The New Turing Omnibus by A.K. Dewdney (which I believe comes from D.E. Knuth Art of Programming Volume 3)
// Also some of the general ideas about hashmaps from a lecture about data structures from Mark Handley at UCL

// Just needs to be 'good enough' to somewhat work
long hash_str(char *str, long m) {
    long hash = 0;
    while (*str != '\0') {
        long k = ((long) *str) * GOLDEN_RATIO_FRACTIONAL;
        hash += k;

        str++;
    }

    return hash % m;
}

// Quick check to make sure the hash function is preforming reasonably well
void run_hash_eval(char **args) {
    while (*args != NULL) {
        char *start = *args;
        char *end = strchr(start, '=');

        if (end == NULL) {
            fprintf(stderr, "No = found in %s\n", start);
            continue;
        }

        int len = end - start;
        char *str = calloc(sizeof(char), len + 1);
        strncpy(str, start, len);
        str[len] = '\0';

        fprintf(stderr, "h(%s)=", str);
        // Magic number is a large prime
        int hash = hash_str(str, LARGE_PRIME);
        // we only want the last 8 bits
        printf("%i\n", hash & 0b1111);

        args++;
    }
}