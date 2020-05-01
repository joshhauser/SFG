#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "../headers/sgf.h"

/*
typedef struct disk_t {
  inode_t inodes[50];
  block_t blocks[150];
} disk_t;
*/

disk_t disk;

void initDisk() {
  FILE* diskFile = NULL;

  if ((diskFile = fopen("disk0", "r")) == NULL) {
    perror("Can't open disk");
    
    diskFile = fopen("disk0", "w");
    fputs("[]\n", diskFile);
    fputs("[]\n", diskFile);
    fclose(diskFile);
    
  }

  if (fseek(diskFile, 0, SEEK_END) != 0) {
    printf("The disk is empty.\n");
  }
}