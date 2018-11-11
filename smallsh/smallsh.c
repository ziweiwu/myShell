//
// Created by ZIWEIWU on 2018-11-08.
//
#include "smallsh.h"

// global flag for foreground mode
int foreground_only_mode_on = 0;

void smallsh() {
  /*printf("Smallsh running\n");*/

  pid_t shell_pid = getpid();
  /*printf("Smallsh pid is %d\n", shell_pid);*/

  // set up an array to save child process PID
  pid_t background_children_array[1000];
  int i = 0;
  int background_children_count = 0;
  int background_children_array_size = 0;

  // set up signals
  struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0},
                   ignore_action = {0};

  // SIGINT
  SIGINT_action.sa_handler = SIG_DFL;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = SA_RESTART;  // prevent interrupt system call

  // SIGTSTP
  SIGTSTP_action.sa_handler = catch_SIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = SA_RESTART;

  ignore_action.sa_handler = SIG_IGN;

  sigaction(SIGINT, &ignore_action, NULL);

  pid_t spawnPid = -5;
  int childExitStatus = 0;

  while (1) {

    // flags
    int is_background_process = 0;
    int redirect_input = 0;
    int redirect_output = 0;

    // input file and output file
    char *input_file = 0;
    char *output_file = 0;

    // check if there are background children that has terminated
    check_background_processes(background_children_array,
                               background_children_array_size,
                               &background_children_count);

    // set up  main shell to ignore SIGINT
    // save current standard input and standard out
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    printf(": ");
    // need to flush out unused output buffer before input
    fflush(stdout);

    char buffer[MAX_COMMAND_LENGTH];
    char *buffer_ptr = buffer;
    size_t buffer_len = MAX_COMMAND_LENGTH;
    memset(buffer, '\0', buffer_len);
    int n = read(STDOUT_FILENO, buffer, sizeof(buffer));

    /*int n = getline(&buffer_ptr, &buffer_len, stdin);*/
    buffer[strlen(buffer) - 1] = '\0';

    /*printf("You entered %d characters\n", n);*/
    /*printf("You entered %s\n", buffer);*/
    /*fflush(stdout);*/

    // detect empty line or blank line
    if (strlen(buffer) == 0 || is_blank_line(buffer, n)) {
      continue;
    }

    // detect comment line
    if (buffer[0] == '#') {
      continue;
    }

    // if user uses cd
    if (strstr(buffer, "cd") != NULL) {
      // move buffer point to parse path name
      char *path_name = buffer + strlen("cd") + 1;
      printf("%s\n", path_name);
      cd_command(path_name);
    }

    // if user uses status
    else if (strcmp(buffer, "status") == 0) {
      printf("exit status: %d\n", get_status(childExitStatus));
      fflush(stdout);
    }

    // if user uses exit
    else if (strcmp(buffer, "exit") == 0) {
      exit_command(background_children_array, background_children_count);
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
          redirect_output = 1;
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

      // fork a child process for exec the command
      spawnPid = fork();

      // fork failed
      if (spawnPid == -1) {

        perror("Error forking\n");
        exit(1);
      }

      // in child process
      else if (spawnPid == 0) {

        // set all children to ignore SIGTSTP
        sigaction(SIGTSTP, &ignore_action, NULL);
        
        // set foreground process to catch SIGINT
        if (!is_background_process || foreground_only_mode_on) {
          sigaction(SIGINT, &SIGINT_action, NULL);
        }

        // setup redirect input
        if (redirect_input) {
          printf("openning %s\n", input_file);
          fflush(stdout);

          sourceFD = open(input_file, O_RDONLY);
          if (sourceFD == -1) {
            printf("Failed to open source file %s\n", input_file);
            fflush(stdout);
            exit(1);
          }
          if (dup2(sourceFD, STDIN_FILENO) == -1) {
            perror("Unable to redirect source file\n");
            exit(1);
          }
        }

        // setup redirect output
        if (redirect_output) {
          printf("openning %s\n", output_file);
          fflush(stdout);

          targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
          if (targetFD == -1) {
            printf("failed to open target file %s\n", output_file);
            fflush(stdout);
            exit(1);
          }
          if (dup2(targetFD, STDOUT_FILENO) == -1) {
            perror("Unable to redirect target file\n");
            exit(1);
          }
        }

        // if background child process does not redirect input
        if (!foreground_only_mode_on&&is_background_process && !redirect_input) {
          int dev_NULL = open("/dev/null", O_WRONLY);

          if (dev_NULL == -1) {
            perror("failed to open dev/null as source\n");
            exit(1);
          }

          if (dup2(dev_NULL, STDIN_FILENO) == -1) {
            perror("Unable to redirect input to dev/null\n");
            exit(1);
          }
        }

        // if background child process does not redirect output
        if (!foreground_only_mode_on&&is_background_process && !redirect_output) {
          int dev_NULL = open("/dev/null", O_WRONLY);

          if (dev_NULL == -1) {
            perror("failed to open dev/null as target\n");
            exit(1);
          }

          if (dup2(dev_NULL, STDOUT_FILENO) == -1) {
            perror("Unable to redirect output to dev/null\n");
            exit(1);
          }
        }

        // execute none builtin commands
        execvp(commands[0], commands);
        printf("%s failed to execute\n", commands[0]);
        fflush(stdout);
        exit(1);
      }

      // in parent
      else {

        // if child process is background
        if (is_background_process && !foreground_only_mode_on) {
          background_children_array[background_children_array_size++] =
              spawnPid;
          background_children_count++;
          printf("background pid is %d\n", spawnPid);
          fflush(stdout);
        }

        // child process is foreground
        else {

          pid_t child_pid = waitpid(spawnPid, &childExitStatus, 0);

          // print exit status or termination status
          /*if (WIFEXITED(childExitStatus)) {*/
            /*printf("exit value %d\n", WEXITSTATUS(childExitStatus));*/
          /*}*/
          if (WIFSIGNALED(childExitStatus)) {
            printf("terminated by signal %d\n", WTERMSIG(childExitStatus));
          }

          fflush(stdout);
        }
      }
    }
  }
}

