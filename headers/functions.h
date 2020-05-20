#ifndef FUNCTIONS_H
#define FUNCTIONS_H

char ** splitStr(char * string, const char * delimiter, int * bufferSize);
char * strDup(char * str);

int countOcc(char *string, char searchedChar);

void nstdError(const char *format, ...);
#endif
