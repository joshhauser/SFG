#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "../headers/sgf.h"
#include "../headers/functions.h"

disk_t disk;
inode_t currentFolderInode;
FILE* diskFile = NULL;
char * currentFolder = MAIN_FOLDER;
int availableBlocks = BLOCKS_COUNT;
int availableInodes = INODES_COUNT;


// Inits the disk structure
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
  // Puts the file's index at the beginning
  fseek(diskFile, 0, SEEK_SET);
  // Gets inodes
  for (i = 0; i < INODES_COUNT; i++) {
    fread(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
    currentFolderInode = disk.inodes[0];
    if (strcmp(disk.inodes[i].fileName, "") != 0) availableInodes--;
  }
  // Gets blocks
  for (i = 0; i < BLOCKS_COUNT; i++) {
    fread(&disk.blocks[i], sizeof(block_t), 1, diskFile);
    if (strcmp(disk.blocks[i], "") != 0) availableBlocks--;
  }


  // Closes file
  fclose(diskFile);
}

// Inits disk content with empty inodes & blocks
void initDiskContent() {
  int i;

  // Creates the disk's file
  diskFile = fopen("disk0", "a+b");
  
  // Create main inode
  inode_t superInode;
  superInode.id = 0;
  strcpy(superInode.fileName, MAIN_FOLDER);
  strcat(superInode.fileName, "\0");
  strcpy(superInode.rights, "drw");

  for (i = 0; i < BLOCKS_COUNT; i++) superInode.usedBlocks[i] = -1;

  fwrite(&superInode, sizeof(inode_t), 1, diskFile);

  // Creates inodes to init disk content
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
 * Creates a new file
 * @param name the file name
 * @param fileType the type of the file (ordinary --> '-' or directory 'd');
 * @return the inode of the created file
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
    currentFolderContent = getFolderContent(currentFolderInode);
    
    // Check if file with same name and same type exists
    if (fileExists(name, '-', currentFolderContent) != -1 || fileExists(name, 'd', currentFolderContent) != -1) {
      nstdError("Un fichier avec ce nom existe déjà.\n");
      return newFileInode;
    }

    int lastBlock = usedBlocks[usedBlocksCount-1];
    int lbContentSize = strlen(disk.blocks[lastBlock]) * sizeof(char);
    int availableSpace = BLOCK_SIZE - lbContentSize;

    if ((lbContentSize < BLOCK_SIZE) && availableSpace >= ((strlen(newFile) + strlen(FOLDER_DELIMITER) + 1) * sizeof(char))) {
      strcat(disk.blocks[lastBlock], FOLDER_DELIMITER);
      strcat(disk.blocks[lastBlock], newFile);

      for (i = 0; i < BLOCKS_COUNT; newFileInode.usedBlocks[i++] = -1);
      newFileInode.rights[0] = fileType;
      strcpy(&newFileInode.rights[1], "rw"); 
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

  /* If the new file is a directory, it's necessary to add
  the parent folder path to it, to allow "cd .." execution */
  if (fileType == 'd') {
    char currentFolderInodeID[4];
    sprintf(currentFolderInodeID, "%d", currentFolderInode.id);
    char * newFolderContent = (char*) malloc(4 + strlen(currentFolderInodeID) * sizeof(char));
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

// Saves changes
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

/**
 * Removes an empty folder
 * @param folderName the name of the folder to remove
 **/
void removeFolder(char * folderName) {
  inode_t folderInode;
  char * currentFolderContent = getFolderContent(currentFolderInode);
  int i;
  int usedBlocksCount = 0;


  int folderInodeID = fileExists(folderName, 'd', currentFolderContent);
  if (folderInodeID == -1) {
    nstdError("Le répertoire \"%s\" n'existe pas.\n", folderName);
    return;
  }

  folderInode = getInodeByID(folderInodeID);

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
      availableInodes++;
    }
  }

  saveDisk();
}

/**
 * Rewrites folder content when a file is deleted
 * @param folderInode the folder of which the content will be rewrote
 * @param folderContent the content to rewrite
 * @param usedBlocksCount the number of used blocks for the folder
 **/
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

  if (strcmp(disk.blocks[folderInode->usedBlocks[usedBlocksCount-1]], "") == 0) {
    folderInode->usedBlocks[usedBlocksCount-1] = -1;
    availableBlocks++;
  }
}


