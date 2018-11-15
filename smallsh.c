/**
 * Filename:    smallsh.c
 * Filetype:    source
 * Author:      Wu, Ziwei
 * Class:       CS344
 * Program:     3
 * Description: Defines smallsh function which starts up the shell,
 *              and execute shell functionalities, including
 *              three builtin commands cd, status, and exit.
 *              It also executes a shell command by using
 *              exec. Another feature is that it performs
 *              expansion of string `$$` into the shell
 *              process ID
 */

#include "smallsh.h"

// Constants
static const int MAX_INPUT_BUFFER_LENGTH = 2048;
static const int MAX_NUM_ARGUMENTS = 512;
static const int MAX_BACKGROUND_CHILDREN_ARRAY_SIZE = 1000;

// Global flag for foreground only mode
static int foreground_only_mode_on = 0;

/**
 * Function: smallsh
 * Description: start the shell program, ask for user input, and execute
 *              either builtin commands cd, status, and exit, or
 *              an external command by using fork and exec
 * Return: None
 */
void smallsh() {

  const pid_t shell_pid = getpid();  // Get pid of shell
  pid_t child_process_pid = -5;
  int exit_status = 0;  // Stores the exit status of children processes

  // Set up an array to store the PID of background children processes
  pid_t background_children_array[MAX_BACKGROUND_CHILDREN_ARRAY_SIZE];
  int i = 0;
  int background_children_count = 0;
  int current_background_children_array_size = 0;
  for (i = 0; i < MAX_BACKGROUND_CHILDREN_ARRAY_SIZE; i++) {
    background_children_array[i] = 0;
  }

  // Initialize the signals handling actions
  struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0},
                   ignore_action = {0};

  // Setup up SIGINT handler
  SIGINT_action.sa_handler = SIG_DFL;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = SA_RESTART;

  // Setup up SIGTSTP handler
  SIGTSTP_action.sa_handler = catch_SIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = 0;

  // Setup up ignore action handler
  ignore_action.sa_handler = SIG_IGN;

  // Setup the parent process to ignore SIGINT
  sigaction(SIGINT, &ignore_action, NULL);

  // Setup the parent process to use handler for SIGTSTP
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  // main shell loop
  while (1) {

    // Initialize flags
    int is_background_process = 0;
    int redirect_input = 0;
    int redirect_output = 0;

    // Initialize input file and output file pointers
    char *input_file = NULL;
    char *output_file = NULL;

    // Check if there are background children that has terminated
    check_background_children_processes(background_children_array,
                                        current_background_children_array_size,
                                        &background_children_count);

    // Initialize the user input buffer
    char buffer[MAX_INPUT_BUFFER_LENGTH];
    char *buffer_ptr = buffer;
    int num_characters_entered = -5;
    size_t buffer_len = MAX_INPUT_BUFFER_LENGTH;
    memset(buffer, '\0', buffer_len);

    // User input loop
    while (1) {

      // print the prompt
      printf(": ");
      fflush(stdout);

      // Get user input
      num_characters_entered = getline(&buffer_ptr, &buffer_len, stdin);

      // If user input failed, ask for input again
      if (num_characters_entered == -1) {
        clearerr(stdin);
      } else {
        break;
      }
    }
    buffer[strlen(buffer) - 1] = '\0';  // Remove \n from input

    // Detect if empty line or blank line are entered by the user
    if (strlen(buffer) == 0 || is_blank_line(buffer, num_characters_entered)) {
      continue;
    }

    // Detect if comment line is entered by the user
    if (buffer[0] == '#') {
      continue;
    }

    // Initialize source and target file descriptors
    int source_FD = -5;
    int target_FD = -5;

    // Intialize variables for parsing the user input
    char *word = NULL;
    int command_count = 0;

    // Initialize command array to store input command and its arguments
    char *commands_array[MAX_NUM_ARGUMENTS];
    for (i = 0; i < MAX_NUM_ARGUMENTS; i++) {
      commands_array[i] = NULL;
    }

    // Loop to parse command and arguments
    word = strtok(buffer, " ");
    while (word != NULL) {

      // When receive "<", parse the input filename and set redirect input flag
      if (strcmp(word, "<") == 0) {
        word = strtok(NULL, " ");
        input_file = word;
        redirect_input = 1;
      }  // When receive ">", parse the output filename and set redirect output
         // flag
      else if (strcmp(word, ">") == 0) {
        word = strtok(NULL, " ");
        output_file = word;
        redirect_output = 1;
      } else {

        // Expand any "$$" substring in command to shell PID
        char shell_pid_buffer[sizeof(shell_pid) * 4 + 1];
        memset(shell_pid_buffer, '\0', sizeof(shell_pid) * 4 + 1);
        sprintf(shell_pid_buffer, "%d", shell_pid);
        word = replace_substring("$$", shell_pid_buffer, word);

        // Add the parsed command to the commands array
        commands_array[command_count] = word;
        command_count++;
      }
      // Kepp parsing the next word
      word = strtok(NULL, " ");
    }

    // Check there is "&" is at the end of command
    if (strcmp(commands_array[command_count - 1], "&") == 0) {

      // If foreground only mode is off, set background process flag on for
      // child process
      if (!foreground_only_mode_on) {
        is_background_process = 1;
      }

      // Remove & symbol from command
      commands_array[command_count - 1] = NULL;
    }

    // If user uses a builtin command
    // When user uses "cd"
    if (strcmp(commands_array[0], "cd") == 0) {
      cd_command(commands_array[1]);
      continue;
    }

    // When user uses "status"
    if (strcmp(commands_array[0], "status") == 0) {
      status_command(&exit_status);
      continue;
    }

    // When user uses "exit"
    if (strcmp(commands_array[0], "exit") == 0) {
      exit_command(background_children_array,
                   current_background_children_array_size);
    }

    // If user uses a none builtin command
    // Fork a child process to execute the none builtin command
    child_process_pid = fork();

    // If the child process failed to fork, repot an error
    if (child_process_pid == -1) {
      perror("error forking a child process");
    }
    // Child process forked succussfully
    else if (child_process_pid == 0) {

      // Note: This scope of code is inside the child process
      // It is important to exit child process whenever an error occurs
      // Or else the child process may take over and become the main loop

      // Setup signal handler for child process
      // Ignore SIGTSTP (CTRL+Z)
      sigaction(SIGTSTP, &ignore_action, NULL);
      // Catch SIGINT (CTRL+C) if child is foreground
      if (!is_background_process) {
        sigaction(SIGINT, &SIGINT_action, NULL);
      }

      // When input needs redirecting
      if (redirect_input) {
        // Open the source file descriptor in read-only mode
        source_FD = open(input_file, O_RDONLY);

        // If file descriptor failed to open, print an error
        if (source_FD == -1) {
          printf("cannot open %s for input\n", input_file);
          fflush(stdout);
          exit(1);
        }

        // If redirect input to stdin failed, output an error
        if (dup2(source_FD, STDIN_FILENO) == -1) {
          perror("unable to redirect source file");
          exit(1);
        }
      }

      // When output needs redirecting
      if (redirect_output) {
        // Open the target file descriptor (create it if it doesn't exit)
        target_FD = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (target_FD == -1) {
          printf("cannot open %s for output\n", input_file);
          fflush(stdout);
        }
        if (dup2(target_FD, STDOUT_FILENO) == -1) {
          perror("unable to redirect target file");
          exit(1);
        }
      }

      // Set dev/null as input for background child without an input
      if (is_background_process && !redirect_input) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          printf("cannot open /dev/null for input\n");
          fflush(stdout);
          exit(1);
        }

        if (dup2(dev_NULL, STDIN_FILENO) == -1) {
          perror("unable to redirect input to dev/null");
          exit(1);
        }
      }

      // Set dev/null as input for background child without an output
      if (is_background_process && !redirect_output) {
        int dev_NULL = open("/dev/null", O_WRONLY);

        if (dev_NULL == -1) {
          printf("cannot open /dev/null for output\n");
          fflush(stdout);
          exit(1);
        }

        if (dup2(dev_NULL, STDOUT_FILENO) == -1) {
          perror("unable to redirect output to dev/null");
          exit(1);
        }
      }

      // Execute the command
      int exec_res = execvp(commands_array[0], commands_array);

      // Note: this section of code only execute if exec fails
      // Print an error and exit if exec fail
      if (exec_res == -1) {
        printf("%s: no such file or directory\n", commands_array[0]);
        fflush(stdout);
        exit(1);
      }
    }  // In parent
    else {
      // Note: this scope of code is in the parent process

      // If the child process forked is a background process, add it to the
      // background children array to track its PID
      if (is_background_process) {
        background_children_array[current_background_children_array_size] =
            child_process_pid;
        current_background_children_array_size++;
        background_children_count++;

        // Print out the PID of background child to the user
        printf("background pid is %d\n", child_process_pid);
        fflush(stdout);
      }  // If the child process forked is a foreground process
      else {

        // Parent waits for the foreground child to exit or terminate
        pid_t child_pid = waitpid(child_process_pid, &exit_status, 0);

        // Parent checks to see if it was terminated by a signal
        // and print out the signal number to the user
        if (WIFSIGNALED(exit_status)) {
          printf("terminated by signal %d\n", WTERMSIG(exit_status));
          fflush(stdout);
        }
      }
    }
  }
}

