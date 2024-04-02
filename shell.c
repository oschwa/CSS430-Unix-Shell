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
    }
    if (equal(line, "")) {
      continue;
    }
    processLine(line);
  }
  free(line);
  return 0;
}

//  create a new parent and child process for executing the
//  shell command.
void createNewProcess(char ** args, bool concurrent) {
    //  fork a new process.
    int pid = fork();

    if (pid < 0) {
        perror("ERROR: Could not fork - please try again\n");
        return;
    } else if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("ERROR: Invalid command - check command syntax\n");
            return;
        }
    } else {
        //  if the concurrency flag is false, then
        //  wait for the child to terminate.
        if (!concurrent) wait();
    }
}

//  processes command and initiates jobs.
void processLine(char *line) { 

    //  length of line
    int len = strlen(line) + 1;

    //  make a copy of the line to preserve the original 
    //  input.
    char * lineCopy = (char *)malloc(len);
    strcpy(lineCopy, line);

    if (strcmp(line, "!!") == 0) {
      free(lineCopy);
      lineCopy = (char *)malloc(strlen(prevCommand) + 1);
      strcpy(lineCopy, prevCommand);
    }

    //  boolean for keeping track of concurrency.
    bool concurrent = false;

    //  array for all command arguments (including command).
    char *args[len];

    //  stores current string.
    char * curr_s;

    //  counter for args.
    int i = 0;

    //  delimiter for string tokens.
    curr_s = strtok(lineCopy, " ");

    while (curr_s != NULL) {
        if (strcmp(curr_s, "&") != 0) {
          args[i++] = curr_s;
        }
        //  if the concurrency operator '&' is found,
        //  then set flag.
        else {
          concurrent = true;
          break;
        }
        curr_s = strtok(NULL, " ");
    }

    args[i] = NULL;

    createNewProcess(args, concurrent);


    free(lineCopy);

    //  copy successful shell command string.
    free(prevCommand);
    prevCommand = malloc(len);
    strcpy(prevCommand, line);
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