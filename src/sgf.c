#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "../headers/sgf.h"
#include "../headers/functions.h"

disk_t disk;
inode_t currentFolderInode;
FILE *diskFile = NULL;
char *currentFolder = MAIN_FOLDER;
int availableBlocks = BLOCKS_COUNT;
int availableInodes = INODES_COUNT;

// Inits the disk structure
void initDisk()
{
  int i;
  int index = -1;
  int fileSize = -1;

  diskFile = fopen("disk0", "rb");
  if (diskFile != NULL)
  {
    index = fseek(diskFile, 0, SEEK_END);
    if (index == 0)
      fileSize = ftell(diskFile);
    if (fileSize == -1)
    {
      fclose(diskFile);
      initDiskContent();
    }
  }
  else
  {
    initDiskContent();
  }

  diskFile = fopen("disk0", "rb");
  // Puts the file's index at the beginning
  fseek(diskFile, 0, SEEK_SET);
  // Gets inodes
  for (i = 0; i < INODES_COUNT; i++)
  {
    fread(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
    currentFolderInode = disk.inodes[0];
    if (strcmp(disk.inodes[i].fileName, "") != 0)
      availableInodes--;
  }
  // Gets blocks
  for (i = 0; i < BLOCKS_COUNT; i++)
  {
    fread(&disk.blocks[i], sizeof(block_t), 1, diskFile);
    if (strcmp(disk.blocks[i], "") != 0)
      availableBlocks--;
  }

  // Closes file
  fclose(diskFile);
}

// Inits disk content with empty inodes & blocks
void initDiskContent()
{
  int i;

  // Creates the disk's file
  diskFile = fopen("disk0", "a+b");

  // Create main inode
  inode_t superInode;
  superInode.id = 0;
  strcpy(superInode.fileName, MAIN_FOLDER);
  strcat(superInode.fileName, "\0");
  strcpy(superInode.rights, "drw");

  // Inits modification date
  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  superInode.lastModificationDate[0] = currentTime.tm_mday;
  superInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  superInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  for (i = 0; i < BLOCKS_COUNT; i++)
    superInode.usedBlocks[i] = -1;

  fwrite(&superInode, sizeof(inode_t), 1, diskFile);

  // Creates inodes to init disk content
  inode_t inode;
  strcpy(inode.fileName, "");
  strcat(inode.fileName, "\0");
  strcpy(inode.rights, "");

  inode.lastModificationDate[0] = currentTime.tm_mday;
  inode.lastModificationDate[1] = currentTime.tm_mon + 1;
  inode.lastModificationDate[2] = currentTime.tm_year + 1900;

  for (i = 0; i < BLOCKS_COUNT; i++)
    inode.usedBlocks[i] = -1;
  for (i = 1; i < INODES_COUNT; i++)
  {
    inode.id = i;
    fwrite(&inode, sizeof(inode_t), 1, diskFile);
  }

  // Create blocks to init disk content
  block_t block;
  strcpy(block, "");

  for (i = 0; i < BLOCKS_COUNT; i++)
    fwrite(&block, sizeof(block_t), 1, diskFile);

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
inode_t createFile(char *name, char fileType)
{
  inode_t newFileInode;
  int i, j;
  // Current folder content
  char *currentFolderContent;
  // Used blocks count for current inode
  int usedBlocksCount = 0;
  // New folder
  char *newFile;
  // String for available inode id
  char availableInodeID[4];

  // Checks file name length
  if (strlen(name) > 100)
  {
    nstdError("Veuillez choisir un nom de moins de 100 caractères.\n");
    return newFileInode;
  }

  i = 0;
  // Counts used blocks for the current directory
  while (currentFolderInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    usedBlocksCount++;
    i++;
  }

  // Checks if there are available inodes & blocks
  if (availableInodes == 0 || availableBlocks == 0)
  {
    nstdError("Veuillez supprimer des fichiers pour libérer de l'espace de stockage\n");
    return newFileInode;
  }

  // Searches for free inode
  for (i = 0; i < INODES_COUNT; i++)
  {
    // If an inode has empty file name, it means it's free
    if (strcmp(disk.inodes[i].fileName, "") == 0)
    {
      newFileInode = disk.inodes[i];
      // Gets string value of inode id
      sprintf(availableInodeID, "%d", newFileInode.id);
      break;
    }
  }

  // Strlen of folder name + strlen of availableInodeID + 4 chars for delimiters and \0
  newFile = (char *)malloc((strlen(name) + strlen(availableInodeID) + 4) * sizeof(char));
  strcpy(newFile, "<");
  strcat(newFile, availableInodeID);
  strcat(newFile, ":");
  strcat(newFile, name);
  strcat(newFile, ">");

  // If some blocks are used for the current folder
  if (usedBlocksCount != 0)
  {
    currentFolderContent = getFileContent(currentFolderInode);

    // Check if file with same name and same type exists
    if (fileExists(name, '-', currentFolderContent) != -1 || fileExists(name, 'd', currentFolderContent) != -1 || fileExists(name, 'l', currentFolderContent) != -1)
    {
      nstdError("Le fichier \"%s\" existe déjà.\n", name);
      return newFileInode;
    }

    // Gets last used block
    int lastBlock = currentFolderInode.usedBlocks[usedBlocksCount - 1];
    // Computes the remaining space in the last used block
    int availableSpace = getRemainingSpace(disk.blocks[lastBlock]);

    if ((strlen(disk.blocks[lastBlock]) < BLOCK_SIZE) && availableSpace >= ((strlen(newFile) + strlen(FOLDER_DELIMITER) + 1)))
    {
      strcat(disk.blocks[lastBlock], FOLDER_DELIMITER);
      strcat(disk.blocks[lastBlock], newFile);

      // Sets blocks of the new inode to -1
      for (i = 0; i < BLOCKS_COUNT; newFileInode.usedBlocks[i++] = -1)
        ;
      newFileInode.rights[0] = fileType;
      strcpy(&newFileInode.rights[1], "rw");
      strcpy(newFileInode.fileName, name);
      strcat(newFileInode.fileName, "\0");

      goto addParentToNewFolder;
    }
    else
    {
      goto getNewBlock;
    }
  }
  else
  {
    goto getNewBlock;
  }

getNewBlock:
  for (i = 0; i < BLOCKS_COUNT; i++)
  {
    if (strcmp(disk.blocks[i], "") == 0)
    {
      usedBlocksCount++;
      strcpy(disk.blocks[i], newFile);

      for (j = 0; j < BLOCKS_COUNT; newFileInode.usedBlocks[j++] = -1)
        ;
      newFileInode.rights[0] = fileType;
      strcpy(&newFileInode.rights[1], "rw");
      strcpy(newFileInode.fileName, name);
      strcat(newFileInode.fileName, "\0");

      currentFolderInode.usedBlocks[usedBlocksCount - 1] = i;
      break;
    }
  }

addParentToNewFolder:
  /* If the new file is a directory, it's necessary to add
    the parent folder path to it, to allow "cd .." execution */
  if (fileType == 'd')
  {
    char currentFolderInodeID[4];
    sprintf(currentFolderInodeID, "%d", currentFolderInode.id);
    char *newFolderContent = (char *)malloc((5 + strlen(currentFolderInodeID) + strlen(currentFolderInodeID)) * sizeof(char));
    strcpy(newFolderContent, "<");
    strcat(newFolderContent, currentFolderInodeID);
    strcat(newFolderContent, ":..>");

    int availableBlock;

    for (i = 0; i < BLOCKS_COUNT; i++)
    {
      if (strcmp(disk.blocks[i], "") == 0)
      {
        availableBlock = i;
        break;
      }
    }

    strcpy(disk.blocks[availableBlock], newFolderContent);
    newFileInode.usedBlocks[0] = availableBlock;
  }

  // Inits the last modification date
  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);

  currentFolderInode.lastModificationDate[0] = currentTime.tm_mday;
  currentFolderInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  currentFolderInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  newFileInode.lastModificationDate[0] = currentTime.tm_mday;
  newFileInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  newFileInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  // Updates inodes array
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == currentFolderInode.id)
      disk.inodes[i] = currentFolderInode;
    if (disk.inodes[i].id == newFileInode.id)
      disk.inodes[i] = newFileInode;
  }

  saveDisk();
  return newFileInode;
}

