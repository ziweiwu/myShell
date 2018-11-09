//
// Created by ZIWEIWU on 2018-11-08.
//

#include "builtin_commands.h"

void cd_command(char *path) {
  int ret;

  printf("cd to %s\n", path);
  ret = chdir(path);

  if (ret == -1) {
    perror("chdir failed");
    exit(1);
  }
  if (ret == 0) {
    printf("Changed directory to %s\n", path);
  }
}

void status_command() { printf("Here are the status\n"); }

void exit_command() { printf("Smallsh is exiting\n"); }
