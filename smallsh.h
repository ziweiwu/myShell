/**
 * Filename:    smallsh.h
 * Filetype:    header 
 * Author:      Wu, Ziwei
 * Class:       CS371
 * Program:     3
 * Description: smallsh function which starts up the shell,
 *              and execute shell functionalities, including
 *              three builtin commands cd, status, and exit.
 *              It also executes a shell command by using
 *              exec. Another feature is that it performs
 *              expansion of string `$$` into the shell
 *              process ID
 */

#ifndef PROJECT_SMALLSH_H
#define PROJECT_SMALLSH_H

#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "builtin_commands.h"

//#define MAX_COMMAND_LENGTH 2050
//#define MAX_NUM_ARGS 512
//#define MAX_CHILDREN_ARRAY_SIZE 1000
//#define SHELL_PID getpid()

int is_blank_line(char *, int);
char * str_replace(char *, char *, char *);
void check_background_processes(pid_t *, int, int *);
void catch_SIGTSTP(int);
  
void smallsh();

#endif  // PROJECT_SMALLSH_H
