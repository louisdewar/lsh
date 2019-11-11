project(lsh)

set(SOURCE_FILES src/main.c src/parse.c src/executor.c src/builtins.c src/builtins.h src/path.c src/path.h src/shell.c src/shell.h src/str_vec.c src/str_vec.h src/hashmap.c src/hashmap.h src/util.c src/util.h src/growable_string.c src/growable_string.h src/execution_plan.c src/execution_plan.h)

add_executable(lsh ${SOURCE_FILES})