/**
 * Filename:    builtin_commands.c
 * Filetype:    source 
 * Author:      Wu, Ziwei
 * Class:       CS371
 * Program:     3
 * Description: contains three builtin commands function 
 *              for smallsh including cd, status, and exit
 *              commands
 */

#include "builtin_commands.h"

/**
 * change directory given a directory path
 */
void cd_command(char* path) {
  int ret;
  // if no path is given, cd to home dir
  if (!path) {
    ret = chdir(getenv("HOME"));
  } 
  // cd to dir given by path 
  else {
    ret = chdir(path);
  }

  // if cd failed
  if (ret==-1) {
    perror("cd command failed");
  }
}

/*
 * print the exit or termination status of last foreground process 
 */
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
}

/*
*perform cleanup of leftover process and exit the shell
*/
void exit_command(pid_t* child_pid_array, int child_pid_array_size) {
  int i;
  //ensure no background processes are left running  
  for (i = 0; i < child_pid_array_size; i++) {
    kill(*(child_pid_array + i), 15);
  }
  exit(0);
}
