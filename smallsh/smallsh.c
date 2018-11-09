//
// Created by ZIWEIWU on 2018-11-08.
//
#include "smallsh.h"

void smallsh() {
  printf("Smallsh running\n");

  // set up signals
  struct sigaction SIGINT_action = {0}, SIGTERM_action = {0},
                   ignore_action = {0};
  SIGINT_action.sa_handler = catch_SIGTERM;
  sigfillset(&SIGTERM_action.sa_mask);
  SIGTERM_action.sa_flags = 0;

  ignore_action.sa_handler = SIG_IGN;

  /*sigaction(SIGTERM, &SIGTERM_action, NULL);*/
  /*sigaction(SIGINT, &ignore_action, NULL);*/
  /*sigaction(SIGHUP, &ignore_action, NULL);*/

  pid_t spawnPid = -5;
  int childExitStatus = -5;

  while (1) {

    // read user input
    write(1, ": ", 2);
    char buffer[512];
    memset(buffer, '\0', 512);
    int n = read(0, buffer, sizeof(buffer));
    char *path_ptr = strchr(buffer, '\n');
    if (path_ptr != NULL) {
      *path_ptr = '\0';
    }
    printf("You entered %s\n", buffer);

    // if user uses cd
    if (strstr(buffer, "cd") != NULL) {
      // move buffer point to parse path name
      char *path_name = buffer + strlen("cd") + 1;
      printf("%s\n", path_name);
      cd_command(path_name);
    }

    // if user uses status
    if (strcmp(buffer, "status") == 0) {
      status_command();
      break;
    }

    // if user uses exit
    if (strcmp(buffer, "exit") == 0) {
      exit_command();
      break;
    }

    // fork a child process for exec
    spawnPid = fork();

    if (spawnPid == -1) {
      // fork failed

      perror("Error forking\n");
      exit(1);
    } else if (spawnPid == 0) {
      // fork success, in child

      sleep(1);
      printf("Child(%d): converting into \'ls -a\'\n", getpid());
      execlp("ls", "ls", "-la", NULL);
      perror("Child: exec failure!\n");
      exit(2);
    } else {
      // in parent

      printf("Parent(%d): sleeping for 2 seconds\n", getpid());
      sleep(2);
      printf("Parent(%d): waiting for child(%d) to terminate\n", getpid(),
             spawnPid);
      pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
      printf("Parent(%d): Child(%d) terminated, Exiting!\n", getpid(),
             actualPid);
    }
  }
  exit(0);
}

