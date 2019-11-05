project(lsh)

set(SOURCE_FILES src/main.c src/parse.c src/executor.c src/builtins.c src/builtins.h src/path.c src/path.h src/shell.c src/shell.h)

add_executable(lsh ${SOURCE_FILES})