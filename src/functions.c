#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "../headers/functions.h"

/**
* Splits a string by a delimiter and puts each token in a buffer
* @param str the string to split
* @param delimiter the delimiter to search in the string to split it
* @param bufferSize the number of tokens in the buffer
* @return the buffer with the tokens

* Note: if the delimiter is not found, the returned buffer contains the full
* string and buffer size is 1.
**/
char **splitStr(char * str, const char *delimiter, int *bufferSize) {
  char **buffer = (char**) malloc(sizeof(char*));
  char *substr;
  int i;
  int tokenSize;

  // Create a copy of the string so that it's not modified
  char *copy = strDup(str);
  // Init buffer size
  (*bufferSize) = 0;

  while ((substr = strstr(copy, delimiter)) != NULL) {
    (*bufferSize)++;
    // Compute strlen of the token
    tokenSize = strlen(copy) - strlen(substr);
    // Dynamic reallocation of the buffer array
    buffer = (char**) realloc(buffer, (*bufferSize) * sizeof(char*));
    // Dynamic allocation of a new index in the buffer array
    buffer[(*bufferSize)-1] = (char*) malloc(tokenSize+1 * sizeof(char));

    // Token copying in the buffer
    for (i = 0; i < tokenSize; i++) {
      buffer[(*bufferSize)-1][i] = copy[i];
    }

    buffer[(*bufferSize)-1][tokenSize] = '\0';

    // Shift the position in the string copy
    copy = substr + strlen(delimiter);
  }

  /* If there are some chars after the last found delimiter (or if the delimiter)
  was not found, these chars are copied in the buffer */
  if (strlen(copy) > 0) {
    (*bufferSize)++;
    buffer = (char**) realloc(buffer, (*bufferSize)*sizeof(char*));
    buffer[(*bufferSize)-1] = (char*) malloc(strlen(copy) + 1 * sizeof(char));
    buffer[(*bufferSize)-1] = copy;
    buffer[(*bufferSize)-1][strlen(copy)] = '\0';
  }

  return buffer;
}

/**
* Creates a copy of a string
* @param str the string to copy
* @return the copy of str
**/
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
//remove 1 char
void remove_char(char* word,int idxToDel)
{	
	memmove(&word[idxToDel], &word[idxToDel + 1], strlen(word) - idxToDel);
}
//gets rid of the file's index and delimiters
void clean_file_name(char* word)
{
	remove_char(word,strlen(word)-1);
	remove_char(word,2);
	remove_char(word,1);
	remove_char(word,0);
		
}
//removes a substring
/*void remove_string(char* str,char* str_rem)
{
	int i;
	//for(i=strlen(str_rem);i>0;i--)
	for(i=0;i<strlen(str_rem);i++)
	{
		remove_char(str,i);
	}
}*/

void remove_string(char *s, char const *srem) /* Source https://www.developpez.net/forums/d133547/c-cpp/c/enlever-d-chaine/ */
//Retire une sous chaine d'une chaine 
{
  int n = 0;
  char *p;
 
  if (s && srem)
  {
    size_t const len = strlen(srem);
 
    while((p = strstr(s, srem)) != NULL)
    {
      memmove(p, p + len, strlen(p + len) + 1);
      n++;
    }
  }
 
}

/**
 * Displays error with formatted variables
 * @param format the string to display with C format tags
 * @param ... list of args which correspond to C format tags in "format"
 */
void nstdError(const char *format, ...) {
    va_list args;

    fprintf(stderr, "Erreur: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}
