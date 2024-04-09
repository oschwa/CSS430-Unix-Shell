/**
 * Author: Oliver Schwab
 * Class: CSS430 Spring 2024
 * Professor: Faisal Ahmed 
 * Program: Unix Shell 
 * Description: A Unix shell capable of executing commands the following capabilities...
 *    Creating child processes using fork().
 *    Running commands on child processes using execvp().
 *    Using a history feature via the command "!!".
 *    Redirecting I/O using "<" and ">".
 *    Using commands via a pipe with "|".
 * Issues with program: "|" works with two commands, however it will print a 
 * "no such file exists" error alongside working input. 
 * Built off of starter code provided by Professor Pisan's material. 
*/
#include "shell.h"

int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  prev_command = NULL;
  char *line = calloc(1, MAXLINE);
  while (should_run) {
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    // ^D results in n == -1
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    } else if (equal(line, "!!")) {
      processLine(prev_command);
    } else if (equal(line, "ascii")) {
      ascii();
    } else if (equal(line, "")) {
      continue;
    } else {
      processLine(line);
    }
  }
  free(line);
  free(prev_command);
  return 0;
}

//  runs a pipe operation.
bool usePipe(char ** args1, char ** args2) {
  //  create a pipe.
  int pipe_fd[2];
  int pid;

  //  use pipe()
  if (pipe(pipe_fd) < 0) {
    perror("Pipe could not open.\n");
    return false;
  }

  //  create a new child process.
  pid = fork(); 

  if (pid < 0) {
    perror("Failed to create new processes.\n");
    return false;
  } else if (pid == 0) {
    //  close STDOUT.
    //  make pipe the same as stdout.
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);
    close(pipe_fd[1]);
    //  execute first command.
    if (execvp(args1[0], args1) == -1) {
      perror("Failed to execute first command.\n");
      exit(1);
    }
  } else {
    //  fork another new process.
    int pid2 = fork();

    if (pid2 < 0) {
      perror("Failed to create new processes.\n");
      return false;
    } else if (pid2 == 0) {
      //  close STDIN.
      //  make pipe the same as stdin.
      close(pipe_fd[1]);
      dup2(pipe_fd[0], STDIN_FILENO);
      close(pipe_fd[0]);
      //  execute second command.
      if (execvp(args2[0], args2) == -1) {
        perror("Failed to execute second command\n");
        exit(1);
      } 
    } else {
      close(pipe_fd[0]);
      close(pipe_fd[1]);

      int stat1, stat2;
      // Wait for the first child process.
      waitpid(pid, &stat1, 0);
      // Wait for the second child process.
      waitpid(pid2, &stat2, 0);
    }
  }
  return true;
}

//  conducts output redirection.
void executeToFile(char ** args, char * fileName) {
  //  fork and create a child process.
  int pid = fork();
  if (pid < 0) {
    perror("Failed to execute input redirection.\n");
  } else if (pid == 0) {
    //  open the file.
    int fd = open(fileName, O_WRONLY | O_APPEND | O_CREAT);
    //  check file integrity.
    if (fd == -1) {
      perror("ERROR: file could not be opened\n");
      close(fd);
      return;
    }
    //  use dup2 to duplicate file to stdin.
    dup2(fd, STDOUT_FILENO);
    //  execute command to be outputted to file.
    if (execvp(args[0], args) == -1) {
      perror("ERROR: command failed to execute - check command syntax\n");
      return;
    }
    //  close file.
    close(fd);
  } else {
    wait( NULL );
  }
}

//  takes contents of text file and uses it as
//  input for command.
void executeFromFile(char ** args, char * fileName) {
  //  fork and create a child process.
  int pid = fork();
  if (pid < 0) {
    perror("Failed to execute input redirection.\n");
  } else if (pid == 0) {
    //  open the file.
    int fd = open(fileName, O_RDONLY);
    //  check file integrity.
    if (fd == -1) {
      perror("ERROR: file could not be opened\n");
      close(fd);
      return;
    }
    //  use dup2 to duplicate file to stdin.
    dup2(fd, STDIN_FILENO);
    //  execute command to be outputted to file.
    if (execvp(args[0], args) == -1) {
      perror("ERROR: command failed to execute - check command syntax\n");
      return;
    }
    //  close file.
    close(fd);
  } else {
    wait( NULL );
  }
}

