#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "../headers/sgf.h"
#include "../headers/functions.h"

disk_t disk;
FILE* diskFile = NULL;
char * currentFolder = MAIN_FOLDER;
int availableBlocks = BLOCKS_COUNT;
int availableInodes = INODES_COUNT;


// Init the disk structure
void initDisk() {
  int i;
  int index = -1;
  int fileSize = -1;

  diskFile = fopen("disk0", "rb");
  if (diskFile != NULL) {
    index = fseek(diskFile, 0, SEEK_END);
    if (index == 0) fileSize = ftell(diskFile);
    if (fileSize == -1) {
      fclose(diskFile);
      initDiskContent();
    }
  }
  else {
    initDiskContent();
  }
  diskFile = fopen("disk0", "rb");
  // Put the file's index at the beginning
  fseek(diskFile, 0, SEEK_SET);
  // Get inodes
  for (i = 0; i < INODES_COUNT; i++) {
    fread(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
    if (strcmp(disk.inodes[i].fileName, "") != 0) availableInodes--;
  }
  // Get blocks
  for (i = 0; i < BLOCKS_COUNT; i++) {
    fread(&disk.blocks[i], sizeof(block_t), 1, diskFile);
    if (strcmp(disk.blocks[i], "") != 0) availableBlocks--;
  }

  // Close file
  fclose(diskFile);
}

// Init disk content with empty inodes & blocks
void initDiskContent() {
  int i;

  // Create the disk's file
  diskFile = fopen("disk0", "a+b");
  
  // Create main inode
  inode_t superInode;
  superInode.id = 0;
  strcpy(superInode.fileName, MAIN_FOLDER);
  strcat(superInode.fileName, "\0");
  strcpy(superInode.rights, "drw");

  for (i = 0; i < BLOCKS_COUNT; i++) superInode.usedBlocks[i] = -1;

  fwrite(&superInode, sizeof(inode_t), 1, diskFile);

  // Create inodes to init disk content
  inode_t inode;
  strcpy(inode.fileName, "");
  strcat(inode.fileName, "\0");
  strcpy(inode.rights, "");
  
  for (i = 0; i < BLOCKS_COUNT; i++) inode.usedBlocks[i] = -1;
  for (i = 1; i < INODES_COUNT; i++) {
    inode.id = i;
    fwrite(&inode, sizeof(inode_t), 1, diskFile);
  }

  // Create blocks to init disk content
  block_t block;
  strcpy(block, "");

  for (i = 0; i < BLOCKS_COUNT; i++) fwrite(&block, sizeof(block_t), 1, diskFile);

  fclose(diskFile);
}

/**
 * Create a new file
 * @param name the file name
 * @param fileType the type of the file (ordinary --> '-' or directory 'd');
 * */
inode_t createFile(char * name, char fileType) {
  inode_t currentFolderInode, newFileInode;
  int i, j;
  // Current folder content
  char * currentFolderContent;
  // Used blocks for current inode
  int usedBlocks[BLOCKS_COUNT];
  // Used blocks count for current inode
  int usedBlocksCount = 0;
  // New folder
  char * newFile;
  // String for available inode id
  char availableInodeID[4];

  // Check file name length
  if (strlen(name) > 100) {
    nstdError("Veuillez choisir un nom de moins de 100 caractères.\n");
    return newFileInode;
  }

  // Get inode of current folder
  for (i = 0; i < INODES_COUNT; i++) {
    if (strcmp(disk.inodes[i].fileName, currentFolder) ==  0) {
      currentFolderInode = disk.inodes[i];
      j = 0;
      while (currentFolderInode.usedBlocks[j] != -1 && j < BLOCKS_COUNT) {
        usedBlocksCount++;
        usedBlocks[usedBlocksCount-1] = currentFolderInode.usedBlocks[j];
        j++;
      }
      break;
    }
  }

  // Check if there are available inodes & blocks
  if (availableInodes == 0 || availableBlocks == 0) {
    nstdError("Veuillez supprimer des fichiers pour libérer de l'espace de stockage\n");
    return newFileInode;
  }

  // Search for free inode
  for (i = 0; i < INODES_COUNT; i++) {
    // If an inode has empty file name, it means it's free
    if (strcmp(disk.inodes[i].fileName, "") == 0) {
      newFileInode = disk.inodes[i];
      // Get string value of inode id
      sprintf(availableInodeID, "%d", newFileInode.id);
      break;
    }
  }

  // Strlen of folder name + strlen of availableInodeID + 4 chars for delimiters and \0
  newFile = (char*) malloc((strlen(name) + strlen(availableInodeID) + 4) * sizeof(char));
  strcpy(newFile, "<");
  strcat(newFile, availableInodeID);
  strcat(newFile, ":");
  strcat(newFile, name);
  strcat(newFile, ">");

  // If some blocks are used for the current folder
  if (usedBlocksCount != 0) {
    // Put folder content in an array
    currentFolderContent = malloc(sizeof(char));
    strcpy(currentFolderContent, "");
    for (i = 0; i < usedBlocksCount; i++) {
      currentFolderContent = realloc(currentFolderContent, (i+1) * BLOCK_SIZE * sizeof(char));
      strcat(currentFolderContent, disk.blocks[i]);
    }

    // Check if file with same name and same type exists
    int newFileExists = fileExists(name, fileType, currentFolderContent);
    if (newFileExists) {
      if (fileType == 'd') nstdError("Un répertoire avec ce nom existe déjà.\n");
      if (fileType == '-') nstdError("Un fichier avec ce nom existe déjà.\n");

      return newFileInode;
    }

    int lastBlock = usedBlocks[usedBlocksCount-1];
    int lbContentSize = strlen(disk.blocks[lastBlock]) * sizeof(char);
    int availableSpace = BLOCK_SIZE - lbContentSize;

    if ((lbContentSize < BLOCK_SIZE) && availableSpace >= ((strlen(newFile) + 3) * sizeof(char))) {
      strcat(disk.blocks[lastBlock], "||");
      strcat(disk.blocks[lastBlock], newFile);

      for (i = 0; i < BLOCKS_COUNT; newFileInode.usedBlocks[i++] = -1);
      newFileInode.rights[0] = fileType;
      strcpy(&newFileInode.rights[1], "rw"); 
      //newFileInode.fileName = name;
      strcpy(newFileInode.fileName, name);
      strcat(newFileInode.fileName, "\0");
    }
    else {
      if (availableBlocks >= 2) {
        for (i = 0; i < BLOCKS_COUNT; i++) {
          if (strcmp(disk.blocks[i], "") == 0) {
            usedBlocksCount++;
            // Use a new block
            strcpy(disk.blocks[i], newFile);

            for (j = 0; j < BLOCKS_COUNT; newFileInode.usedBlocks[j++] = -1);
            newFileInode.rights[0] = fileType;
            strcpy(&newFileInode.rights[1], "rw");
            strcpy(newFileInode.fileName, name);
            strcat(newFileInode.fileName, "\0");

            for (j = 0; j < BLOCKS_COUNT; j++) {
              if (currentFolderInode.usedBlocks[j] == -1) {
                currentFolderInode.usedBlocks[j] = i;
                break;
              }
            }
            break;
          }
        }
      }
      else {
        nstdError("Impossible de créer le dossier \"%s\". Il n'y a pas assez de place sur le disque.\n");
        return newFileInode;
      }
    }
  }
  else {
    if (availableBlocks >= 2) {
      for (i = 0; i < BLOCKS_COUNT; i++) {
        if (strcmp(disk.blocks[i], "") == 0) {
          usedBlocksCount++;
          strcpy(disk.blocks[i], newFile);

          for (j = 0; j < BLOCKS_COUNT; newFileInode.usedBlocks[j++] = -1);
          newFileInode.rights[0] = fileType;
          strcpy(&newFileInode.rights[1], "rw");
          //newFileInode.fileName = name;
          strcpy(newFileInode.fileName, name);
          strcat(newFileInode.fileName, "\0");

          for (j = 0; j < BLOCKS_COUNT; j++) {
            if (currentFolderInode.usedBlocks[j] == -1) {
              currentFolderInode.usedBlocks[j] = i;
              break;
            }
          }
          break;
        }
      }
    }
    else {
      nstdError("Impossible de créer le dossier \"%s\". Il n'y a pas assez de place sur le disque.\n");
      return newFileInode;
    }
  }

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == currentFolderInode.id) disk.inodes[i] = currentFolderInode;
    if (disk.inodes[i].id == newFileInode.id) disk.inodes[i] = newFileInode;
  }

  saveDisk();
  return newFileInode;
}

