//
// Created by ZIWEIWU on 2018-11-08.
//

#ifndef PROJECT_SIGNAL_CATCHERS_H
#define PROJECT_SIGNAL_CATCHERS_H

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void catch_SIGTERM(int);
void catch_SIGCHLD(int);

#endif //PROJECT_SIGNAL_CATCHERS_H
