#include "../headers/shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void initShell() {
  system("clear");
  char** args;

  while (1) {
    prompt();
    readCommand(&args);
  }
}

void prompt() {
  system("echo [ShellLite]:~$");
}

int readCommand(char*** args) {
  char* p = NULL;
  char* string = NULL;
  *args = (char**) malloc(sizeof(char*));

  if (fgets(string, MAX_READABLE, stdin) != NULL) {
    p = strchr(string, '\n');

    if (p) *p = '\0';
    else cleanBuffer();

    p = strchr(string, ' ');

    if (!p) {
      *args[0] = string;
    }
    else {
      int argsLength = 1;
      char splitToken[] = " ";
      char *ptr = strtok(string, splitToken);

      while (ptr) {
        *args[argsLength - 1] = ptr;
        argsLength++;
        *args = (char**) realloc(args, argsLength * sizeof(char*));
      }
    }
    
    return 1;
  }
  else {
    cleanBuffer();
    return 0;
  }
}

void cleanBuffer() {
  int c = 0;
  
  while (c != '\n' && c != EOF) c = getchar();
}