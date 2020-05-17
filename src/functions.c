#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../headers/functions.h"


char **splitStr(char * str, const char *delimiter, int *bufferSize) {
  char **buffer = (char**) malloc(sizeof(char*));
  char *substr;
  int i;
  int tokenSize;

  char *copy = strDup(str);
  (*bufferSize) = 0;
  while ((substr = strstr(copy, delimiter)) != NULL) {
    (*bufferSize)++;
    tokenSize = strlen(copy) - strlen(substr);
    buffer = (char**) realloc(buffer, (*bufferSize) * sizeof(char*));
    buffer[(*bufferSize)-1] = (char*) malloc(tokenSize+1 * sizeof(char));

    for (i = 0; i < tokenSize; i++) {
      buffer[(*bufferSize)-1][i] = copy[i];
    }
    buffer[(*bufferSize)-1][tokenSize] = '\0';

    copy = substr + strlen(delimiter);
  }

  if (strlen(copy) > 0) {
    (*bufferSize)++;
    buffer = (char**) realloc(buffer, (*bufferSize)*sizeof(char*));
    buffer[(*bufferSize)-1] = (char*) malloc(strlen(copy) + 1 * sizeof(char));
    buffer[(*bufferSize)-1] = copy;
    buffer[(*bufferSize)-1][strlen(copy)] = '\0';
  }

  /* for (i = 0; i < (*bufferSize); i++) {
    printf("TOKEN N°%d: %s\n", i, buffer[i]);
  } */
  return buffer;
}

char * strDup(char * str) {
  char * copy = malloc(strlen(str) + 1 *sizeof(char));
  strcpy(copy, str);

  return copy;
}

int countOcc(char *string, char searchedChar) {
  int length = strlen(string);
  int i;
  int count = 0;

  for (i = 0 ; i < length; i++) {
    if (string[i] == searchedChar) count++;
  }

  return count;
}

