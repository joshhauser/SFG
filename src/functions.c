#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../headers/functions.h"


/* Search delimiter as a substring of "string" to cut main string
into tokens, and use bufferSize to count tokens */
char ** splitStr(char * string, const char * delimiter, int * bufferSize) {
  char ** buffer = malloc(sizeof(char));

  // Buffer's index
  *bufferSize = 1;
  // Index for delimiter search
  int j;
  // Delimiter's index
  int delimPos;
  // Main string's index
  int strPos = 0;
  // Delimiter size
  int delimiterSize = strlen(delimiter);

  // While string is not empty
  while (strlen(string) != 0 && string[0] != '\0' && strPos < strlen(string)) {
    // Token
    char * token = malloc(sizeof(char));

    // Increase strPos until first char of delimiter is encountered
    while (string[strPos] != delimiter[0] && strPos < strlen(string)) strPos++;
    j = strPos;
    delimPos = 0;

    // Search each character of the delimiter
    while (string[j+1] == delimiter[delimPos+1]) {
      j++;
      delimPos++;
    }

    // If all delimiter's chars have been found
    if (delimPos == (delimiterSize - 1)) {
      int k = 0;
      printf("founded\n");
      (*bufferSize)++;

      // Dynamic realloc
      buffer = (char**) realloc(buffer, (*bufferSize) * sizeof(char*));

      // Copy each token's char into token's array
      while (k < strPos) {
        token = (char*) realloc(token, (k+1) * sizeof(char));
        token[k] = string[k];
        k++;
      }
      
  
      k = 0;
      j++;
      
      // Shift other string's chars to the left
      while (j < strlen(string) && string[j] != '\0') {
        string[k] = string[j];        
        j++;
        k++;
      }
    
      // Delete useless chars at the end of the string
      string[k] = '\0';

      // Add the token to the buffer's array
      buffer[(*bufferSize)-2] = token;

      //printf("valeur de i: %d\n", (*bufferSize));
      //printf("lecture du buffer dans la case %d: %s\n", (*bufferSize), buffer[(*bufferSize)-1]);

      // Reset position in main string
      strPos = 0;
    }
    else  {
      strPos++;
    }
   
  }


    buffer[(*bufferSize)-1] = (char*) malloc((strlen(string) + 1) * sizeof(char));
    buffer[(*bufferSize)-1] = string;
  
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