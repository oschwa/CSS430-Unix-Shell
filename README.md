# CSS430-Unix-Shell

An interactive Unix Shell application that is able to run any Unix Command.

This project makes use of various system calls to enable functionality:
- 'fork()' for new processes.
- 'exec()' for running commands.
- 'dup2()' for altering the standard input and output for redirection to files.
- 'pipe()' for allowing parent and child processes to communicate and provide input for commands.

NOTE: This project requires a Unix-based environment, such as Linux or macOS. To run this project 
Windows, use a container such as [Docker](https://docs.docker.com/guides/walkthroughs/run-a-container/)

To compile this project, enter the following into your command-line interface of choice:
GNU: 'gcc -o shell shell.c shell.h'
Clang: 'clang -o shell shell.c shell.h'

This project has two options for execution:

1. './shell --interactive' - runs shell as an interactive Unix command line interface.
2. './shell' - runs the executable with a test script and outputs resulting commands.
