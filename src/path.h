//
// Created by Louis de Wardt
//

#pragma once

typedef struct Path {
    int cap;
    int len;
    char* str;
} Path;

typedef enum PathType {
    P_RELATIVE,
    P_ABSOLUTE,
} PathType;

Path* new_path_from_cwd();
Path* new_path_from_str(char*);
Path* new_path_from_join(Path*, char*);
Path* new_path_from_str_slice(char*, int);

PathType get_path_type(char*);

char* get_path_last_segment(Path*);

void path_join(Path*, char*);
void path_insert_home(Path*, char*);

void free_path(Path*);