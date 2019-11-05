//
// Created by Louis de Wardt
//


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "parse.h"
#include "executor.h"
#include "path.h"

int main(void) {
  printf("lsh v0.1\n");

  Path* wd = new_path_from_cwd();

  // Main loop
  while(true) {
    printf("lsh>>");
    // Max line chars is 4096
    char line[4096];

    if(fgets(line, 4096, stdin) == NULL) {
      perror("LINE TOO LONG");
      break;
    }

    // Replace the newline with a null char (terminate string early)
    line[strcspn(line, "\n")] = 0;

    Executor* executor = parse_line(line, wd);

    if (executor == NULL) {
      printf("NULL executor\n");
      continue;
    }

    print_executor(executor);

    printf("\n");
  }

  return 0;
}