/**
 * Function: is_blank_line
 * Description: Check if a string is a blank line or empty line
 * Param1: A buffer pointer
 * Param2: Length of the buffer
 * Return: 1 if buffer is a blank/empty line, 0 if buffer is not a blank/empty
 * line
 */
int is_blank_line(char *buffer, int len) {
  int i;
  for (i = 0; i < len - 1; i++) {
    if (!isspace(buffer[i])) {
      return 0;
    }
  }
  return 1;
}

/**
 * Function: check_background_children_processes
 * Description: Check for the termination or exit status of an array of
 *              background processes,if any process terminates or exits,
 *              print out the exit statu or termination signal
 * Param1: An array of process PID for background children processes
 * Param2: Current size of the child processes PID array
 * Param3: An int pointer storing the total number of background children
 * processes
 * Return: None
 */
void check_background_children_processes(
    pid_t *background_children_array,
    int current_background_children_array_size,
    int *background_children_count) {

  int i = 0;
  int pid = -5;
  int status = -5;

  // If there is any current background children
  if (background_children_count > 0) {

    // for each child process in the array, check if it's terminated
    for (i = 0; i < current_background_children_array_size; i++) {

      // PID is 0 for array location without an background child
      if (background_children_array[i] == 0) {
        continue;
      } else {

        // Note: WNOHANG argument allows checking for any exited or terminated
        // child processes
        pid = waitpid(background_children_array[i], &status, WNOHANG);

        // If child has terminated (indicated by pid > 0), get their
        // exit/termination status
        if (pid > 0) {

          // Child was exited
          if (WIFEXITED(status)) {
            status = WEXITSTATUS(status);
            printf("background pid %d is done: exit value %d\n", pid, status);
            fflush(stdout);
          }

          // Child was terminated
          if (WIFSIGNALED(status)) {
            status = WTERMSIG(status);
            printf("background pid %d is done: terminated by signal %d\n", pid,
                   status);
            fflush(stdout);
          }

          // Set pid to 0 indicate such child PID has exited/terminated
          background_children_array[i] = 0;
          (*background_children_count)--;
        }
      }
    }
  }
}

