//
// Created by ZIWEIWU on 2018-11-08.
//
#include "builtin_commands.h"

void cd_command(char* path) {
  int ret;

  // if cd has no argument, cd to home directory
  if (path == NULL) {
    /*printf("cd to %s\n", getenv("HOME"));*/
    ret = chdir(getenv("HOME"));
  } else {
    /*printf("cd to %s\n", path);*/
    ret = chdir(path);
  }

  if (ret == -1) {
    perror("chdir failed");
    exit(1);
  }
  exit(0);
}

int get_status(int status) {
  if (WIFEXITED(status)) {
    status= WEXITSTATUS(status);
  }  
  if (WIFSIGNALED(status))
	{
    status= WTERMSIG(status);
	}
  return status;
}

void exit_command(pid_t* child_pid_array, int child_count) {
  int i;
  for (i = 0; i < child_count; i++) {
    /*printf("kill process %d\n", *(child_pid_array + i));*/
    kill(*(child_pid_array + i), 15);
  }
  /*printf("All processes are killed\n");*/
  /*printf("Smallsh is exiting\n");*/
  exit(0);
}
