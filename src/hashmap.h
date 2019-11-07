//
// Created by Louis de Wardt on 07/11/2019.
//

#pragma once

typedef struct HashMapEntry {
    char* key;
    char* value;
    struct HashMapEntry* next;
} HashMapEntry;

typedef struct HashMap {
    HashMapEntry** table;
    // The number of bits in a pointer to a table
    short table_ptr_size;
    // The number of occupied entry slots in the table
    int occupancy;
    // The number of elements (sum of each entry)
    int element_count;
    // The magic number
    long m;
} HashMap;

/// A good default for table_ptr_size is 4 which means 16 unique locations
HashMap* new_hashmap(short);

/// Creates a new hashmap entry (copies strings onto heap)
HashMapEntry* new_hashmap_entry(char*, char*);
/// Creates a new hashmap entry (copies string slices onto heap)
HashMapEntry *new_hashmap_entry_slice(char *key, int key_len, char *value, int value_len);

void hashmap_insert_or_update(HashMap *map, HashMapEntry *new_entry);
int hashmap_entry_insert_or_update(HashMapEntry *prev_entry, HashMapEntry *new_entry);

char* hashmap_get_value(HashMap*, char*);

long hash_str(char*, long);

void run_hash_eval(char**);