/*check if a string is a blank line*/
int is_blank_line(char *buffer, int len) {
  int i;
  for (i = 0; i < len - 1; i++) {
    if (!isspace(buffer[i])) {
      return 0;
    }
  }
  return 1;
}

/*check if any background children has terminated */
void check_background_processes(pid_t *background_children_array,
                                int background_children_array_size,
                                int *background_children_count) {
  int i, pid;
  int status = -5;
  if (background_children_count > 0) {
    for (i = 0; i < background_children_array_size; i++) {
      // pid is process pid if child has terminated, 0 if nothing changes,
      pid = waitpid(background_children_array[i], &status, WNOHANG);
      // if child has terminated
      if (pid > 0) {
        if (WIFEXITED(status)) {
          status = WEXITSTATUS(status);
          printf("background pid %d is done: exit value %d\n", pid, status);
        }
        if (WIFSIGNALED(status)) {
          status = WTERMSIG(status);
          printf("background pid %d is terminated by signal: %d\n", pid,
                 status);
        }

        (*background_children_count)--;
        printf("Currently there are  %d background children \n",
               *background_children_count);
      }
    }
  }
}


// signal catcher 

void catch_SIGTSTP(int signo) {
  char* message1 = "Entering foreground-only mode (& is now ignored)\n";
  char* message2 = "Exiting foreground-only mode (& is now in effect)\n";
  if(!foreground_only_mode_on){
    write(STDOUT_FILENO, message1, 49);
  }else{
    write(STDOUT_FILENO, message2, 50);
  }
  foreground_only_mode_on = !foreground_only_mode_on;
  fflush(stdout);
}