/**
 * Opens a file
 * @param fileName the name of the file to open
 * @param mode the mode for the file opening (R, W, RW)
 * @return the file structure that contains
 * the inode ID of the file + the access mode
 **/
file_t openFile(char * fileName, accessMode_e mode) {
  file_t file;
  char * currentFolderContent = getFolderContent(currentFolderInode);

  int fileInodeID = fileExists(fileName, '-', currentFolderContent);
  if (fileInodeID != -1) {
    file.inodeID = fileInodeID;
  }
  else {
    inode_t fileInode = createFile(fileName, '-');
    file.inodeID = fileInode.id;
  }
  file.mode = mode;  
  
  return file;
}

/**
 * Closes a file
 * @param file the structure that corresponds to the file to close
 **/
void closeFile(file_t file) {
  if (file.inodeID != -1) {
    file.inodeID = -1;
    file.mode = -1;
  }
}

/**
 * A faire:
 * - move()
 * - copy()
 * - link()
 * - cd()
 **/
/* 
void myls() {
   int i, j;
   //Displays file names inside the current folder's blocks
   int init_size = strlen(disk.blocks);
   char delim[] = "||";
   char *ptr = strtok(disk.blocks, delim);
   while (ptr != NULL)
	{
		if ( disk.inodes[j].rights[0] == 'd' ) 
		{
			printf("[d]");
		}
		else if ( disk.inodes[j].rights[0] == '-' ) {
			printf("[f] '%c'\n", ptr);
		}
			
		for (i=0;i<strlen(ptr);i++)
		{
			if(( ptr[i] != '<'  &&  ptr[i] != '>'  && ptr[i] != ':' ) && ( ptr[i]<'0' || ptr[i]>'9' ))
			{
				printf("%c", ptr[i]);
			}
		}
		printf("    ");
		ptr = strtok(NULL, delim);
		j++;
	}
	
	printf("\n");
}

*/
void myls() {
	int i, j;
   //Split the disk's block content for display
  
   int init_size = strlen(disk.blocks);
   char delim[] = "||";
   char *ptr = strtok(disk.blocks, delim);
   while (ptr != NULL)
	{
		if ( disk.inodes[j].rights[0] == 'd' ) 
		{
			printf("[d]");
		}
		else if ( disk.inodes[j].rights[0] == '-' ) {
			printf("[f] '%c'\n", ptr);
		}
			
		for (i=0;i<strlen(ptr);i++)
		{
			if(( ptr[i] != '<'  &&  ptr[i] != '>'  && ptr[i] != ':' ) && ( ptr[i]<'0' || ptr[i]>'9' ))
			{
				printf("%c", ptr[i]);
			}
		}
		printf("    ");
		ptr = strtok(NULL, delim);
		j++;
	}
	
	printf("\n");
}

/**
 * Writes a buffer's content in a file
 * @param file the file to write to
 * @param buffer the content to add to the file
 * @param bufferSize the number of the buffer's bytes
 **/
