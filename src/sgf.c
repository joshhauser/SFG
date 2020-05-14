#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#include "../headers/sgf.h"
#include "../headers/functions.h"

/**
 * A faire:
 * - link()
 * - unlink()
 * - ajout de la date de modification
 **/


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
    //printf(".......%d\n", disk.inodes[i].usedBlocks[0]);
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
 * @param fileType the type of the file:
 * - 'l' --> link
 * - 'd' --> directory
 * - '-' --> ordinary file
 * @return the inode of the created file
 * */
inode_t createFile(char * name, char fileType) {
  inode_t newFileInode;
  int i, j;
  // Current folder content
  char * currentFolderContent;
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

  i = 0;
  while (currentFolderInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
    usedBlocksCount++;
    i++;
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
    currentFolderContent = getFileContent(currentFolderInode);
    
    // Check if file with same name and same type exists
    if (fileExists(name, '-', currentFolderContent) != -1 || fileExists(name, 'd', currentFolderContent) != -1) {
      nstdError("Un fichier avec ce nom existe déjà.\n");
      return newFileInode;
    }

    int lastBlock = currentFolderInode.usedBlocks[usedBlocksCount-1];
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

      goto addParentToNewFolder;
    }
    else {
      goto getNewBlock;
    }
  }
  else {
    goto getNewBlock;
  }

  getNewBlock:
    for (i = 0; i < BLOCKS_COUNT; i++) {
      if (strcmp(disk.blocks[i], "") == 0) {
        usedBlocksCount++;
        strcpy(disk.blocks[i], newFile);

        for (j = 0; j < BLOCKS_COUNT; newFileInode.usedBlocks[j++] = -1);
        newFileInode.rights[0] = fileType;
        strcpy(&newFileInode.rights[1], "rw");
        strcpy(newFileInode.fileName, name);
        strcat(newFileInode.fileName, "\0");

        currentFolderInode.usedBlocks[usedBlocksCount-1] = i;
        break;
      }
    }

  addParentToNewFolder:
    /* If the new file is a directory, it's necessary to add
    the parent folder path to it, to allow "cd .." execution */
    if (fileType == 'd') {
      char currentFolderInodeID[4];
      sprintf(currentFolderInodeID, "%d", currentFolderInode.id);
      char * newFolderContent = (char*) malloc(4 + strlen(currentFolderInodeID) * sizeof(char));
      strcpy(newFolderContent, "<");
      strcat(newFolderContent, currentFolderInodeID);
      strcat(newFolderContent, ":..>");
      
      int availableBlock;

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
          if (disk.inodes[j].id == currentItemInodeID) {
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
  char * currentFolderContent = getFileContent(currentFolderInode);
  int i;

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

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  strcpy(folderInode.fileName, "");
  strcpy(disk.blocks[folderInode.usedBlocks[0]], "");
  folderInode.usedBlocks[0] = -1;
  availableBlocks++;
  
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
void rewriteFolderContent(inode_t *folderInode, char *folderContent) {
 
  int i, j, newBlock;
  int usedBlocksCount = 0;
  int bufferPos = 0;
  int bufferSize = strlen(folderContent);
  int necessaryBlocks = (int) ceil((double) bufferSize / BLOCK_SIZE);


  i = 0;
  while (folderInode->usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
    strcpy(disk.blocks[folderInode->usedBlocks[i]], "");
    folderInode->usedBlocks[i] = -1;
    availableBlocks++;
    i++;
  }

  for (i = 0; i < necessaryBlocks; i++) {
    for (j = 0 ; j < BLOCKS_COUNT; j++) {
      if (strcmp(disk.blocks[j], "") == 0) {
        availableBlocks--;
        newBlock = j;
        break;
      }
    }

    j = 0;

    // Write bytes from the buffer while there is a free byte in the current block
    while (getRemainingSpace(disk.blocks[newBlock]) >= 1 && bufferPos < bufferSize) {
      disk.blocks[newBlock][j] = folderContent[bufferPos];
      j++;
      bufferPos++;
    }

    disk.blocks[newBlock][j] = '\0';
    usedBlocksCount++;
    folderInode->usedBlocks[usedBlocksCount-1] = newBlock;
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
  char * currentFolderContent = getFileContent(currentFolderInode);

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


void myls() {
	int i, j;
  //Split the disk's block content for display
  
  int init_size = strlen(*disk.blocks);
  char delim[] = "||";
  char * ptr = strtok(*disk.blocks, delim);
    printf("i: %d, j: %d, strlen: %d\n", i, j, init_size);
  while (ptr != NULL)
	{
		//printf(" %c", disk.inodes[j].rights[0]);
		if ( disk.inodes[j].rights[0] == '-' ) {
			printf("[f]");
		}
		else if ( disk.inodes[j].rights[0] == 'd' ) 
		{
			printf("[d]");
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
  int usedBlocksCount = 0;
  inode_t fileInode;
  // Necessary blocks to write buffer's content
  int necessaryBlocksCount = ceil((double) bufferSize / BLOCK_SIZE);

  int remainingSpace = 0;

  for (i = 0; i < BLOCKS_COUNT; i++) {
    remainingSpace += getRemainingSpace(disk.blocks[i]);
  }

  if (remainingSpace < bufferSize) {
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
    while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
      strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
      fileInode.usedBlocks[i] = -1;
      availableBlocks++;
      i++;
    }
    goto writeFileContent;
  }

  i = 0; 
  while (fileInode.usedBlocks[i] != -1  && i < BLOCKS_COUNT) {
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

  while (fileInode.usedBlocks[i] != -1  && i < BLOCKS_COUNT) {
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

void move(char *source, char *destination) {
  int i;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int sourceInodeID, destinationInodeID;
  inode_t sourceInode, destinationInode;
  
  // Check paths
  if (countOcc(source, '/') > 1 || countOcc(destination, '/') > 1) {
    nstdError("Erreur lors de la copie.\n");
    return;
  }

  if (strstr(source, "/") != NULL) {
    // Remove '/'
    source[strlen(source) - 1] = '\0';
  }

  if (strstr(destination, "/") !=  NULL) {
    // Remove '/'
    destination[strlen(destination) - 1] = '\0';
  }

  // Check if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1) {
    nstdError("Ce fichier n'existe pas.\n");
    return;
  }

  // Get the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);

  if (sourceInode.rights[0] == 'd') {
    if ((destinationInodeID = fileExists(destination, '-', currentFolderContent)) != -1) {
      nstdError("Impossible de déplacer un répertoire dans un fichier.\n");
      return;
    }
    else if ((destinationInodeID = fileExists(destination, 'd', currentFolderContent)) != -1) {
      destinationInode = getInodeByID(destinationInodeID);
    }
    else {
      destinationInode = createFile(destination, 'd');
    }

  }
  else if (sourceInode.rights[0] == '-') {
    if ((destinationInodeID = fileExists(destination, '-', currentFolderContent)) != -1) {
      char *sourceContent = getFileContent(sourceInode);
      int sourceContentLength = strlen(sourceContent);
      file_t target = openFile(destination, W);
      writeFile(target, sourceContent, sourceContentLength);
      closeFile(target);
      removeFile(source);
      return;
    }
    else if ((destinationInodeID = fileExists(destination, 'd', currentFolderContent)) != -1) {
      destinationInode = getInodeByID(destinationInodeID);
    }
    else {
      strcpy(sourceInode.fileName, destination);
      for (i = 0; i < INODES_COUNT; i++) {
        if (disk.inodes[i].id == sourceInode.id) {
          disk.inodes[i] = sourceInode;
          break;
        }
      }

      char stringValueOfSourceID[4];
      sprintf(stringValueOfSourceID, "%d", sourceInodeID);
      char *fileToMove = malloc((strlen(source) + 4) * sizeof(char));
      strcpy(fileToMove, "<");
      strcat(fileToMove, stringValueOfSourceID);
      strcat(fileToMove, ":");
      strcat(fileToMove, sourceInode.fileName);
      strcat(fileToMove, ">");
      strcat(fileToMove, "\0");

      int curFolderItemsCount;
      char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &curFolderItemsCount);

      free(currentFolderContent);
      currentFolderContent = (char*) malloc(sizeof(char));
      strcpy(currentFolderContent, "");

      for (i = 0; i < curFolderItemsCount; i++) {
        int curItemInodeID;
        sscanf(currentFolderItems[i], "%*c%d", &curItemInodeID);
        if (i > 0) strcat(currentFolderContent, FOLDER_DELIMITER);
        if (curItemInodeID != sourceInode.id) {
          currentFolderContent = (char*) realloc(currentFolderContent, (i+3) * strlen(currentFolderItems[i]) * sizeof(char));
          strcat(currentFolderContent, currentFolderItems[i]);
        }
        else {
          currentFolderContent = (char*) realloc(currentFolderContent, (i+3) * strlen(fileToMove) * sizeof(char));
          strcat(currentFolderContent, fileToMove);
        }
      }

      currentFolderContent = (char*) realloc(currentFolderContent, strlen(currentFolderContent) + 1 * sizeof(char));
      currentFolderContent[strlen(currentFolderContent)] = '\0';

      rewriteFolderContent(&currentFolderInode, currentFolderContent);

      goto saveChanges;
      return;
    }
  }
  char *destinationContent = getFileContent(destinationInode);
  if (fileExists(source, 'd', destinationContent) != -1 || fileExists(source, '-', destinationContent) != -1) {
    nstdError("La cible contient déjà ce fichier.\n");
    return;
  }

  char stringValueOfSourceID[4];
  sprintf(stringValueOfSourceID, "%d", sourceInodeID);
  char *fileToMove = malloc((strlen(source) + 4) * sizeof(char));
  strcpy(fileToMove, "<");
  strcat(fileToMove, stringValueOfSourceID);
  strcat(fileToMove, ":");
  strcat(fileToMove, sourceInode.fileName);
  strcat(fileToMove, ">");
  strcat(fileToMove, "\0");

  int curFolderItemsCount;
  char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &curFolderItemsCount);

  free(currentFolderContent);
  currentFolderContent = (char*) malloc(sizeof(char));
  strcpy(currentFolderContent, "");

  for (i = 0; i < curFolderItemsCount; i++) {
    int curItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &curItemInodeID);
    if (curItemInodeID != sourceInode.id) {
      printf("i: %d\n", i);
      if (i > 0) strcat(currentFolderContent, FOLDER_DELIMITER);
      currentFolderContent = (char*) realloc(currentFolderContent, (i+3) * strlen(currentFolderItems[i]) * sizeof(char));
      strcat(currentFolderContent, currentFolderItems[i]);
    }
  }

  currentFolderContent = (char*) realloc(currentFolderContent, strlen(currentFolderContent) + 1 * sizeof(char));
  currentFolderContent[strlen(currentFolderContent)] = '\0';

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  destinationContent = (char*) realloc(destinationContent, (strlen(FOLDER_DELIMITER) + strlen(destinationContent) + strlen(fileToMove) + 1) * sizeof(char));
  strcat(destinationContent, FOLDER_DELIMITER);
  strcat(destinationContent, fileToMove);

  changeDirectory(destination);
  rewriteFolderContent(&destinationInode, destinationContent);
  changeDirectory("..");

  saveChanges: 
    for (i = 0; i < INODES_COUNT; i++) {
      if (disk.inodes[i].id == currentFolderInode.id) disk.inodes[i] = currentFolderInode;
      else if (disk.inodes[i].id == destinationInode.id) disk.inodes[i] = destinationInode;
    }

    saveDisk();
}


void copy(char *source, char *destination) {
  int i;
  char *sourceFileContent;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int sourceInodeID, destinationInodeID;
  inode_t sourceInode, destinationInode;

  // Check paths
  if (countOcc(source, '/') > 1 || countOcc(destination, '/') > 1) {
    nstdError("Erreur lors de la copie.\n");
    return;
  }
  
  if (strstr(source, "/") != NULL) {
    // Remove '/'
    source[strlen(source) - 1] = '\0';
  }

  if (strstr(destination, "/") !=  NULL) {
    // Remove '/'
    destination[strlen(destination) - 1] = '\0';
  }

  // Check if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1) {
    nstdError("Ce fichier n'existe pas.\n");
    return;
  }
  
  // Get the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);
  // Source file content
  sourceFileContent = getFileContent(sourceInode);

  
  // Check if the source folder exists
  if ((destinationInodeID = fileExists(destination, '-', currentFolderContent)) == -1 && (destinationInodeID = fileExists(destination, 'd', currentFolderContent)) == -1) {
    if (sourceInode.rights[0] == 'd') destinationInode = createFile(destination, 'd');
    else if (sourceInode.rights[0] == '-') destinationInode = createFile(destination, '-');
  }
  else {
    destinationInode = getInodeByID(destinationInodeID);
  }

  if (destinationInode.rights[0] == 'd') {
    int usedBlocks = 0;
    countFolderBlocks(sourceInode, &usedBlocks);

    if (usedBlocks <= availableBlocks) {
      changeDirectory(destination);
      copyFile(sourceInode, sourceFileContent);
      changeDirectory("..");
    }
    else {
      nstdError("Impossible de copier le dossier et son contenu. Il n' a plus de place sur le disque.\n");
      return;
    }
    
  }
  else if (destinationInode.rights[0] == '-') {
    if (sourceInode.rights[0] == '-') {
      nstdError("Impossible de copier un répertoire dans un fichier.\n");
      return;
    }
    file_t copy = openFile(destination, W);
    int contentLength = strlen(sourceFileContent);
    writeFile(copy, sourceFileContent, contentLength);
    closeFile(copy);
  }

  // Update inodes
  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == sourceInode.id) disk.inodes[i] = sourceInode;
    else if (disk.inodes[i].id == destinationInode.id) disk.inodes[i] = destinationInode;
  }

  saveDisk();
}

void copyFile(inode_t fileInode, char *content) {
  int i;
  
  inode_t fileCopy = createFile(fileInode.fileName, fileInode.rights[0]);

  if (fileCopy.rights[0] == 'd') {
    int folderItemsCount;
    char ** folderItems = splitStr(content, FOLDER_DELIMITER, &folderItemsCount);
    changeDirectory(fileCopy.fileName);
    for (i = 1; i < folderItemsCount; i++) {
      int curItemInodeID;
      sscanf(folderItems[i], "%*c%d", &curItemInodeID);
      inode_t curItemInode = getInodeByID(curItemInodeID);
      char *subContent = getFileContent(curItemInode);
      copyFile(curItemInode, subContent);
    }

    changeDirectory("..");
  }
  else if (fileCopy.rights[0] == '-') {
    file_t openedCopy = openFile(fileCopy.fileName, W);
    writeFile(openedCopy, content, strlen(content));
    closeFile(openedCopy);
  }
}

int changeDirectory(char *newDir) {
  int newDirInodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

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
 * Get the content of the given file.
 * For ordinary files, it returns their full content,
 * whithout depending on a buffer size.
 * @return the content of the given file
 **/
char *getFileContent(inode_t fileInode) {
  int usedBlocksCount = 0;
  int i;
  char * fileContent;

  i = 0;
  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
    usedBlocksCount++;
    i++;
  }

  fileContent = (char*) malloc(sizeof(char));
  strcpy(fileContent, "");
  for (i = 0; i < usedBlocksCount; i++) {
    fileContent = realloc(fileContent, (i+1) * BLOCK_SIZE * sizeof(char));
    strcat(fileContent, disk.blocks[fileInode.usedBlocks[i]]);
  }

  return fileContent;
}

void removeFile(char *fileName) {
  inode_t fileInode;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int i;

  int fileInodeID = fileExists(fileName, '-', currentFolderContent);
  if (fileInodeID == -1) {
    nstdError("Le fichier %s n'existe pas.\n", fileName);
    return;
  }

  fileInode = getInodeByID(fileInodeID);

  i = 0;
  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
    strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
    fileInode.usedBlocks[i] = -1;
    availableBlocks++;
    i++;
  }

  int currentFolderItemsCount;
  char ** currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &currentFolderItemsCount);

  strcpy(currentFolderContent, "");
  for (i = 0; i < currentFolderItemsCount; i++) {
    int currentItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &currentItemInodeID);
    if (currentItemInodeID != fileInodeID) {
      if (i > 0) strcat(currentFolderContent, FOLDER_DELIMITER);
      strcat(currentFolderContent, currentFolderItems[i]);
    }
  }

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  strcpy(fileInode.fileName, "");
  strcpy(fileInode.rights, "");

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == currentFolderInode.id) {
      disk.inodes[i] = currentFolderInode;
    }
    else if (disk.inodes[i].id == fileInodeID) {
      disk.inodes[i] = fileInode;
      availableInodes++;
    }
  }

  saveDisk();
}

