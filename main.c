#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/shell.h"
#include "headers/sgf.h"

int main() {
  initDisk();
  createFile("militaire", 'd'); 
  createFile("cactus", 'd'); 
  
  createFile("test.txt", '-');
  /* file_t file = openFile("test.txt", W);
  printf("%d\n", file.inodeID);
  writeFile(file, "Sandwich", strlen("Sandwich"));
  closeFile(file); */
  testContent();
  return 0;
}