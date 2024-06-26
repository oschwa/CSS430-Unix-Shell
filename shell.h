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
#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp

#define MAXLINE 80
#define PROMPT "osh> "

#define RD 0
#define WR 1
#define ARG_LIMIT 12
#define BUF_SIZE 256

char * prev_command;

bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();
void ascii();
bool usePipe(char ** args1, char ** args2);
void executeToFile(char ** args, char * fileName);
void executeFromFile(char ** args, char * fileName);
void execute(char ** args, bool concurrent);
void processLine(char *line);
int main();

#endif