// Save changes
void saveDisk() { 
  int i;

  diskFile = fopen("disk0", "wb");
  for (i = 0; i < INODES_COUNT; i++) fwrite(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
  for (i = 0; i < BLOCKS_COUNT; i++) fwrite(&disk.blocks[i], sizeof(block_t), 1, diskFile);
  fclose(diskFile);    
}

/**
 * Displays error with formatted variables
 * @param format the string to display with C format tags
 * @param ... list of args which correspond to C format tags in "format"
 */
void nstdError(const char *format, ...) {
    va_list args;
    
    fprintf(stderr, "Erreur: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

/**
 * Check if a file exists in the current folder
 * @param fileName the name of the file to look for
 * @param fileType the type of the file (ordinary or directory)
 * @param folderContent the string which contains the pair <inode:file name> 
 * for all files of the current folder
 */
int fileExists(char * fileName, char fileType, char * folderContent) {
  int i, j;
  // Check if fileName is a substring of folderContent
  if (strstr(folderContent, fileName) != NULL) {
    int folderItemsCount;
    
    // Split folderContent to get an array which contains all pairs <inode:file name>
    char ** folderItems = splitStr(folderContent, "||", &folderItemsCount);
    printf("items count: %d\n", folderItemsCount);
    for (i = 0; i < folderItemsCount; i++) {
    printf("%s\n", folderItems[i]);
      // Check if the current element of folderItems contains fileName as a substring
      if (strstr(folderItems[i], fileName) != NULL) {
        int currentItemInodeID;

        // Get the inode id of the current item
        sscanf(folderItems[i], "%*c%d", &currentItemInodeID);
        for (j = 0; j < INODES_COUNT; j++) {
          if (disk.inodes[j].id == currentItemInodeID && strcmp(disk.inodes[j].fileName, fileName) == 0) {
            if (disk.inodes[j].rights[0] == fileType) {
              return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}

void testContent() {
  int i, j;

  for (i = 0; i < INODES_COUNT; i++) {
    for (j = 0; j < BLOCKS_COUNT; j++) {
      if (disk.inodes[i].usedBlocks[j] != -1) {
        if (strcmp(disk.blocks[disk.inodes[i].usedBlocks[j]], "") != 0) {
          printf("contenu en [%d, %d]: %s\n", i, j, disk.blocks[disk.inodes[i].usedBlocks[j]]);
        }
      }
    }
  }
}