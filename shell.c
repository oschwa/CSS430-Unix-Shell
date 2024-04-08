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

    //  bools for signaling a redirect of I/O
    bool redirect_in = false;
    bool redirect_out = false;
    //  counter for args.
    int i = 0;

    //  iterate through each token.
    while( arg != NULL ) {
      //  if a redirect is detected, then manage 
      //  accordingly. 
      if (equal(arg, ">")) {
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

      //  if a redirect (command to file)
      //  is detected, then manage it.
      if (redirect_in) {
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
      if (end && arg != NULL) {
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