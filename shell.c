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
      processLine(prevCommand);
    }
    if (equal(line, "")) {
      continue;
    }
    processLine(line);
  }
  free(line);
  free(prevCommand);
  return 0;
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

    //  split string using ' ' as a delimiter.
    char * arg = strtok(line, " ");

    //  use an array of string to hold arguments.
    char * args[ARG_LIMIT + 1];

    //  bool for concurrency flag.
    bool concurrent = false;

    //  bool for signaling end of command.
    bool end = false;

    //  counter for args.
    int i = 0;

    //  iterate through each token.
    while( arg != NULL ) {
      //  if a concurrency operator is detected,
      //  set the flag to true.
      if (equal(arg, "&")) {
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

    args[i] = NULL; 
    //  final command is executed.
    execute(args, concurrent);
    //  free the token string.
    free(arg);
    //  empty argument array.
    memset(args, 0, sizeof args);
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