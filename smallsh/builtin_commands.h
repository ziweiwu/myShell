//
// Created by ZIWEIWU on 2018-11-08.
//

#ifndef BUILTIN_COMMANDS_H
#define BUILTIN_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void cd_command(char *);
int get_status(int);
void exit_command(pid_t *, int);

#endif
