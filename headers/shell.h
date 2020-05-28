#ifndef SHELL_H
#define SHELL_H

#define MAX_READABLE 1000
#define BUFSIZE 1000
#define INPBUF 100
#define ARGMAX 10

char *input, *input1;
int filepid, fd[2];

char *argval[ARGMAX]; // our local argc, argv
char inputfile[INPBUF], outputfile[INPBUF];

int getInput();

void help();
void screenFetch();
void exitScreen();
void launchShell();

#endif