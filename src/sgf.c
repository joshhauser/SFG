#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "../headers/sgf.h"
#include "../headers/functions.h"

disk_t disk;
inode_t currentFolderInode;
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
    currentFolderInode = disk.inodes[0];
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
  inode_t newFileInode;
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
  /*for (i = 0; i < INODES_COUNT; i++) {
    if (strcmp(disk.inodes[i].fileName, currentFolder) ==  0) {
      currentFolderInode = disk.inodes[i];
      
      
      break;
    }
  } */
  j = 0;
  while (currentFolderInode.usedBlocks[j] != -1 && j < BLOCKS_COUNT) {
    usedBlocksCount++;
    usedBlocks[usedBlocksCount-1] = currentFolderInode.usedBlocks[j];
    j++;
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
      strcat(currentFolderContent, disk.blocks[usedBlocks[i]]);
    }

    // Check if file with same name and same type exists
    int newFileExists = fileExists(name, fileType, currentFolderContent);
    if (newFileExists != -1) {
      if (fileType == 'd') nstdError("Un répertoire avec ce nom existe déjà.\n");
      if (fileType == '-') nstdError("Un fichier avec ce nom existe déjà.\n");

      return newFileInode;
    }

    int lastBlock = usedBlocks[usedBlocksCount-1];
    int lbContentSize = strlen(disk.blocks[lastBlock]) * sizeof(char);
    int availableSpace = BLOCK_SIZE - lbContentSize;

    if ((lbContentSize < BLOCK_SIZE) && availableSpace >= ((strlen(newFile) + 3) * sizeof(char))) {
      strcat(disk.blocks[lastBlock], FOLDER_DELIMITER);
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


  if (fileType == 'd') {
    char currentFolderInodeID[4];
    sprintf(currentFolderInodeID, "%d", currentFolderInode.id);
    char * newFolderContent = (char*) malloc(3 + strlen(currentFolderInodeID));
    strcpy(newFolderContent, "<");
    strcat(newFolderContent, currentFolderInodeID);
    strcat(newFolderContent, ":..>");
    
    int i, availableBlock;

    for (i = 0; i < BLOCKS_COUNT; i++) {
      if (strcmp(disk.blocks[i], "") == 0) {
        availableBlock = i;
        break;
      }
    }
    
    strcpy(disk.blocks[availableBlock], newFolderContent);
    
    newFileInode.usedBlocks[0] = availableBlock;
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
 * @return the inode id of the file if it exists, otherwise -1
 */
int fileExists(char * fileName, char fileType, char * folderContent) {
  int i, j;
  // Check if fileName is a substring of folderContent
  if (strstr(folderContent, fileName) != NULL) {
    int folderItemsCount;
    
    // Split folderContent to get an array which contains all pairs <inode:file name>
    char ** folderItems = splitStr(folderContent, FOLDER_DELIMITER, &folderItemsCount);
    for (i = 0; i < folderItemsCount; i++) {
      // Check if the current element of folderItems contains fileName as a substring
      if (strstr(folderItems[i], fileName) != NULL) {
        int currentItemInodeID;

        // Get the inode id of the current item
        sscanf(folderItems[i], "%*c%d", &currentItemInodeID);
        for (j = 0; j < INODES_COUNT; j++) {
          if (disk.inodes[j].id == currentItemInodeID && strcmp(disk.inodes[j].fileName, fileName) == 0) {
            if (disk.inodes[j].rights[0] == fileType) {
              return disk.inodes[j].id;
            }
          }
        }
      }
    }
  }

  return -1;
}


void removeFolder(char * folderName) {
  inode_t folderInode;
  char * currentFolderContent;
  int i;
  int usedBlocksCount = 0;

  i = 0;
  while (currentFolderInode.usedBlocks[i] != -1) {
    usedBlocksCount++;
    i++;
  }

  currentFolderContent = (char*) malloc(sizeof(char));
  strcpy(currentFolderContent, "");
  for (i = 0; i < usedBlocksCount; i++) {
    currentFolderContent = realloc(currentFolderContent, (i+1) * BLOCK_SIZE * sizeof(char));
    strcat(currentFolderContent, disk.blocks[currentFolderInode.usedBlocks[i]]);
  }

  int folderInodeID = fileExists(folderName, 'd', currentFolderContent);
  if (folderInodeID == -1) {
    nstdError("Le répertoire \"%s\" n'existe pas.\n", folderName);
    return;
  }

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == folderInodeID) {
      folderInode = disk.inodes[i];
      break;
    }
  }

  // Number of items in the folder to delete
  int folderItemsCount;
  // Items of the folder to delete
  char ** folderItems = splitStr(disk.blocks[folderInode.usedBlocks[0]], FOLDER_DELIMITER, &folderItemsCount);

  // Check if the folder is empty: if it is, the only item should be the link to the parent directory
  if (folderItemsCount > 1) {
    nstdError("Le répertoire \"%s\" n'est pas vide\n");
  }

  int currentFolderItemsCount;
  char ** currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &currentFolderItemsCount);

  strcpy(currentFolderContent, "");
  for (i = 0; i < currentFolderItemsCount; i++) {
    int currentItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &currentItemInodeID);

    if (currentItemInodeID != folderInodeID) {
      strcat(currentFolderContent, currentFolderItems[i]);
      if (i < (currentFolderItemsCount-1)) strcat(currentFolderContent, FOLDER_DELIMITER);
    }
  }

  rewriteFolderContent(&currentFolderInode, currentFolderContent, usedBlocksCount);

  strcpy(folderInode.fileName, "");
  strcpy(disk.blocks[folderInode.usedBlocks[0]], "");
  folderInode.usedBlocks[0] = -1;
  
  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == currentFolderInode.id) {
      disk.inodes[i] = currentFolderInode;
    }
    else if (disk.inodes[i].id == folderInode.id) {
      disk.inodes[i] = folderInode;
    }
  }

  saveDisk();
}


