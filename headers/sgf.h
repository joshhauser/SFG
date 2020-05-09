#ifndef DISK_H
#define DISK_H

// Preproc consts
#define BLOCK_SIZE 1048576
#define INODES_COUNT 50
#define BLOCKS_COUNT 150
#define MAIN_FOLDER "/home/"
#define FOLDER_DELIMITER "||"

// Enums ================
/* typedef enum fileType_e {
  TEXT,
  BINARY, 
  FOLDER
} fileType_e; */

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
  char fileName[100];
  char rights[3];
  int usedBlocks[BLOCKS_COUNT];
} inode_t;

/* A block is a section of a disk which contains datas */
typedef char block_t[1048576];

// Disk
typedef struct disk_t {
  inode_t inodes[INODES_COUNT];
  block_t blocks[BLOCKS_COUNT];
} disk_t;

/* // Pair fileName + inode
typedef struct fileID_t {
  char fileName;
  inode_t inode;
} fileID_t;

// Directory
typedef struct directory_t {
  int nothing;
  fileID_t fileEntities[];
} directory_t;
 */

// File
typedef struct file_t {
  inode_t inode;
  accessMode_e mode;
} file_t;


// Functions
void initDisk();
void initDiskContent();
void saveDisk();
void diskFree();
void testContent();
void nstdError(const char *format, ...);
void removeFolder(char * folderName);
void rewriteFolderContent(inode_t * folderInode, char * folderContent, int usedBlocksCount);


int fileExists(char * fileName, char fileType, char * folderContent);

inode_t createFile(char * name, char fileType);


#endif