//  create a new parent and child process for executing the
//  shell command.
void execute(char ** args, bool concurrent) {
    //  fork a new process.
    int pid = fork();

    if (pid < 0) {
        perror("ERROR: Could not fork - please try again\n");
        return;
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("ERROR: command failed to execute - check command syntax\n");
            printf("%s\n", args[0]);
            return;
        }
    } else {
        //  if the concurrency flag is false, then
        //  wait for the child to terminate.
        if (!concurrent) wait(NULL);
    }
}

//  processes command and initiates jobs.
void processLine(char *line) { 
    //  copy line to preserve original input.
    char * copy = malloc(sizeof(line));
    strcpy(copy, line);
    //  split string using ' ' as a delimiter.
    char * arg = strtok(copy, " ");

    //  use an array of string to hold arguments.
    char * args[sizeof(line) + 1];

    //  bool for concurrency flag.
    bool concurrent = false;

    //  bool for signaling end of command.
    bool end = false;

    //  bools for signaling a pipe.
    bool is_pipe = false;
    bool success = false;
    //  bools for signaling a redirect of I/O
    bool redirect_in = false;
    bool redirect_out = false;

    //  counter for args.
    int i = 0;

    //  iterate through each token.
    while( arg != NULL ) {
      //  if a pipe is detected, then prepare
      //  a pipe file descriptor.
      if (equal(arg, "|")) {
        is_pipe = true;
      }
      //  if a redirect is detected, then manage 
      //  accordingly. 
      else if (equal(arg, ">")) {
        redirect_in = true;
      } else if (equal(arg, "<")) {
        redirect_out = true;
      }
      //  if a concurrency operator is detected,
      //  set the flag to true.
      else if (equal(arg, "&")) {
        concurrent = true;
        end = true;
      } 
      //  if the wait operator is detected, 
      //  set the flag to false.
      else if (equal(arg, ";")) {
        concurrent = false;
        end = true;
      } 
      //  otherwise, add the command to the
      //  command args.
      else {
        args[i++] = arg;
      }

      arg = strtok(NULL, " ");

      //  if a pipe operation is detected, then
      //  call usePipe().
      if (is_pipe) {
        //  set up argument array for second command.
        char *args2[2];
        args2[0] = arg;
        args2[1] = NULL;

        // begin pipe operations.
        args[i] = NULL;

        if (!usePipe(args, args2)) {
          break;
        }
        is_pipe = false;
      }
      //  if a redirect (command to file)
      //  is detected, then manage it.
      else if (redirect_in) {
        args[i] = NULL;
        executeToFile(args, arg);
        break;
      } else if (redirect_out) {
        args[i] = NULL;
        executeFromFile(args, arg);
        break;
      }

      //  if the end of a command has been reached, 
      //  but there are more arguments, execute the 
      //  current command and it's arguments. Before
      //  further iterating.
      else if (end && arg != NULL) {
        args[i] = NULL;
        execute(args, concurrent);
        memset(args, 0, sizeof args);
        i = 0;
        end = false;
        concurrent = false;
      }
    }

    if (!redirect_in && !redirect_out) {
      args[i] = NULL; 
      //  final command is executed.
      execute(args, concurrent);
      //  free the token string.
      free(arg);
      //  save input string to previous command global.
      free(prev_command);
      prev_command = malloc(sizeof(line));
      strcpy(prev_command, line);
      //  free the line copy.
      free(copy);
      //  empty argument array.
      memset(args, 0, sizeof args);
    }
}

//  prints ASCII art.
void ascii() {
  printf(" /-------/\n");
  printf(" | O  O |\n");
  printf("/| .  . |/\n");
  printf(" |  ''  |\n");
  printf(" |''''''|\n");
  printf("I tried to make Spongebob\n");
}

int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls",      "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0';
  }
  return n;
}