// Saves changes
void saveDisk()
{
  int i;

  diskFile = fopen("disk0", "wb");
  for (i = 0; i < INODES_COUNT; i++)
    fwrite(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
  for (i = 0; i < BLOCKS_COUNT; i++)
    fwrite(&disk.blocks[i], sizeof(block_t), 1, diskFile);
  fclose(diskFile);
}

/**
 * Check if a file exists in the current folder
 * @param fileName the name of the file to look for
 * @param fileType the type of the file (ordinary or directory)
 * @param folderContent the string which contains the pair <inode:file name>
 * for all files of the current folder
 * @return the inode id of the file if it exists, otherwise -1
 */
int fileExists(char *fileName, char fileType, char *folderContent)
{
  int i, j;
  // Checks if fileName is a substring of folderContent
  if (strstr(folderContent, fileName) != NULL)
  {
    int folderItemsCount;

    // Splits folderContent to get an array which contains all pairs <inode:file name>
    char **folderItems = splitStr(folderContent, FOLDER_DELIMITER, &folderItemsCount);
    for (i = 0; i < folderItemsCount; i++)
    {
      // Checks if the current element of folderItems contains fileName as a substring
      if (strstr(folderItems[i], fileName) != NULL)
      {
        int currentItemInodeID;
        // Gets the inode id of the current item
        sscanf(folderItems[i], "%*c%d", &currentItemInodeID);
        for (j = 0; j < INODES_COUNT; j++)
        {
          if (disk.inodes[j].id == currentItemInodeID)
          {
            if (disk.inodes[j].rights[0] == fileType)
            {
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
void removeFolder(char *folderName)
{
  inode_t folderInode;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int i;

  int folderInodeID = fileExists(folderName, 'd', currentFolderContent);
  if (folderInodeID == -1)
  {
    nstdError("Le répertoire \"%s\" n'existe pas.\n", folderName);
    return;
  }

  folderInode = getInodeByID(folderInodeID);

  // Number of items in the folder to delete
  int folderItemsCount;
  // Items of the folder to delete
  char **folderItems = splitStr(disk.blocks[folderInode.usedBlocks[0]], FOLDER_DELIMITER, &folderItemsCount);

  // Check if the folder is empty: if it is, the only item should be the link to the parent directory
  if (folderItemsCount > 1)
  {
    nstdError("Le répertoire \"%s\" n'est pas vide\n");
    return;
  }

  // Items count in the current folder
  int currentFolderItemsCount;
  // Items of the current folder
  char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &currentFolderItemsCount);

  // Resets the "current folder content" str
  strcpy(currentFolderContent, "");
  for (i = 0; i < currentFolderItemsCount; i++)
  {
    int currentItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &currentItemInodeID);

    // If the current item is different that the one the delete, it's kept in the current folder
    if (currentItemInodeID != folderInodeID)
    {
      strcat(currentFolderContent, currentFolderItems[i]);
      if (i < (currentFolderItemsCount - 1))
        strcat(currentFolderContent, FOLDER_DELIMITER);
    }
  }

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  // Updates the inode of the deleted item
  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  folderInode.lastModificationDate[0] = currentTime.tm_mday;
  folderInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  folderInode.lastModificationDate[2] = currentTime.tm_year + 1900;
  strcpy(folderInode.fileName, "");
  strcpy(disk.blocks[folderInode.usedBlocks[0]], "");
  folderInode.usedBlocks[0] = -1;

  availableBlocks++;

  // Updates inodes array
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == currentFolderInode.id)
    {
      disk.inodes[i] = currentFolderInode;
    }
    else if (disk.inodes[i].id == folderInode.id)
    {
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
void rewriteFolderContent(inode_t *folderInode, char *folderContent)
{
  int i, j, newBlock;
  int usedBlocksCount = 0;
  int bufferPos = 0;
  int bufferSize = strlen(folderContent);
  int necessaryBlocks = (int)ceil((double)bufferSize / BLOCK_SIZE);

  i = 0;
  // Resets all used blocks
  while (folderInode->usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    strcpy(disk.blocks[folderInode->usedBlocks[i]], "");
    folderInode->usedBlocks[i] = -1;
    availableBlocks++;
    i++;
  }

  // Rewrites the content
  for (i = 0; i < necessaryBlocks; i++)
  {
    for (j = 0; j < BLOCKS_COUNT; j++)
    {
      if (strcmp(disk.blocks[j], "") == 0)
      {
        availableBlocks--;
        newBlock = j;
        break;
      }
    }

    j = 0;

    // Writes bytes from the buffer while there is a free byte in the current block
    while (getRemainingSpace(disk.blocks[newBlock]) >= 1 && bufferPos < bufferSize)
    {
      disk.blocks[newBlock][j] = folderContent[bufferPos];
      j++;
      bufferPos++;
    }

    disk.blocks[newBlock][j] = '\0';
    usedBlocksCount++;
    folderInode->usedBlocks[usedBlocksCount - 1] = newBlock;
  }
}

// Lists the files of the current folder
void myls()
{

  int i, j, k;
  char *chaine = (char *)malloc(sizeof(char) * 100);
  strcpy(chaine, "");

  printf(" %s:   ", currentFolderInode.fileName);
  for (i = 0; i < BLOCKS_COUNT; i++)
  {
    // Gets used block
    if (currentFolderInode.usedBlocks[i] != -1)
    {
      j = currentFolderInode.usedBlocks[i];
    }
  }
  //recuperation des fichiers du dossier courant
  for (k = 0; k < BLOCKS_COUNT; k++)
  {
    if (disk.inodes[j].usedBlocks[k] != -1)
    {
      if (strcmp(disk.blocks[disk.inodes[j].usedBlocks[k]], "") != 0)
      {
        strcat(chaine, disk.blocks[disk.inodes[j].usedBlocks[k]]);
      }
    }
  }

  i = 0, j = 1;
  //manips pour afficher seulement le nom du fichier
  int init_size = strlen(chaine);
  char delim[] = "||";
  char *ptr = strtok(chaine, delim);

  while (ptr != NULL)
  {
    if (disk.inodes[j].rights[0] == 'd')
    {
      printf("[d]");
    }
    else
    {
      printf("[f]");
    }
    clean_file_name(ptr);
    printf("%s", ptr);
    printf("    ");
    ptr = strtok(NULL, delim);
    j++;
  }
  printf("\n");
}

/**
 * Opens a file
 * @param fileName the name of the file to open
 * @param mode the mode for the file opening (R, W, RW)
 * @return the file structure that contains
 * the inode ID of the file + the access mode
 **/
file_t openFile(char *fileName, accessMode_e mode)
{
  file_t file;
  inode_t fileInode;
  int fileInodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

  // Checks if there is a symbolic link
  if ((fileInodeID = fileExists(fileName, 'l', currentFolderContent)) != -1)
  {
    inode_t linkInode = getInodeByID(fileInodeID);
    fileInode = getFileFromLink(linkInode);
    file.inodeID = fileInode.id;
  }
  else if ((fileInodeID = fileExists(fileName, '-', currentFolderContent)) != -1)
  {
    file.inodeID = fileInodeID;
  }
  else
  {
    fileInode = createFile(fileName, '-');
    file.inodeID = fileInode.id;
  }
  file.mode = mode;

  return file;
}

/**
 * Closes a file
 * @param file the structure that corresponds to the file to close
 **/
void closeFile(file_t file)
{
  if (file.inodeID != -1)
  {
    file.inodeID = -1;
    file.mode = -1;
  }
}

/**
 * Writes a buffer's content in a file
 * @param file the file to write to
 * @param buffer the content to add to the file
 * @param bufferSize the number of the buffer's bytes
 **/
void writeFile(file_t file, char *buffer, int bufferSize)
{
  inode_t fileInode;
  int i, j, newBlock;
  int bufferPos = 0;
  int usedBlocksCount = 0;
  int necessaryBlocksCount = ceil((double)bufferSize / BLOCK_SIZE);
  int remainingSpace = 0;

  for (i = 0; i < BLOCKS_COUNT; i++)
    remainingSpace += getRemainingSpace(disk.blocks[i]);


  if (remainingSpace < bufferSize)
  {
    nstdError("Il n'y a plus de place sur le disque.\n");
    return;
  }

  // Checks the inode ID and the access mode of the file
  if (file.inodeID == -1 || (file.mode != W && file.mode != A))
    return;
  else
    fileInode = getInodeByID(file.inodeID);

  // If the mode is W, the existing content is rewritten
  if (file.mode == W)
  {
    i = 0;
    while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
    {
      strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
      fileInode.usedBlocks[i] = -1;
      availableBlocks++;
      i++;
    }
    goto writeFileContent;
  }

  i = 0;
  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    usedBlocksCount++;
    i++;
  }

  if (usedBlocksCount > 0)
  {
    // Last block's content
    char lastBlockContent[BLOCK_SIZE];
    strcpy(lastBlockContent, disk.blocks[fileInode.usedBlocks[usedBlocksCount - 1]]);
    // Last block's remaining space
    int lbRemainingSpace = getRemainingSpace(lastBlockContent);

    // If there is enough free bytes in the last bloc
    if (lbRemainingSpace >= bufferSize)
    {
      strcat(lastBlockContent, buffer);
      strcpy(disk.blocks[fileInode.usedBlocks[usedBlocksCount - 1]], lastBlockContent);
      saveDisk();
      return;
    }
    else
    {
      // Checks if there are enough free blocks
      if (availableBlocks >= necessaryBlocksCount)
      {
        // Checks if there are enough free bytes in the last block
        if (lbRemainingSpace > 0)
        {
          for (i = 0; i < lbRemainingSpace; i++)
            lastBlockContent[BLOCK_SIZE - lbRemainingSpace + i] = buffer[i];
    
          bufferPos = i;
          strcpy(disk.blocks[fileInode.usedBlocks[usedBlocksCount - 1]], lastBlockContent);
        }
      }
      else
      {
        nstdError("Il n'y a pas assez de place sur le disque.\n");
        return;
      }
    }
  }

// Writes remaining content in the file
writeFileContent:
  for (i = 0; i < necessaryBlocksCount; i++)
  {
    for (j = 0; j < BLOCKS_COUNT; j++)
    {
      // Get new block
      if (strcmp(disk.blocks[j], "") == 0)
      {
        newBlock = j;
        break;
      }
    }

    j = 0;
    // Writes bytes from the buffer while there is a free byte in the current block
    while (getRemainingSpace(disk.blocks[newBlock]) >= 1 && bufferPos < bufferSize)
    {
      disk.blocks[newBlock][j] = buffer[bufferPos];
      j++;
      bufferPos++;
    }

    usedBlocksCount++;
    availableBlocks--;
    // Adds the new block to fileInode.usedBlocks array
    fileInode.usedBlocks[usedBlocksCount - 1] = newBlock;
  }

  // Updates last modification date
  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  fileInode.lastModificationDate[0] = currentTime.tm_mday;
  fileInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  fileInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  // Updates inodes array
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == fileInode.id)
    {
      disk.inodes[i] = fileInode;
      break;
    }
  }

  saveDisk();
}

/**
 * Read a file and put its content in a buffer
 * @param file the file to read
 * @param buffer the buffer in which the content is put
 * @param bufferSize the number of bytes to put in the buffer
 **/
void readFile(file_t file, char **buffer, int bufferSize)
{
  inode_t fileInode;
  int i = 0;
  int usedBlocksCount = 0;

  if (file.inodeID == -1 || (file.mode != R && file.mode != A))
    return;
  else
    fileInode = getInodeByID(file.inodeID);

  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    usedBlocksCount++;
    i++;
  }

  if (usedBlocksCount == 0)
  {
    *buffer = (char *)malloc(sizeof(char));
    *buffer = "";
    return;
  }

  char *fileContent = (char *)malloc(sizeof(char));
  strcpy(fileContent, "");
  for (i = 0; i < usedBlocksCount; i++)
  {
    fileContent = (char *)realloc(fileContent, ((i + 1) * BLOCK_SIZE) * sizeof(char));
    strcat(fileContent, disk.blocks[fileInode.usedBlocks[i]]);
  }

  if (strlen(fileContent) < bufferSize)
  {
    *buffer = fileContent;
  }
  else
  {
    *buffer = (char *)malloc(bufferSize * sizeof(char));
    for (i = 0; i < bufferSize; i++)
    {
      (*buffer)[i] = fileContent[i];
    }
  }
}

/**
 * Move a file to another one
 * @param source the file to move
 * @param destination the moving destination
 **/
void move(char *source, char *destination)
{
  int i, j;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int sourceInodeID, destinationInodeID;
  inode_t sourceInode, destinationInode;

  // Checks paths
  if (countOcc(source, '/') > 1 || countOcc(destination, '/') > 1)
  {
    nstdError("Erreur lors de la copie.\n");
    return;
  }

  if (strstr(source, "/") != NULL)
  {
    // Remove '/'
    source[strlen(source) - 1] = '\0';
  }

  if (strstr(destination, "/") != NULL)
  {
    // Remove '/'
    destination[strlen(destination) - 1] = '\0';
  }

  // Checks if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'l', currentFolderContent)) == -1)
  {
    nstdError("La cible \"%s\" n'existe pas.\n", source);
    return;
  }

  // Gets the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);

  if (sourceInode.rights[0] == 'd')
  {
    if ((destinationInodeID = fileExists(destination, '-', currentFolderContent)) != -1)
    {
      nstdError("Impossible de déplacer un répertoire dans un fichier.\n");
      return;
    }
    else if ((destinationInodeID = fileExists(destination, 'd', currentFolderContent)) != -1)
    {
      destinationInode = getInodeByID(destinationInodeID);
    }
    else
    {
      destinationInode = createFile(destination, 'd');
    }
  }
  else
  {
    if (sourceInode.rights[0] == '-' && (destinationInodeID = fileExists(destination, '-', currentFolderContent)) != -1)
    {
      char *sourceContent = getFileContent(sourceInode);
      int sourceContentLength = strlen(sourceContent);
      file_t target = openFile(destination, W);
      writeFile(target, sourceContent, sourceContentLength);
      closeFile(target);
      removeFile(source);
      return;
    }
    else if ((destinationInodeID = fileExists(destination, 'd', currentFolderContent)) != -1)
    {
      destinationInode = getInodeByID(destinationInodeID);
    }
    else
    {
      // Renames the file
      strcpy(sourceInode.fileName, destination);

      // Updates last modification date
      time_t cTime = time(NULL);
      struct tm currentTime = *localtime(&cTime);
      sourceInode.lastModificationDate[0] = currentTime.tm_mday;
      sourceInode.lastModificationDate[1] = currentTime.tm_mon + 1;
      sourceInode.lastModificationDate[2] = currentTime.tm_year + 1900;

      currentFolderInode.lastModificationDate[0] = currentTime.tm_mday;
      currentFolderInode.lastModificationDate[1] = currentTime.tm_mon + 1;
      currentFolderInode.lastModificationDate[2] = currentTime.tm_year + 1900;

      // Updates inodes array
      for (i = 0; i < INODES_COUNT; i++)
      {
        if (disk.inodes[i].id == sourceInode.id)
        {
          disk.inodes[i] = sourceInode;
          break;
        }
      }

      /*** Rewrites current folder content ***/
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
      currentFolderContent = (char *)malloc(sizeof(char));
      strcpy(currentFolderContent, "");

      j = 0;
      for (i = 0; i < curFolderItemsCount; i++)
      {
        int curItemInodeID;
        sscanf(currentFolderItems[i], "%*c%d", &curItemInodeID);
        if (i > 0 && j > 1)
          strcat(currentFolderContent, FOLDER_DELIMITER);
        if (curItemInodeID != sourceInode.id)
        {
          currentFolderContent = (char *)realloc(currentFolderContent, (strlen(currentFolderContent) + strlen(FOLDER_DELIMITER) + strlen(currentFolderItems[i])) * sizeof(char));
          strcat(currentFolderContent, currentFolderItems[i]);
        }
        else
        {
          currentFolderContent = (char *)realloc(currentFolderContent, (strlen(currentFolderContent) + strlen(FOLDER_DELIMITER) + strlen(fileToMove)) * sizeof(char));
          strcat(currentFolderContent, fileToMove);
        }
        j++;
      }

      currentFolderContent = (char *)realloc(currentFolderContent, strlen(currentFolderContent) + 1 * sizeof(char));
      currentFolderContent[strlen(currentFolderContent)] = '\0';

      rewriteFolderContent(&currentFolderInode, currentFolderContent);

      goto saveChanges;
      return;
    }
  }

  // Gets destination folder content
  char *destinationContent = getFileContent(destinationInode);
  // Checks if the source file already exists in the destination folder
  if (fileExists(source, 'd', destinationContent) != -1 || fileExists(source, '-', destinationContent) != -1 || fileExists(source, 'l', destinationContent) != -1)
  {
    nstdError("La cible contient déjà ce fichier.\n");
    return;
  }

  // Creates the string for the moved file
  char stringValueOfSourceID[4];
  sprintf(stringValueOfSourceID, "%d", sourceInodeID);
  char *fileToMove = malloc((strlen(source) + 4) * sizeof(char));
  strcpy(fileToMove, "<");
  strcat(fileToMove, stringValueOfSourceID);
  strcat(fileToMove, ":");
  strcat(fileToMove, sourceInode.fileName);
  strcat(fileToMove, ">");
  strcat(fileToMove, "\0");

  // Items count for the current folder
  int curFolderItemsCount;
  // Items of the current folder
  char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &curFolderItemsCount);

  free(currentFolderContent);
  currentFolderContent = (char *)malloc(sizeof(char));
  strcpy(currentFolderContent, "");


  /*** Rewrites the current folder's content without the moved file ***/
  j = 0;
  for (i = 0; i < curFolderItemsCount; i++)
  {
    int curItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &curItemInodeID);
    if (curItemInodeID != sourceInode.id)
    {
      if (i > 0 && j > 1)
        strcat(currentFolderContent, FOLDER_DELIMITER);
      currentFolderContent = (char *)realloc(currentFolderContent, (i + strlen(FOLDER_DELIMITER) + strlen(currentFolderItems[i])) * sizeof(char));
      strcat(currentFolderContent, currentFolderItems[i]);
      j++;
    }
  }

  currentFolderContent = (char *)realloc(currentFolderContent, strlen(currentFolderContent) + 1 * sizeof(char));
  currentFolderContent[strlen(currentFolderContent)] = '\0';

  rewriteFolderContent(&currentFolderInode, currentFolderContent);
  
  // Updates last modification date for the current folder inode
  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  currentFolderInode.lastModificationDate[0] = currentTime.tm_mday;
  currentFolderInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  currentFolderInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  
  // Checks if the source is a directory
  if (sourceInode.rights[0] == 'd') {
    /*** Resets the link between the moved folder and its parent folder ***/

    char strValueOfSrcParentID[4];
    sprintf(strValueOfSrcParentID, "%d", destinationInode.id);

    // Create the string for the link between the moved file and its parent folder
    char *sourceParent = (char *)malloc((strlen(strValueOfSrcParentID) + 6) * sizeof(char));
    strcpy(sourceParent, "<");
    strcat(sourceParent, strValueOfSrcParentID);
    strcat(sourceParent, ":..>");

    // Gets source directory content
    char *sourceContent = getFileContent(sourceInode);
    // Items count for the source folder
    int sourceItemsCount;
    // Items of the source folder
    char **sourceItems = splitStr(sourceContent, FOLDER_DELIMITER, &sourceItemsCount);

    free(sourceContent);
    sourceContent = (char *)malloc(strlen(sourceParent) * sizeof(char));
    // Adds the link to the parent in the source folder content
    strcpy(sourceContent, sourceParent);
    // Re-adds other items
    for (i = 1; i < sourceItemsCount; i++)
    {
      sourceContent = (char *)realloc(sourceContent, (strlen(sourceContent) + strlen(FOLDER_DELIMITER) + strlen(sourceItems[i])) * sizeof(char));
      strcat(sourceContent, sourceItems[i]);
      if (i < (sourceItemsCount - 1))
        strcat(sourceContent, FOLDER_DELIMITER);
    }

    rewriteFolderContent(&sourceInode, sourceContent);
  }

  /*** Update destination content ***/
  destinationContent = (char *)realloc(destinationContent, (strlen(FOLDER_DELIMITER) + strlen(destinationContent) + strlen(fileToMove) + 1) * sizeof(char));
  strcat(destinationContent, FOLDER_DELIMITER);
  strcat(destinationContent, fileToMove);

  changeDirectory(destination);
  rewriteFolderContent(&destinationInode, destinationContent);
  changeDirectory("..");

  // Updates last modification date for the destination folder's inode
  destinationInode.lastModificationDate[0] = currentTime.tm_mday;
  destinationInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  destinationInode.lastModificationDate[2] = currentTime.tm_year + 1900;

saveChanges:
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == currentFolderInode.id)
      disk.inodes[i] = currentFolderInode;
    else if (disk.inodes[i].id == destinationInode.id)
      disk.inodes[i] = destinationInode;
  }

  saveDisk();
}

