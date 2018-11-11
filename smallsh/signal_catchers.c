//
// Created by ZIWEIWU on 2018-11-08.
//

#include "signal_catchers.h"

void catch_SIGTSTP(int signo) {
  char* message = "Entering foreground-only mode (& is now ignored)\n";
  write(STDOUT_FILENO, message, 49);
}