void writeFile(file_t file, char *buffer, int bufferSize) {
  int i, j, newBlock;
  int bufferPos = 0;
  inode_t fileInode;
  // Necessary blocks to write buffer's content
  int necessaryBlocksCount = ceil((double) bufferSize / BLOCK_SIZE);

  if (availableBlocks == 0) {
    nstdError("Il n'y a plus de place sur le disque.\n");
    return;
  }
  if (file.inodeID == -1 || (file.mode != W && file.mode != A)) {
    return;
  }
  else {
    fileInode = getInodeByID(file.inodeID);
  }

  // If the mode is W, the existing content is erased
  if (file.mode == W) {
    i = 0;
    while (fileInode.usedBlocks[i] != -1) {
      strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
      fileInode.usedBlocks[i] == -1;
      i++;
    }
    goto writeFileContent;
  }

  int usedBlocksCount = 0;
  i = 0; 
  while (fileInode.usedBlocks[i] != -1) {
    usedBlocksCount++;
    i++;
  }
  
  if (usedBlocksCount > 0) {
    // Last block's content
    char lastBlockContent[BLOCK_SIZE];
    strcpy(lastBlockContent, disk.blocks[fileInode.usedBlocks[usedBlocksCount-1]]);
    // Last block's remaining space
    int lbRemainingSpace = getRemainingSpace(lastBlockContent);

    // If there is enough free bytes in the last bloc
    if (lbRemainingSpace >= bufferSize) {
      strcat(lastBlockContent, buffer);
      strcpy(disk.blocks[fileInode.usedBlocks[usedBlocksCount-1]], lastBlockContent);
      saveDisk();
      return;
    }
    else {
      if (availableBlocks >= necessaryBlocksCount) {
        // If there are some free bytes in the last used block
        if (lbRemainingSpace > 0) {
          for (i = 0; i < lbRemainingSpace; i++) {
            //strcat(lastBlockContent, buffer[i]);
            lastBlockContent[BLOCK_SIZE - lbRemainingSpace + i] = buffer[i];
          }
          bufferPos = i;
          strcpy(disk.blocks[fileInode.usedBlocks[usedBlocksCount-1]], lastBlockContent);
        }
      }
      else{
        nstdError("Il n'y a pas assez de place sur le disque.\n");
        return;
      }
    }
  }

  writeFileContent:
  for (i = 0; i < necessaryBlocksCount; i++) {
    for (j = 0; j < BLOCKS_COUNT; j++) {
      // Get new block
      if (strcmp(disk.blocks[j], "") == 0) {
        newBlock = j;
        break;
      }
    }

    j = 0;
    // Write bytes from the buffer while there is a free byte in the current block
    while (getRemainingSpace(disk.blocks[newBlock]) >= 1 && bufferPos < bufferSize) {
      disk.blocks[newBlock][j] = buffer[bufferPos];
      j++;
      bufferPos++;
    }

    usedBlocksCount++;
    availableBlocks--;
    // Add the new block to fileInode.usedBlocks array
    fileInode.usedBlocks[usedBlocksCount-1] = newBlock;
  }

  // Update inode
  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == fileInode.id) {
      disk.inodes[i] = fileInode;
      break;
    }
  }
  saveDisk();
}

void readFile(file_t file, char **buffer, int bufferSize) {
  inode_t fileInode;
  int i = 0;
  int usedBlocksCount = 0;

  if (file.inodeID == -1 || (file.mode != R && file.mode != A)) {
    return;
  }
  else {
    fileInode = getInodeByID(file.inodeID);
  }

  while (fileInode.usedBlocks[i] != -1) {
    usedBlocksCount++;
    i++;
  }

  if (usedBlocksCount == 0) {
    *buffer = (char*) malloc(sizeof(char));
    *buffer = "";
    return;
  }
  
  char * fileContent = (char*) malloc(sizeof(char));
  strcpy(fileContent, "");
  for (i = 0; i < usedBlocksCount; i++) {
    fileContent = (char*) realloc(fileContent, ((i+1) * BLOCK_SIZE) * sizeof(char));
    strcat(fileContent, disk.blocks[fileInode.usedBlocks[i]]);
  }

  if (strlen(fileContent) < bufferSize) {
    *buffer = fileContent;
  }
  else {
    *buffer = (char*) malloc(bufferSize * sizeof(char));
    for (i = 0; i < bufferSize; i++) {
      (*buffer)[i] = fileContent[i];
    }
  }
}

