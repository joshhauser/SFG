#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/shell.h"
#include "headers/sgf.h"
#include "headers/functions.h"

int main(int argc, char *argv[])
{
  initDisk();
  launchShell(argc, argv);
  return 0;
}
