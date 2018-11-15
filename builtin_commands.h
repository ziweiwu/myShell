/**
 * Filename:    builtin_commands.h
 * Filetype:    header 
 * Author:      Wu, Ziwei
 * Description: Contains three builtin commands function 
 *              for smallsh including cd, status, and exit
 *              commands
 */

#ifndef BUILTIN_COMMANDS_H
#define BUILTIN_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void cd_command(char *);
void status_command(int *);
void exit_command(pid_t *, int);

#endif
