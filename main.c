#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/shell.h"
#include "headers/sgf.h"
#include "headers/functions.h"

int main() {
  initDisk();
  createFile("abc",'d');
  createFile("def", 'd');

  createFile("cactus",'-');
  file_t f = openFile("cactus", W);
  char *c = "test";
  writeFile(f, c, strlen(c));
  closeFile(f);
  char s[10] = "cactus";
  char dd[20] = "newFile.txt";
  move(s, dd); 

  testContent();
  return 0;
}