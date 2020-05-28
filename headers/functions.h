#ifndef FUNCTIONS_H
#define FUNCTIONS_H

char **splitStr(char *string, const char *delimiter, int *bufferSize);
char *strDup(char *str);
int countOcc(char *string, char searchedChar);
void remove_char(char *word, int idxToDel);
void cleanFileName(char *word);
void remove_string(char *s, char const *srem);
void nstdError(const char *format, ...);
#endif