void countFolderBlocks(inode_t folderInode, int *blocksNumber) {
  int i;
  int folderItemsCount;
  char *folderContent = getFileContent(folderInode);
  char **folderItems = splitStr(folderContent, FOLDER_DELIMITER, &folderItemsCount);
  
  i = 0;
  while (folderInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT) {
    (*blocksNumber)++;
    i++;
  }

  for (i = 1; i < folderItemsCount; i++) {
    int curItemInodeID;
    sscanf(folderItems[i], "%*c%d", &curItemInodeID);
    inode_t curItemInode = getInodeByID(curItemInodeID);
    countFolderBlocks(curItemInode, blocksNumber);
  }
}

// Displays disk's information
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


/**
 * Links a file to another one (= symbolic link)
 * @param file1 is the target file
 * @param file2 is the link name
 * @return -1 for fail and 0 for success
 **/
int link(char *file1, char *file2) {
  int i, file1InodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);
  if ((file1InodeID = fileExists(file1, 'd', currentFolderContent)) == -1 && (file1InodeID = fileExists(file1, '-', currentFolderContent)) == -1) {
    nstdError("Le fichier %s n'existe pas.\n", file1);
    return -1;
  }

  inode_t file1Inode = getInodeByID(file1InodeID);
  inode_t file2Inode = createFile(file2, 'l');

  strcpy(file2Inode.fileName, file2);
  strcpy(file2Inode.rights, "lrw");
  file2Inode.usedBlocks = file1Inode.usedBlocks;

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == file2Inode.id) {
      disk.inodes[i] = file2Inode;
      break;
    }
  }

  return 0;
}

