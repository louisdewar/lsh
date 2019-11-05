//
// Created by Louis de Wardt
//

#include "parse.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "executor.h"

enum CommandType get_command_type(char* word) {
  // This is guaranteed to be at least one
  unsigned long len = strlen(word);

  // it is only possible for this to be GLOBAL or RELATIVE
  if (len == 1) {
    if(word[0] == '/') {
      printf("'/' is not a valid command");
      return NULL;
    } else {
      return GLOBAL;
    }
  }


  // This can not be RELATIVE - need './{path}'
  if (len == 2) {
    if(word[0] == '/') {
      return ABSOLUTE;
    } else if (word[0] == '.') {
        printf("Invalid path '%s'", word);
        return NULL;
    } else {
        return GLOBAL;
    }
  }

  if (word[0] == '.' && word[1] == '/') {
      return RELATIVE;
  }

  if (word[0] == '/') {
      return ABSOLUTE;
  }

  // If string contains / then it must be relative and not global
  if (strchr(word, '/') != INVALID) {
      return RELATIVE;
  }

  return GLOBAL;
}

Executor* parse_line(char* line, Path* working_directory) {
  printf("Parsing line: '%s'\n", line);

  // Remove leading whitespace
  while(*line == ' ') {
      line++;
  }

  if (*line == '\0') {
    return INVALID;
  }

  char* first_word = line;

  // Find the start of args
  char* args = strchr(line, ' ');
  if (args != NULL) {
      // Terminate first word string
      *args = '\0';
      // Args should start after the null char;
      args++;
  } else {
      // Set args to be an empty string since there are none (NULL ptr would cause errors later on)
      args = "";
  }

  printf("First word '%s'\n", first_word);
  printf("Line '%s'\n", line);

  // Get the type of path
  enum CommandType type = get_command_type(first_word);

  if (type == INVALID) {
      return INVALID;
  } else {
      return new_executor(type, first_word, args);
  }
}
