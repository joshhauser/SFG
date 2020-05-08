#include <stdio.h>
#include <stdlib.h>
#include "headers/shell.h"
#include "headers/sgf.h"

int main() {
  initDisk();
  createFile("militaire", 'd'); 
  createFile("jambon", 'd'); 
  createFile("fromage", 'd'); 
  removeFolder("militaire");
  diskFree();  
  return 0;
}