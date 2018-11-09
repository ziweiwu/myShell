//
// Created by ZIWEIWU on 2018-11-08.
//

#include "signal_catchers.h"


void catch_SIGTERM(int signo){
  char * message = "Exit command is called. Exiting\n";
  write(STDOUT_FILENO, message, 32);
  raise(SIGTERM);
}
