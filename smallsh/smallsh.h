//
// Created by ZIWEIWU on 2018-11-08.
//

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

#define MAX_COMMAND_LENGTH 2050
#define MAX_NUM_ARGS 512

int is_blank_line(char *, int);
void check_background_processes(pid_t *, int, int *);
void catch_SIGTSTP(int);

void smallsh();

#endif  // PROJECT_SMALLSH_H
