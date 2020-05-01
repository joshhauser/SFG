#ifndef SHELL_H
#define SHELL_H

#define MAX_READABLE 1000

void initShell();
void prompt();
int readCommand(char*** args);
void cleanBuffer();

#endif