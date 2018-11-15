/**
 * Filename:    builtin_commands.c
 * Filetype:    source
 * Author:      Wu, Ziwei
 * Description: Contains the implementation for three builtin commands
 *              for smallsh including cd, status, and exit
 */

#include "builtin_commands.h"

/**
 * Function: cd_command
 * Description: Change directory to directory specified by the user
 * Param1: A char pointer to the path of directory
 * Return: None
 */
void cd_command(char* path) {
  int ret;
  // If no path is given, cd to home directory
  if (!path) {
    ret = chdir(getenv("HOME"));
  }
  // cd to directory given by path
  else {
    ret = chdir(path);
  }

  // If cd failed, print out the error
  if (ret == -1) {
    perror("cd command failed");
  }
}

/**
 * Function: status_command
 * Description: Print the exit or termination status of the last foreground
 *              process that was exited or terminated
 * Param1: An int pointer points the exit status
 * Return: None
 */
void status_command(int* exit_status) {
  int status = -5;
  // Process is exited, print the exit status
  if (WIFEXITED(*exit_status)) {
    status = WEXITSTATUS(*exit_status);
    printf("exit value %d\n", status);
    fflush(stdout);
  }
  // Process is terminated by signal, print the termination status
  if (WIFSIGNALED(*exit_status)) {
    status = WTERMSIG(*exit_status);
    printf("terminated by signal %d\n", status);
    fflush(stdout);
  }
}

/**
 * Function: exit_command
 * Description: Perform clean up of leftover processes, and exit the shell
 * Param1: An array of background children PID
 * Param1: The size of background children array
 * Return: None
 */
void exit_command(pid_t* background_children_array,
                  int background_children_array_size) {
  int i = 0;

  // Loop over background children array and kill off any left over processes
  for (i = 0; i < background_children_array_size; i++) {
    if (background_children_array[i] == 0) {
      continue;
    } else {
      kill(background_children_array[i], 15);
    }
  }
  exit(0);
}
