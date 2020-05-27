#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/shell.h"
#include "headers/sgf.h"
#include "headers/functions.h"

int main(int argc, char* argv[]){
  initDisk();
  //createFile("abc",'-');
  //file_t file;
  //file = openFile("abc",W);
  
  //char str[] = "ceci est un test";
  //writeFile(file,str,strlen(str));
 // readFile(file,&str,strlen(str));
 // closeFile(file);
  //writeFile(file_t file, char *buffer, int bufferSize)
  launch_shell(argc,argv);
  //createFile("abc",'d');
  //createFile("def", 'd');
  //char s[10] = "abc";
  //char d[10] = "def";
  //move(s, d);
  testContent();

  return 0;
}