void moveFile(char *source, char *destination) {
  int i, j;
  char *currentFolderContent = getFolderContent(currentFolderInode);
  int sourceInodeID, destinationInodeID;
  inode_t sourceInode, destinationInode;
  
  // Check if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1) {
    nstdError("Ce fichier n'existe pas.\n");
    return;
  }
  
  // Get the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);
 
  // The source is a folder
  if (strstr(destination, "/") != NULL) {
    // Remove '/'
    /* Note: if the destination paths has multiple folder,
    it won't works and an error will be thrown, because this program
    doesn't take that type of paths in account */
    destination[strlen(destination) - 1] = '\0';
    destinationInodeID = fileExists(destination, 'd', currentFolderContent);
    // Check if the source folder exists
    if (destinationInodeID != -1) {
      destinationInode = getInodeByID(destinationInodeID);
      // String value of source's inode id
      char stringValueOfSourceID[4];
      sprintf(stringValueOfSourceID, "%d", destinationInodeID);

      char *fileToMove = malloc((strlen(source) + 4) * sizeof(char));
      strcpy(fileToMove, "<");
      strcat(fileToMove, stringValueOfSourceID);
      strcat(fileToMove, ":>");
      strcat(fileToMove, "\0");

      // Last block used by the source folder
      int lastUsedBlock;

      i = 0;
      while (destinationInode.usedBlocks[i] != -1) {
        lastUsedBlock = destinationInode.usedBlocks[i];
        i++;
      }

      // Remaining space in the last used block
      int lbRemainingSpace = getRemainingSpace(disk.blocks[lastUsedBlock]);
      // Check if there is enough free bytes to add the new entry to the source folder
      if (lbRemainingSpace >= (strlen(destination) + strlen(FOLDER_DELIMITER))) {
        strcat(disk.blocks[lastUsedBlock], FOLDER_DELIMITER);
        strcat(disk.blocks[lastUsedBlock], fileToMove);
      }
      else {
        for (i = 0; i < BLOCKS_COUNT; i++) {
          // Get a free block
          if (strcmp(disk.blocks[i], "") == 0) {
            strcpy(disk.blocks[i], fileToMove);
            for (j = 0; j < BLOCKS_COUNT; j++) {
              if (sourceInode.usedBlocks[j] == -1) {
                sourceInode.usedBlocks[j] = i;
                break;
              }
            }
            break;
          }
        }
      }
    }
    else  {
      nstdError("Le répertoire %s n'existe pas.\n", destination);
      return;
    }
  }
  else {
    strcpy(sourceInode.fileName, destination);
  }

  for (i = 0 ; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == sourceInode.id) {
      disk.inodes[i] = sourceInode;
      break;
    }
  }

  removeFile(source);
  saveDisk();
}

void copyFile(char *source, char *destination) {
  int i;
  char *sourceFileContent;
  char *currentFolderContent = getFolderContent(currentFolderInode);
  int sourceInodeID, destinationInodeID.
  inode_t sourceInode, destinationInode;

  // Check if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1) {
    nstdError("Ce fichier n'existe pas.\n");
    return;
  }
  
  // Get the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);

  // Get source file content
  sourceFileContent = (char*) malloc(sizeof(char));
  strcpy(sourceFileContent, "");
  i = 0;
  while (sourceInode.usedBlocks[i] != -1) {
    sourceFileContent = (char*) realloc(sourceFileContent, (1 + (i+1) * BLOCK_SIZE) * sizeof(char));
    strcat(sourceFileContent, disk.blocks[sourceInode.usedBlocks[i]]);
    i++;
  }

  sourceFileContent[strlen(sourceFileContent)] = '\0';

  // The source is a folder
  if (strstr(destination, "/") !=  NULL) {
    // Remove '/'
    /* Note: if the destination paths has multiple folder,
    it won't works and an error will be thrown, because this program
    doesn't take that type of paths in account */
    destination[strlen(destination) - 1] = '\0';
    destinationInodeID = fileExists(destination, 'd', currentFolderContent);

    // Check if the source folder exists
    if (destinationInodeID != -1) {
      destinationInode = getInodeByID(destinationInodeID);
      changeDirectory(destination);
      file_t copy = openFile(source, W);
      writeFile(copy, sourceFileContent, strlen(sourceFileContent));
      closeFile(copy);
      changeDirectory("..");
    }
    else {
      nstdError("Le répertoire %s n'existe pas.\n", destination);
      return;
    }
  }
  else {
    file_t copy = openFile(destination, W);
    writeFile(copy, sourceFileContent, strlen(sourceFileContent));
    closeFile(copy);
  }

  saveDisk();
}


