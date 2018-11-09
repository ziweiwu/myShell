//
// Created by ZIWEIWU on 2018-11-08.
//

#ifndef PROJECT_SMALLSH_H
#define PROJECT_SMALLSH_H

#include<stdio.h>
#include<stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include "signal_catchers.h"
#include "builtin_commands.h"

void if_used_builtin_commands(char *buffer);
void smallsh();

#endif //PROJECT_SMALLSH_H
