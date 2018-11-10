//
// Created by ZIWEIWU on 2018-11-08.
//
#include "smallsh.h"

void smallsh() {
  printf("Smallsh running\n");

  pid_t shell_pid = getpid();
  printf("Smallsh pid is %d\n", shell_pid);

  // set up an array to save child process PID
  pid_t child_pid_array[1000];

  int i, child_count = 0;

  // set up signals
  struct sigaction SIGINT_action = {0}, SIGTERM_action = {0},
                   SIGCHLD_action = {0}, ignore_action = {0};
  SIGINT_action.sa_handler = catch_SIGTERM;
  SIGCHLD_action.sa_handler = catch_SIGCHLD;
  sigfillset(&SIGTERM_action.sa_mask);
  sigfillset(&SIGINT_action.sa_mask);
  SIGTERM_action.sa_flags = 0;
  SIGCHLD_action.sa_flags = 0;

  ignore_action.sa_handler = SIG_IGN;

  /*sigaction(SIGTERM, &SIGTERM_action, NULL);*/
  /*sigaction(SIGINT, &ignore_action, NULL);*/
  /*sigaction(SIGHUP, &ignore_action, NULL);*/

  pid_t spawnPid = -5;
  int childExitStatus = -5;

  while (1) {
    // print past child pid
    /*for (i = 0; i < child_count; i++) {*/
    /*printf("Past child pid: is %d\n", child_pid_array[i]);*/
    /*fflush(stdout);*/
    /*}*/

    // flags
    int is_background_process = 0;
    int  redirect_input = 0;
    int  redirect_output = 0;

    // input file and output file
    char *input_file = 0;
    char *output_file = 0;

    // save current standard input and standard out
    write(1, ": ", 2);
    // need to flush out unused output buffer before input
    fflush(stdout);

    char buffer[MAX_COMMAND_LENGTH];
    memset(buffer, '\0', MAX_COMMAND_LENGTH);
    int n = read(STDOUT_FILENO, buffer, sizeof(buffer));
    char *path_ptr = strchr(buffer, '\n');
    if (path_ptr != NULL) {
      *path_ptr = '\0';
    }
    printf("You entered %d characters\n", n);
    printf("You entered %s\n", buffer);
    fflush(stdout);

    // if user uses cd
    if (strstr(buffer, "cd") != NULL) {
      // move buffer point to parse path name
      char *path_name = buffer + strlen("cd") + 1;
      printf("%s\n", path_name);
      cd_command(path_name);
    }

    // if user uses status
    if (strcmp(buffer, "status") == 0) {
      status_command(childExitStatus);
      fflush(stdout);
    }

    // if user uses exit
    if (strcmp(buffer, "exit") == 0) {
      exit_command(child_pid_array, child_count);
      break;
    }

    // if user uses a none-builtin commands
    else {
      // convert buffer into an array of words
      int sourceFD, targetFD, result;
      char *p = strtok(buffer, " ");
      char *commands[MAX_NUM_ARGS];
      int count = 0;

      // initialize commands array
      for (i = 0; i < MAX_NUM_ARGS; i++) {
        commands[i] = NULL;
      }

      // parse command and arguments
      while (p != NULL) {
        // receive $$, need expand pid 
        if (strcmp(p, "$$") == 0) {
          char shell_pid_buffer[sizeof(shell_pid) * 4 + 1];
          memset(shell_pid_buffer, '\0', sizeof(shell_pid) * 4 + 1);
          sprintf(shell_pid_buffer, "%d", shell_pid);
          commands[count++] = shell_pid_buffer;
        }

        // receive <
        else if (strcmp(p, "<") == 0) {
          p = strtok(NULL, " ");
          input_file = p;
          redirect_input = 1;
          printf("input target is %s\n", p);
          fflush(stdout);

        }

        // receive >
        else if (strcmp(p, ">") == 0) {
          p = strtok(NULL, " ");
          output_file = p;
          redirect_output=1;
          printf("output target is %s\n", p);
          fflush(stdout);
        }

        // receive &
        else if (strcmp(p, "&") == 0) {
          is_background_process = 1;
        }

        // if current command pointer is not pecial symbols
        else {
          commands[count++] = p;
        }
        // move string pointer forward
        p = strtok(NULL, " ");
      }

      printf("The command you enter is: ");
      printf("%s ", commands[0]);
      printf("%s ", commands[1]);
      printf("\n");
      fflush(stdout);

      // fork a child process for exec the command
      spawnPid = fork();

      // setup redirect input
      if (redirect_input) {
        printf("openning %s\n", input_file);
        sourceFD = open(input_file, O_RDONLY);
        if (sourceFD == -1) {
          perror("failed to open source file");
          exit(1);
        }
        if (dup2(sourceFD, STDIN_FILENO) == -1) {
          perror("Unable to redirect source file");
          exit(1);
        }
      }

      // setup redirect output
      if (redirect_output) {
        printf("openning %s\n", output_file);
        targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) {
          perror("failed to open target file");
          exit(1);
        }
        if (dup2(targetFD, STDOUT_FILENO) == -1) {
          perror("Unable to redirect source file");
          exit(1);
        }
      }

      // if background child process does not redirect input
      if (is_background_process && !redirect_input) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          perror("failed to open dev/null");
          exit(1);
        }

        if (dup2(dev_NULL, STDIN_FILENO) == -1) {
          perror("Unable to redirect input to dev/null");
          exit(1);
        }
      }

      // if background child process does not redirect output
      if (is_background_process && !redirect_output) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          perror("failed to open dev/null");
          exit(1);
        }

        if (dup2(dev_NULL, STDOUT_FILENO) == -1) {
          perror("Unable to redirect output to dev/null");
          exit(1);
        }
      }

      // fork failed
      if (spawnPid == -1) {

        perror("Error forking\n");
        exit(1);
      }

      // in child process
      else if (spawnPid == 0) {
        execvp(commands[0], commands);
        perror("Child: exec failure!\n");
        printf("%s failed", commands[0]);
        fflush(stdout);
        exit(1);
      }

      // in parent process
      else {
        // if child process is background
        if (is_background_process) {
          child_pid_array[child_count++] = spawnPid;
          printf("background pid is %d\n", spawnPid);
          fflush(stdout);
        }

        // child process is foreground
        else {
          sleep(1);
          pid_t actualPid = waitpid(spawnPid, &childExitStatus, 0);
          printf("Parent(%d): Child(%d) terminated, Exiting!\n", getpid(),
                 actualPid);
          fflush(stdout);
        }
      }
    }
  }
}