/**
 * Function: catch_SIGSTP
 * Description: Catch SIGSTP sigal and switch foreground only mode on and off
 * Param1: An integer signo
 * Return: None
 */
void catch_SIGTSTP(int signo) {
  char *message1 = "\nEntering foreground-only mode (& is now ignored)\n";
  char *message2 = "\nExiting foreground-only mode\n";

  if (!foreground_only_mode_on) {
    printf("%s", message1);
    fflush(stdout);
  } else {
    printf("%s", message2);
    fflush(stdout);
  }
  foreground_only_mode_on = !foreground_only_mode_on;
}

/**
 * Function: replace_substring
 * Description: Replace a substring in a string given a search term and a place
 * term
 * Param1: A char pointer to the target substring to replace
 * Param2: A char pointer to the term which to replace the substring
 * Param3: A char pointer to the target string to perform the substring
 * replacement
 * Return: None
 * Reference: https://www.binarytides.com/replace_substring-for-c/
 */
char *replace_substring(char *search_term, char *replace_term, char *string) {

  char *pointer = NULL;
  char *old_string = NULL;
  char *new_string = NULL;

  int count = 0;
  int search_term_size = 0;
  int new_string_size = 0;

  search_term_size = strlen(search_term);

  // Count occurance of search term
  pointer = strstr(string, search_term);
  for (pointer = strstr(string, search_term); pointer != NULL;
       pointer = strstr(pointer + search_term_size, search_term)) {
    count++;
  }

  // Compute the size of new string, allocate the space for it
  new_string_size =
      (strlen(replace_term) - search_term_size) * count + strlen(string);
  new_string = malloc(new_string_size);

  strcpy(new_string, "");
  old_string = string;

  // Replacing the substring with replace string
  for (pointer = strstr(string, search_term); pointer != NULL;
       pointer = strstr(pointer + search_term_size, search_term)) {
    strncpy(new_string + strlen(new_string), old_string, pointer - old_string);
    strcpy(new_string + strlen(new_string), replace_term);
    old_string = pointer + search_term_size;
  }

  strcpy(new_string + strlen(new_string), old_string);

  return new_string;
}
