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
  SIGTSTP_action.sa_flags = 0;

  ignore_action.sa_handler = SIG_IGN;

  sigaction(SIGINT, &ignore_action, NULL);
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

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
                               &background_children_count, &childExitStatus);

    // set up  main shell to ignore SIGINT
    // save current standard input and standard out

    char buffer[MAX_COMMAND_LENGTH];
    char *buffer_ptr = buffer;
    int num_characters_entered = -5;
    size_t buffer_len = MAX_COMMAND_LENGTH;
    memset(buffer, '\0', buffer_len);

    /*int n = read(STDOUT_FILENO, buffer, sizeof(buffer));*/

    // get user input
    while (1) {
      printf(": ");
      // need to flush out unused output buffer before input
      fflush(stdout);
      num_characters_entered = getline(&buffer_ptr, &buffer_len, stdin);
      if (num_characters_entered == -1) {
        clearerr(stdin);
      } else {
        break;
      }
    }
    buffer[strlen(buffer) - 1] = '\0';

    // check if there are background children that has terminated
    check_background_processes(background_children_array,
                               background_children_array_size,
                               &background_children_count, &childExitStatus);

    // detect empty line or blank line
    if (strlen(buffer) == 0 || is_blank_line(buffer, num_characters_entered)) {
      continue;
    }

    // detect comment line
    if (buffer[0] == '#') {
      continue;
    }
    // convert buffer into an array of words
    int sourceFD, targetFD, result;
    char *p = strtok(buffer, " ");
    char *commands[MAX_NUM_ARGS];
    int count = 0;

    // initialize command array
    for (i = 0; i < MAX_NUM_ARGS; i++) {
      commands[i] = NULL;
    }

    // parse command and arguments
    while (p != NULL) {

      // receive $$
      /*if (strcmp(p, "$$") == 0) {*/
      /*char shell_pid_buffer[sizeof(shell_pid) * 4 + 1];*/
      /*memset(shell_pid_buffer, '\0', sizeof(shell_pid) * 4 + 1);*/
      /*sprintf(shell_pid_buffer, "%d", shell_pid);*/
      /*commands[count++] = shell_pid_buffer;*/
      /*}*/

      // receive < for input file
      if (strcmp(p, "<") == 0) {
        p = strtok(NULL, " ");
        input_file = p;
        redirect_input = 1;
        /*printf("input target is %s\n", p);*/
        /*fflush(stdout);*/

      }

      // receive > for output file
      else if (strcmp(p, ">") == 0) {
        p = strtok(NULL, " ");
        output_file = p;
        redirect_output = 1;
        /*printf("output target is %s\n", p);*/
        /*fflush(stdout);*/
      } else {

        // perform $$ expansion to shell pid
        char shell_pid_buffer[sizeof(shell_pid) * 4 + 1];
        memset(shell_pid_buffer, '\0', sizeof(shell_pid) * 4 + 1);
        sprintf(shell_pid_buffer, "%d", shell_pid);
        p = str_replace("$$", shell_pid_buffer, p);

        /*printf("Your command is %s\n", p);*/
        /*fflush(stdout);*/

        // add command to command array
        commands[count++] = p;
      }
      // keep parsing
      p = strtok(NULL, " ");
    }

    // check if & is at the end of command
    // if there is, turn on background flag
    // as long as foreground only mode is off
    if (strcmp(commands[count - 1], "&") == 0) {
      if (!foreground_only_mode_on) {
        is_background_process = 1;
      }
      // remove & symbol from command
      commands[count - 1] = NULL;
    }

    // if user uses cd
    if (strcmp(commands[0], "cd") == 0) {
      cd_command(commands[1], &childExitStatus);
      continue;
    }

    // if user uses status
    if (strcmp(commands[0], "status") == 0) {
      status_command(&childExitStatus);
      continue;
    }

    // if user uses exit
    if (strcmp(commands[0], "exit") == 0) {
      exit_command(background_children_array, background_children_array_size);
      break;
    }

    // fork a child process for exec the command
    spawnPid = fork();

    // fork failed
    if (spawnPid == -1) {
      perror("rrror forking a child process");
      childExitStatus = 1;
    }

    // in child process
    else if (spawnPid == 0) {

      // set all children to ignore SIGTSTP
      sigaction(SIGTSTP, &ignore_action, NULL);

      // set foreground process to catch SIGINT
      if (!is_background_process) {
        sigaction(SIGINT, &SIGINT_action, NULL);
      }

      // setup redirect input
      if (redirect_input) {
        /*printf("openning %s\n", input_file);*/
        /*fflush(stdout);*/

        sourceFD = open(input_file, O_RDONLY);
        if (sourceFD == -1) {
          perror("failed to open source file");
          exit(1);
        }
        if (dup2(sourceFD, STDIN_FILENO) == -1) {
          perror("unable to redirect source file");
          exit(1);
        }
      }

      // setup redirect output
      if (redirect_output) {
        /*printf("openning %s\n", output_file);*/
        /*fflush(stdout);*/

        targetFD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (targetFD == -1) {
          perror("failed to open target file");
          exit(1);
        }
        if (dup2(targetFD, STDOUT_FILENO) == -1) {
          perror("unable to redirect target file");
          exit(1);
        }
      }

      // if background child process does not redirect input
      if (is_background_process && !redirect_input) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          perror("failed to open dev/null as source");
          exit(1);
        }

        if (dup2(dev_NULL, STDIN_FILENO) == -1) {
          perror("unable to redirect input to dev/null");
          exit(1);
        }
      }

      // if background child process does not redirect output
      if (is_background_process && !redirect_output) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          perror("failed to open dev/null as target");
          exit(1);
        }

        if (dup2(dev_NULL, STDOUT_FILENO) == -1) {
          perror("unable to redirect output to dev/null");
          exit(1);
        }
      }

      // execute none builtin command
      int exec_res = execvp(commands[0], commands);
      // if command failed to excute
      if (exec_res == -1) {
        perror("command failed to execute");
        exit(1);
      }
    }

    // in parent
    else {
      // if child process is background, track it by adding its pid to an array
      if (is_background_process) {
        background_children_array[background_children_array_size++] = spawnPid;
        background_children_count++;
        printf("background pid is %d\n", spawnPid);
        fflush(stdout);
      }

      // child process is foreground
      else {

        pid_t child_pid = waitpid(spawnPid, &childExitStatus, 0);

        // check to see if it was terminated by a signal
        // and print out the signal number
        if (WIFSIGNALED(childExitStatus)) {
          printf("terminated by signal %d\n", WTERMSIG(childExitStatus));
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
                                int *background_children_count,
                                int *childExitStatus) {
  int i, pid;
  int status = -5;
  if (background_children_count > 0) {

    // for each child process in the array, check if it's terminated
    for (i = 0; i < background_children_array_size; i++) {
      pid = waitpid(background_children_array[i], &status, WNOHANG);
      // if child has terminated, check their exit status
      if (pid > 0) {
        if (WIFEXITED(status)) {
          status = WEXITSTATUS(status);
          printf("background pid %d is done: exit value %d\n", pid, status);
          fflush(stdout);
        }
        if (WIFSIGNALED(status)) {
          status = WTERMSIG(status);
          printf("background pid %d is terminated by signal: %d\n", pid,
                 status);
          fflush(stdout);
        }
        (*background_children_count)--;
        /*printf("Currently there are  %d background children \n",*/
        /**background_children_count);*/

        // update exit status in shell
        *childExitStatus = status;
      }
    }
  }
}

// signal catcher for SIGTSTP
void catch_SIGTSTP(int signo) {
  char *message1 = "\nEntering foreground-only mode (& is now ignored)\n";
  char *message2 = "\nExiting foreground-only mode (& is now in effect)\n";
  /*if (!foreground_only_mode_on) {*/
  /*write(STDOUT_FILENO, message1, 50);*/
  /*} else {*/
  /*write(STDOUT_FILENO, message2, 51);*/
  /*}*/

  if (!foreground_only_mode_on) {
    printf("%s", message1);
  } else {
    printf("%s", message2);
  }
  fflush(stdout);
  foreground_only_mode_on = !foreground_only_mode_on;
}

// string replace
// reference: https://www.binarytides.com/str_replace-for-c/

char *str_replace(char *search_term, char *replace_term, char *string) {
  char *p = NULL, *old_string = NULL, *new_string = NULL;
  int count = 0, search_term_size, new_string_size;

  search_term_size = strlen(search_term);

  // count occurance of search term
  p = strstr(string, search_term);
  for (p = strstr(string, search_term); p != NULL;
       p = strstr(p + search_term_size, search_term)) {
    count++;
  }

  // compute the size of new string, allocate the space for it
  new_string_size =
      (strlen(replace_term) - search_term_size) * count + strlen(string);
  new_string = malloc(new_string_size);

  strcpy(new_string, "");
  old_string = string;

  // replacing search term with replace term, create the new string
  for (p = strstr(string, search_term); p != NULL;
       p = strstr(p + search_term_size, search_term)) {
    strncpy(new_string + strlen(new_string), old_string, p - old_string);
    strcpy(new_string + strlen(new_string), replace_term);
    old_string = p + search_term_size;
  }

  strcpy(new_string + strlen(new_string), old_string);

  return new_string;
}