/**
 * Deletes a symbolic link
 * @param link the name of the link to delete
 **/
void unlink(char *link) {
  int i, linkInodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

  if ((linkInodeID = fileExists(link, 'l', currentFolderContent)) == -1) {
    nstdError("Le lien symbolique %s n'existe pas.\n");
    return;
  }

  inode_t linkInode = getInodeByID(linkInodeID);
  strcpy(linkInode.fileName, "");
  strcpy(linkInode.rights, "");
  for (i = 0; i < BLOCKS_COUNT; linkInode.usedBlocks[i++] = -1);

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == linkInode.id) {
      disk.inodes[i] = linkInode;
      break;
    }
  }
}

/**
 * Gets a file from a link. A link correspond to a file if they used the same blocks.
 * @param linkInode the inode of the link that corresponds to the search file.
 * @return the founded file.
 **/
inode_t getFileFromLink(inode_t linkInode) {
  int i;

  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].usedBlocks[0] == linkInode.usedBlocks[0]) return disk.inodes[i];
  }
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

void testCount() {
  char *c = getFileContent(currentFolderInode);
  int f = fileExists("abc", 'd', c);
  if (f != -1) {
    inode_t i = getInodeByID(f);
    int b = 0;
    countFolderBlocks(i, &b);
  }
}