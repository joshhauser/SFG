#ifndef DISK_H
#define DISK_H

// Preproc consts
#define BLOCK_SIZE 1024

// Enums ================
typedef enum fileType_e {
  TEXT,
  BINARY, 
  FOLDER
} fileType_e;

typedef enum accessMode_e {
  R, // read
  W, // write
  RW // read & write
} accessMode_e;

// Structs ===============

/* An inode is a structure which gives information
about a file (ordinary file or folder) */
typedef struct inode_t {
  unsigned int id;
  unsigned int size;
  fileType_e fileType;
  unsigned char rights[8];
  int usedBlocks[150];
} inode_t;

/* A block is a section of a dosk which contains datas */
typedef char block_t[1048576];

// Disk
typedef struct disk_t {
  inode_t inodes[50];
  block_t blocks[150];
} disk_t;

// Couple filename + inode
typedef struct fileID_t {
  char filename;
  inode_t inode;
} fileID_t;

// Directory
typedef struct directory_t {
  int nothing;
  fileID_t fileEntities[];
} directory_t;

// File
typedef struct file_t {
  inode_t inode;
  accessMode_e mode;
} file_t;


// Functions
void initDisk();

#endif