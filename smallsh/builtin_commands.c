//
// Created by ZIWEIWU on 2018-11-08.
//
#include "builtin_commands.h"

void cd_command(char* path, int* exit_status) {
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
    *exit_status = 1;
  } else {
    *exit_status = 0;
  }
  return;
}

// print the exit or termination status  
void status_command(int *exit_status) {
  int status; 
  //if process is exited
  if (WIFEXITED(*exit_status)) {
    status = WEXITSTATUS(*exit_status);
    printf("exit value %d\n", status);
    fflush(stdout);
  }
  //if process is terminated by signal
  if (WIFSIGNALED(*exit_status)) {
    status = WTERMSIG(*exit_status);
    printf("terminated by signal %d\n", status);
    fflush(stdout);
  }

  //set exit status for status command itself 
  *exit_status = 0;
  return;
}

// exit the shell
void exit_command(pid_t* child_pid_array, int child_pid_array_size) {
  int i;
  
  //ensure no background processes are left running  
  for (i = 0; i < child_pid_array_size; i++) {
    /*printf("kill process %d\n", *(child_pid_array + i));*/
    kill(*(child_pid_array + i), 15);
  }
  /*printf("All background processes are killed\n");*/
  /*printf("Smallsh is exiting\n");*/
  exit(0);
}