/**
 * Copy a file
 * @param source the file to copy
 * @param destination the destination in which the file will be copied
 **/
void copy(char *source, char *destination)
{
  int i;
  char *sourceFileContent;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int sourceInodeID, destinationInodeID;
  inode_t sourceInode, destinationInode;

  // Checks paths
  if (countOcc(source, '/') > 1 || countOcc(destination, '/') > 1)
  {
    nstdError("Erreur lors de la copie.\n");
    return;
  }

  if (strstr(source, "/") != NULL)
  {
    // Remove '/'
    source[strlen(source) - 1] = '\0';
  }

  if (strstr(destination, "/") != NULL)
  {
    // Remove '/'
    destination[strlen(destination) - 1] = '\0';
  }

  // Check if the source file exists
  if ((sourceInodeID = fileExists(source, '-', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'd', currentFolderContent)) == -1 && (sourceInodeID = fileExists(source, 'l', currentFolderContent)) == -1)
  {
    nstdError("Le fichier \"%s\" n'existe pas.\n", source);
    return;
  }

  // Gets the source file inode ID
  sourceInode = getInodeByID(sourceInodeID);
  // Source file content
  sourceFileContent = getFileContent(sourceInode);

  // Checks if the source folder exists
  if ((destinationInodeID = fileExists(destination, '-', currentFolderContent)) == -1 && (destinationInodeID = fileExists(destination, 'd', currentFolderContent)) == -1)
  {
    if (sourceInode.rights[0] == 'd')
      destinationInode = createFile(destination, 'd');
    else if (sourceInode.rights[0] == '-')
      destinationInode = createFile(destination, '-');
  }
  else
  {
    destinationInode = getInodeByID(destinationInodeID);
  }

  if (destinationInode.rights[0] == 'd')
  {
    int usedBlocks = 0;
    // Computes the number of blocks used by the source folder
    countFolderBlocks(sourceInode, &usedBlocks);

    if (usedBlocks <= availableBlocks)
    {
      changeDirectory(destination);
      copyFile(sourceInode, sourceFileContent);
      changeDirectory("..");
    }
    else
    {
      nstdError("Impossible de copier le dossier et son contenu. Il n' a plus de place sur le disque.\n");
      return;
    }
  }
  else if (destinationInode.rights[0] == '-')
  {
    if (sourceInode.rights[0] == '-')
    {
      nstdError("Impossible de copier un répertoire dans un fichier.\n");
      return;
    }
    file_t copy = openFile(destination, W);
    int contentLength = strlen(sourceFileContent);
    writeFile(copy, sourceFileContent, contentLength);
    closeFile(copy);
  }

  // Update inodes array
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == sourceInode.id)
      disk.inodes[i] = sourceInode;
    else if (disk.inodes[i].id == destinationInode.id)
      disk.inodes[i] = destinationInode;
  }

  saveDisk();
}

