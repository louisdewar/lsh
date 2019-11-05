//
// Created by Louis de Wardt
//

#pragma once

typedef struct Path {
    int cap;
    int len;
    char* str;
} Path;

Path* new_path_from_cwd();
Path* new_path_from_str(char*);
Path* new_path_from_join(Path*, char*);
Path* new_path_from_str_slice(char*, int);

void path_join(Path*, char*);
void insert_home(Path*, char*);

void free_path(Path*);