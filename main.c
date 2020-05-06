#include <stdio.h>
#include <stdlib.h>
#include "headers/shell.h"
#include "headers/sgf.h"

int main() {
  initDisk();
  createFile("militaire", 'd'); 
  testContent();
  return 0;
}