void rewriteFolderContent(inode_t * folderInode, char * folderContent, int usedBlocksCount) {
  int folderItemsCount, i, j;
  int remainingSpace;
  char ** folderItems = splitStr(folderContent, FOLDER_DELIMITER, &folderItemsCount);

  for (i = 0; i < usedBlocksCount; strcpy(disk.blocks[folderInode->usedBlocks[i++]], ""));

  for (i = 0; i < folderItemsCount; i++) {
    for (j = 0; j < usedBlocksCount; j++) {
      remainingSpace = BLOCK_SIZE - strlen(disk.blocks[folderInode->usedBlocks[j]]);

      if (remainingSpace >= strlen(folderItems[i])) {
        strcat(disk.blocks[folderInode->usedBlocks[j]], folderItems[i]);
        remainingSpace = BLOCK_SIZE - strlen(disk.blocks[folderInode->usedBlocks[j]]);

        if (remainingSpace >= strlen(FOLDER_DELIMITER) && i < (folderItemsCount -1)) {
          strcat(disk.blocks[folderInode->usedBlocks[j]], FOLDER_DELIMITER);
        }
        break;
      }
    }
  }

  if (strcmp(disk.blocks[folderInode->usedBlocks[usedBlocksCount-1]], "") == 0)
    folderInode->usedBlocks[usedBlocksCount-1] = -1;
}



/**
 * A faire:
 * - move()
 * - copy()
 * - link()
 * - read()
 * - write()
 * - ls()
 * - cd()
 * - open()
 * - close()
 **/

void diskFree() {
  int remainingSpace = 0;
  int i;

  for (i = 0; i < BLOCKS_COUNT; i++) {
    remainingSpace += (BLOCK_SIZE - strlen(disk.blocks[i]));
  }

  printf("Free inodes:\t%d / %d\n", availableInodes, INODES_COUNT);
  printf("Free blocks:\t%d / %d\n", availableBlocks, BLOCKS_COUNT);
  printf("Free bytes:\t%d / %d\n", remainingSpace, BLOCK_SIZE);
}

// TESTS -------
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