/**
 * Recursive copy of a directory and its files/sub folders
 * @param fileInode the inode of the file to copy
 * @param content the content to copy
 **/
void copyFile(inode_t fileInode, char *content)
{
  int i;

  // Creates the copy of fileInode
  inode_t fileCopy = createFile(fileInode.fileName, fileInode.rights[0]);


  if (fileCopy.rights[0] == 'd')
  {
    int folderItemsCount;
    char **folderItems = splitStr(content, FOLDER_DELIMITER, &folderItemsCount);

    // Goes to the created folder
    changeDirectory(fileCopy.fileName);
    // Copies each items of the original folder
    for (i = 1; i < folderItemsCount; i++)
    {
      int curItemInodeID;
      sscanf(folderItems[i], "%*c%d", &curItemInodeID);
      inode_t curItemInode = getInodeByID(curItemInodeID);
      char *subContent = getFileContent(curItemInode);
      copyFile(curItemInode, subContent);
    }
    // Goes back to the parent
    changeDirectory("..");
  }
  else if (fileCopy.rights[0] == '-')
  {
    // Creates the copy of the file
    file_t openedCopy = openFile(fileCopy.fileName, W);
    writeFile(openedCopy, content, strlen(content));
    closeFile(openedCopy);
  }
}

/**
 * Changes the current directory
 * @param newDir the new directory
 **/
