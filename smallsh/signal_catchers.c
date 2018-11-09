//
// Created by ZIWEIWU on 2018-11-08.
//

#include "signal_catchers.h"


void catch_SIGTERM(int sig){
  char * message = "Exit command is called. Exiting\n";
  write(STDOUT_FILENO, message, 32);
  fflush(stdout);
  exit(0);
}


void catch_SIGCHLD(int sig){
  char * message = "Child command is done working. Exit Child\n";
  write(STDOUT_FILENO, message, 32);
  fflush(stdout);
  exit(0);
}
