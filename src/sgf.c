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

  if ((diskFile = fopen("disk0", "r")) != NULL && fseek(diskFile, 0, SEEK_END) == 0) {
    // Read file to init structs
  }
  else {
    
    // Create the disk's file
    diskFile = fopen("disk0", "w");

    // Put [] delimiters on the first line
    fputs("[]\n", diskFile);

    char* buffer = malloc((BLOCK_SIZE + 3) * sizeof(char));
    
    // Put '[' delimiter
    buffer[0] = '[';
    // Put 1024 space chars after the '['
    memset(&buffer[1], ' ', BLOCK_SIZE * sizeof(char));
    // Put ']' at the end of the array
    buffer[1025] = ']'; 

    // Write buffer's content into the file disk
    fputs(buffer, diskFile);

    free(buffer);
    fclose(diskFile);    
  }

}