int changeDirectory(char *newDir)
{
  int newDirInodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

  newDirInodeID = fileExists(newDir, 'd', currentFolderContent);

  if (newDirInodeID == -1)
  {
    nstdError("Le répertoire \"%s\" n'existe pas.\n", newDir);
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
int getRemainingSpace(char *content)
{
  return BLOCK_SIZE - strlen(content);
}

/**
 * Gets an inode by its id
 * @param inodeID the id of the inode to get
 * @return the found inode
 **/
inode_t getInodeByID(int inodeID)
{
  inode_t inode;
  int i;
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == inodeID)
    {
      inode = disk.inodes[i];
      break;
    }
  }

  return inode;
}

/**
 * Gets a file name by the file's inode id
 * @param inodeID the id of the file's inode
 * @return the found file name
 **/
char *getFileNameByID(int inodeID)
{
  int i;
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == inodeID)
      return disk.inodes[i].fileName;
  }

  return NULL;
}

/**
 * Gets the content of the given file.
 * For ordinary files, it returns their full content,
 * whithout depending on a buffer size.
 * @return the content of the given file
 **/
char *getFileContent(inode_t fileInode)
{
  int usedBlocksCount = 0;
  int i;
  char *fileContent;

  i = 0;
  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    usedBlocksCount++;
    i++;
  }

  fileContent = (char *)malloc(sizeof(char));
  strcpy(fileContent, "");
  for (i = 0; i < usedBlocksCount; i++)
  {
    fileContent = realloc(fileContent, (i + 1) * BLOCK_SIZE * sizeof(char));
    strcat(fileContent, disk.blocks[fileInode.usedBlocks[i]]);
  }

  return fileContent;
}

