#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/shell.h"
#include "headers/sgf.h"

int main() {
  initDisk();
  createFile("abc",'d'); 
  createFile("cactus",'-'); 
  // createFile("militaire", 'd'); 
  //createFile("tricycle", 'd');
  //testContent();
  //removeFolder("militaire");
  myls();
  diskFree();  
  return 0;
}
