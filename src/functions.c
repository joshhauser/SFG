#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "functions.h"



char ** splitStr(char * string, const char * delimiter, int * i) {
  char ** buffer = malloc(0);

  // Buffer's index
  *i = 0;
  // Index for delimiter search
  int j;
  // Delimiter's index
  int delimPos;
  // Main string's index
  int strPos = 0;
  // Delimiter size
  int delimiterSize = strlen(delimiter);

  // While string is not empty
  while (strlen(string) != 0 && string[0] != '\0') {
    // Token
    char * token = malloc(0);
   
    // Increase strPos until first char of delimiter is encountered
    while (string[strPos] != delimiter[0]) strPos++;

    j = strPos;
    delimPos = 0;

    // Search each character of the delimiter
    while (string[j+1] == delimiter[delimPos+1]) {
      j++;
      delimPos++;
    }

    if (delimPos == (delimiterSize - 1)) {
      int k = 0;
       (*i)++;

      // Dynamic realloc
      buffer = (char**) realloc(buffer, *i * sizeof(char*));
      while (k < strPos) {
        token = (char*) realloc(token,(k+1) * sizeof(char));
        token[k] = string[k];
        printf("tk: %c\n", token[k]);
        k++;
      }
      printf("%s\n", token);
     

      k = 0;
      j++;
      
      while (j < strlen(string) && string[j] != '\0') {
        string[k] = string[j];        
        j++;
        k++;
      }
      string[k] = '\0';
      buffer[(*i)-1] = token;
      printf("valeur de i: %d\n", *i);
      printf("lecture du buffer dans la case %d: %s\n", *i, buffer[(*i)-1]);
      strPos = 0;
      
    }
    else {
      strPos++;
    }
  }

  return buffer;
}

/*
int splitStr(char * string, const char * delimiter, char *** buffer) {
  *buffer = malloc(0);

  // Buffer's index
  int i = 0;
  // Index for delimiter search
  int j;
  // Delimiter's index
  int delimPos;
  // Main string's index
  int strPos = 0;
  // Delimiter size
  int delimiterSize = strlen(delimiter);

  // While string is not empty
  while (strlen(string) != 0 && string[0] != '\0') {
    printf("TEST\n");
    // Token
    char * token = malloc(0);
   
    // Increase strPos until first char of delimiter is encountered
    while (string[strPos] != delimiter[0] && strPos < strlen(string)) strPos++;

    if (string[strPos] != '\0') {

      j = strPos;
      delimPos = 0;

      // Search each character of the delimiter if it's longer than 1
      if (delimiterSize > 1) {
        while (string[j+1] == delimiter[delimPos+1]) {
          j++;
          delimPos++;
        }
      }

      if (delimPos == (delimiterSize - 1)) {
        int k = 0;
        i++;

        // Dynamic realloc
        *buffer = (char**) realloc(*buffer, i * sizeof(char*));
        while (k < strPos) {
          token = (char*) realloc(token,(k+1) * sizeof(char));
          token[k] = string[k];
          k++;
        }
      
        k = 0;
        j++;

        while (j < strlen(string) && string[j] != '\0') {
          string[k] = string[j];
          j++;
          k++;
        } 

        string[k] = '\0';
        *buffer[i-1] = token;
        printf("valeur de i: %d\n", i);
        printf("lecture du buffer dans la case %d: %s\n", i-1, *buffer[i-1]);
        strPos = 0;
        
      }
      else {
        strPos++;
      }
    }
    else {
      printf("%d\n", i);
      i++;
      *buffer = (char**) realloc(*buffer, i * sizeof(char*));
      strcpy(*buffer[i-1], string);
      string[0] = '\0';
    }

  }

  return i;
}*/