/**
 * Removes a file
 * @param fileName the name of the file to delete
 **/
void removeFile(char *fileName)
{
  inode_t fileInode;
  char *currentFolderContent = getFileContent(currentFolderInode);
  int i;

  int fileInodeID = fileExists(fileName, '-', currentFolderContent);
  if (fileInodeID == -1)
  {
    nstdError("Le fichier \"%s\" n'existe pas.\n", fileName);
    return;
  }

  fileInode = getInodeByID(fileInodeID);

  i = 0;
  // Resets the used blocks
  while (fileInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    strcpy(disk.blocks[fileInode.usedBlocks[i]], "");
    fileInode.usedBlocks[i] = -1;
    availableBlocks++;
    i++;
  }

  /*** Rewrites the current folder content without the deleted file ***/
  int currentFolderItemsCount;
  char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &currentFolderItemsCount);

  strcpy(currentFolderContent, "");
  for (i = 0; i < currentFolderItemsCount; i++)
  {
    int currentItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &currentItemInodeID);
    if (currentItemInodeID != fileInodeID)
    {
      if (i > 0)
        strcat(currentFolderContent, FOLDER_DELIMITER);
      strcat(currentFolderContent, currentFolderItems[i]);
    }
  }

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  // Resets the deleted file's inode
  strcpy(fileInode.fileName, "");
  strcpy(fileInode.rights, "");

  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  fileInode.lastModificationDate[0] = currentTime.tm_mday;
  fileInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  fileInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  // Updates inodes array
  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == currentFolderInode.id)
    {
      disk.inodes[i] = currentFolderInode;
    }
    else if (disk.inodes[i].id == fileInodeID)
    {
      disk.inodes[i] = fileInode;
      availableInodes++;
    }
  }

  saveDisk();
}

