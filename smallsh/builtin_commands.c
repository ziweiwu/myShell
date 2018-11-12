//
// Created by ZIWEIWU on 2018-11-08.
//
#include "builtin_commands.h"

void cd_command(char* path, int* childExitStatus) {
  int ret;
  // if no path is given, cd to home dir
  if (!path) {
    ret = chdir(getenv("HOME"));
  } 
  // cd to dir given by path 
  else {
    ret = chdir(path);
  }

  // set exit status based on ret
  if (ret==-1) {
    perror("cd command failed");
    *childExitStatus = 1;
  } else {
    *childExitStatus = 0;
  }
}

// return the exit status  
int get_status(int status) {
  //if process is exited
  if (WIFEXITED(status)) {
    status = WEXITSTATUS(status);
  }
  //if process is terminated by signal
  if (WIFSIGNALED(status)) {
    status = WTERMSIG(status);
  }
  return status;
}

// print out the exit status
void status_command(int* status) {
  printf("exit status: %d\n", get_status(*status));
  fflush(stdout);
  *status = 0;
}

// exit the shell
void exit_command(pid_t* child_pid_array, int child_pid_array_size) {
  int i;
  
  //ensure no background processes are left running  
  for (i = 0; i < child_pid_array_size; i++) {
    printf("kill process %d\n", *(child_pid_array + i));
    kill(*(child_pid_array + i), 15);
  }
  printf("All background processes are killed\n");
  printf("Smallsh is exiting\n");
  exit(0);
}