int changeDirectory(char *newDir) {
  int i;
  int newDirInodeID;
  char *currentFolderContent = getFolderContent(currentFolderInode);

  newDirInodeID = fileExists(newDir, 'd', currentFolderContent);

  if (newDirInodeID == -1) {
    nstdError("Le répertoire %s n'existe pas.\n", newDir);
    return -1;
  }

  currentFolderInode = getInodeByID(newDirInodeID);
  return 0;
}


/**
 * Computes the remaining size in a block
 * @param content the block's content
 * @return the remaining size
 **/
int getRemainingSpace(char *content) {
  return BLOCK_SIZE - strlen(content);
}

/**
 * Gets an inode by its id
 * @param inodeID the id of the inode to get
 * @return the founded inode
 **/
inode_t getInodeByID(int inodeID) {
  inode_t inode;
  int i;
  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == inodeID) {
      inode = disk.inodes[i];
      break;
    }
  }

  return inode;
}

/**
 * Gets a file name by the file's inode id
 * @param inodeID the id of the file's inode
 * @return the founded file name
 **/
char *getFileNameByID(int inodeID) {
  int i;
  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == inodeID) return disk.inodes[i].fileName;
  }

  return NULL;
}

/**
 * Get the content of the given folder
 * @return the content of the given folder
 **/
char *getFolderContent(inode_t folder) {
  int usedBlocksCount = 0;
  int i;
  char * folderContent;

  if (folder.rights[0] != 'd') {
    nstdError("Le répertoire %s n'existe pas.\n", folder.fileName);
    return NULL;
  }

  i = 0;
  while (folder.usedBlocks[i] != -1) {
    usedBlocksCount++;
    i++;
  }

  folderContent = (char*) malloc(sizeof(char));
  strcpy(folderContent, "");
  for (i = 0; i < usedBlocksCount; i++) {
    folderContent = realloc(folderContent, (i+1) * BLOCK_SIZE * sizeof(char));
    strcat(folderContent, disk.blocks[folder.usedBlocks[i]]);
  }

  return folderContent;
}

void removeFile(char *fileName) {
  inode_t fileInode;
  char *currentFolderContent = getFolderContent(currentFolderInode);
  int i;

  int fileInodeID = fileExists(fileName, '-', currentFolderContent);
  if (fileInodeID == -1) {
    nstdError("Le fichier %s n'existe pas.\n", fileName);
    return;
  }

  fileInode = getInodeByID(fileInodeID);

  i = 0;
  while (fileInode.usedBlocks[i] != -1) {
    strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
    fileInode.usedBlocks[i] = -1;
    availableBlocks++;
    i++;
  }

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == fileInodeID) {
      disk.inodes[i] = fileInode;
      availableInodes++;
      break;
    }
  }

  saveDisk();
}

void diskFree() {
  int remainingSpace = 0;
  int i;

  for (i = 0; i < BLOCKS_COUNT; i++) {
    remainingSpace += (BLOCK_SIZE - strlen(disk.blocks[i]));
  }

  printf("Free inodes:\t%d / %d\n", availableInodes, INODES_COUNT);
  printf("Free blocks:\t%d / %d\n", availableBlocks, BLOCKS_COUNT);
  printf("Free bytes:\t%d / %d\n", remainingSpace, BLOCK_SIZE*BLOCKS_COUNT);
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
