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
  A // append
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

// File
typedef struct file_t {
  int inodeID;
  accessMode_e mode;
} file_t;


// Functions ===============
void initDisk();
void initDiskContent();
void saveDisk();
void diskFree();
void testContent();
void testCount();
void myls();
void closeFile(file_t file);
void removeFile(char *fileName);
void removeFolder(char * folderName);
void nstdError(const char *format, ...);
void writeFile(file_t file, char *buffer, int bufferSize);
void readFile(file_t file, char **buffer, int bufferSize);
void move(char *source, char *destination);
void copy(char *source, char *destination);
inode_t createFile(char * name, char fileType);

void copyFile(inode_t fileInode, char *content);
void rewriteFolderContent(inode_t * folderInode, char * folderContent);
void countFolderBlocks(inode_t folderInode, int *blocksNumber);

int changeDirectory(char *newDir);
int fileExists(char * fileName, char fileType, char * folderContent);
int getRemainingSpace(char *content);

char *getFileContent(inode_t folder);
char *getFileNameByID(int inodeID);

inode_t getInodeByID(int inodeID);
file_t openFile(char * fileName, accessMode_e mode);

#endif