/**
 * Counts the blocks used by a folder and its files/sub folders
 * @param folderInode the inode of the concerned folder
 * @param blocksNumber the number of used blocks
 **/
void countFolderBlocks(inode_t folderInode, int *blocksNumber)
{
  int i;
  int folderItemsCount;
  char *folderContent = getFileContent(folderInode);
  char **folderItems = splitStr(folderContent, FOLDER_DELIMITER, &folderItemsCount);

  i = 0;
  while (folderInode.usedBlocks[i] != -1 && i < BLOCKS_COUNT)
  {
    (*blocksNumber)++;
    i++;
  }

  // Calls this function for each item of the folder
  for (i = 1; i < folderItemsCount; i++)
  {
    int curItemInodeID;
    sscanf(folderItems[i], "%*c%d", &curItemInodeID);
    inode_t curItemInode = getInodeByID(curItemInodeID);
    countFolderBlocks(curItemInode, blocksNumber);
  }
}

// Displays disk's information
void diskFree()
{
  int remainingSpace = 0;
  int i;

  for (i = 0; i < BLOCKS_COUNT; i++)
  {
    remainingSpace += (BLOCK_SIZE - strlen(disk.blocks[i]));
  }

  printf("Free inodes:\t%d / %d\n", availableInodes, INODES_COUNT);
  printf("Free blocks:\t%d / %d\n", availableBlocks, BLOCKS_COUNT);
  printf("Free bytes:\t%d / %d\n", remainingSpace, BLOCK_SIZE * BLOCKS_COUNT);
}

