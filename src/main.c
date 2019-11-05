//
// Created by Louis de Wardt
//


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "parse.h"
#include "executor.h"
#include "shell.h"

int main(void) {
  printf("lsh v0.1\n");

  Shell* shell = new_shell(getenv("PATH"));

  // Main loop
  while(true) {
    printf("lsh>> ");
    // Max line chars is 4096
    char line[4096];

    if(fgets(line, 4096, stdin) == NULL) {
      perror("Couldn't read line");
      break;
    }

    // Replace the newline with a null char (terminate string early)
    line[strcspn(line, "\n")] = '\0';

    Executor* executor = parse_line(line, shell->working_directory);

    if (executor == NULL) {
      printf("NULL executor\n");
      continue;
    }

    print_executor(executor);
    run_executor(executor, shell);

    printf("\n");
  }

  return 0;
}