/**
 * Links a file to another one (= symbolic link)
 * @param file1 is the target file
 * @param file2 is the link name
 * @return -1 for fail and 0 for success
 **/
int linkFile(char *file1, char *file2)
{
  int i, file1InodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

  if ((file1InodeID = fileExists(file1, 'd', currentFolderContent)) == -1 && (file1InodeID = fileExists(file1, '-', currentFolderContent)) == -1)
  {
    nstdError("Le fichier \"%s\" n'existe pas.\n", file1);
    return -1;
  }

  inode_t file1Inode = getInodeByID(file1InodeID);
  inode_t file2Inode = createFile(file2, 'l');

  for (i = 0; i < BLOCKS_COUNT; i++)
    file2Inode.usedBlocks[i] = file1Inode.usedBlocks[i];

  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == file2Inode.id)
    {
      disk.inodes[i] = file2Inode;
      break;
    }
  }

  saveDisk();

  return 0;
}

/**
 * Deletes a symbolic link
 * @param link the name of the link to delete
 **/
void unlinkFile(char *link)
{
  int i, linkInodeID;
  char *currentFolderContent = getFileContent(currentFolderInode);

  if ((linkInodeID = fileExists(link, 'l', currentFolderContent)) == -1)
  {
    nstdError("Le lien symbolique \"%s\" n'existe pas.\n");
    return;
  }

  /*** Resets the link inode ***/
  inode_t linkInode = getInodeByID(linkInodeID);
  strcpy(linkInode.fileName, "");
  strcpy(linkInode.rights, "");

  time_t cTime = time(NULL);
  struct tm currentTime = *localtime(&cTime);
  linkInode.lastModificationDate[0] = currentTime.tm_mday;
  linkInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  linkInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  for (i = 0; i < BLOCKS_COUNT; linkInode.usedBlocks[i++] = -1);

  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].id == linkInode.id)
    {
      disk.inodes[i] = linkInode;
      break;
    }
  }

  /*** Rewrites the current folder content without the link ***/
  int currentFolderItemsCount;
  char **currentFolderItems = splitStr(currentFolderContent, FOLDER_DELIMITER, &currentFolderItemsCount);
  strcpy(currentFolderContent, "");

  for (i = 0; i < currentFolderItemsCount; i++)
  {
    int currentItemInodeID;
    sscanf(currentFolderItems[i], "%*c%d", &currentItemInodeID);
    if (currentItemInodeID != linkInodeID)
    {
      if (i > 0)
        strcat(currentFolderContent, FOLDER_DELIMITER);
      strcat(currentFolderContent, currentFolderItems[i]);
    }
  }

  currentFolderInode.lastModificationDate[0] = currentTime.tm_mday;
  currentFolderInode.lastModificationDate[1] = currentTime.tm_mon + 1;
  currentFolderInode.lastModificationDate[2] = currentTime.tm_year + 1900;

  rewriteFolderContent(&currentFolderInode, currentFolderContent);

  saveDisk();
}

/**
 * Gets a file from a link. A link correspond to a file if they used the same blocks.
 * @param linkInode the inode of the link that corresponds to the search file.
 * @return the founded file.
 **/
inode_t getFileFromLink(inode_t linkInode)
{
  int i;
  inode_t fileInode;
  strcpy(fileInode.fileName, "");
  strcpy(fileInode.rights, "");


  for (i = 0; i < BLOCKS_COUNT; fileInode.usedBlocks[i++] = -1);

  for (i = 0; i < INODES_COUNT; i++)
  {
    if (disk.inodes[i].usedBlocks[0] == linkInode.usedBlocks[0])
    {
      fileInode = disk.inodes[i];
      break;
    }
  }

  return fileInode;
}

// TEST -------
void testContent()
{
  int i, j;

  for (i = 0; i < INODES_COUNT; i++)
  {
    for (j = 0; j < BLOCKS_COUNT; j++)
    {
      if (disk.inodes[i].usedBlocks[j] != -1)
      {
        if (strcmp(disk.blocks[disk.inodes[i].usedBlocks[j]], "") != 0)
        {
          printf("contenu en [%d, %d]: %s\n", i, j, disk.blocks[disk.inodes[i].usedBlocks[j]]);
        }
      }
    }
  }
}
