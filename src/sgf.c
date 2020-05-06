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

void initDiskContent() {
  int i;

  // Create the disk's file
  diskFile = fopen("disk0", "a+b");
  
  // Create main inode
  inode_t superInode;
  superInode.id = 0;
  strcpy(superInode.fileName, MAIN_FOLDER);
  strcat(superInode.fileName, "\0");
  strcpy(superInode.rights, "drw-rw-rw-");

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
  // Close file
  fclose(diskFile);
}


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
  char * availableInodeID;


  /* création d'un dossier:
  - vérifier qu'un dossier avec ce nom n'existe pas déjà: FAIT
  - nouvel inode (donc regarder les inodes dispo): FAIT
  - nouveau block de stocakge: FAIT
  - ajout dans le dernier bloc du dossier courrant: FAIT
  - initialiser l'inode pour le renvoyer: FAIT
  - changer l'inode dans le tableau: FAIT
  */
  
  if (strlen(name) > 100) {
    nstdError("Veuillez choisir un nom de moins de 100 caractères.\n");
    return newFileInode;
  }

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

  // Search for free inodes
  for (i = 0; i < INODES_COUNT; i++) {
    if (strcmp(disk.inodes[i].fileName, "") == 0) {
      newFileInode = disk.inodes[i];
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

  if (usedBlocksCount != 0) {
    // Put folder content in an array
    currentFolderContent = malloc(0);
    for (i = 0; i < usedBlocksCount; i++) {
      currentFolderContent = realloc(currentFolderContent, (i+1) * BLOCK_SIZE * sizeof(char));
      strcat(currentFolderContent, disk.blocks[i]);
    }

    // Check if file exists
    int newFileExists = fileExists(name, fileType, currentFolderContent);
    if (newFileExists) {
      if (fileType == 'd') nstdError("Un répertoire avec ce nom existe déjà.\n");
      if (fileType == '-') nstdError("Un fichier avec ce nom existe déjà.\n");
    }

    int lastBlock = usedBlocks[usedBlocksCount-1];
    int lbContentSize = strlen(disk.blocks[lastBlock]) * sizeof(char);
    int availableSpace = BLOCK_SIZE - lbContentSize;

    if ((lbContentSize < BLOCK_SIZE) && availableSpace >= ((strlen(newFile) + 3) * sizeof(char))) {
      strcat(disk.blocks[lastBlock], "||");
      strcat(disk.blocks[lastBlock], newFile);

      for (i = 0; i < BLOCKS_COUNT; newFileInode.usedBlocks[i++] = -1);
      newFileInode.rights[0] = fileType;
      strcpy(&newFileInode.rights[1], "rw-rw-rw-"); 
      //newFileInode.fileName = name;
      strcpy(newFileInode.fileName, name);
      strcat(newFileInode.fileName, "\0");
    }
    else {
      goto useNewBlock;
    }
  }
  else {
    goto useNewBlock;
  }

  useNewBlock: 
    /* availableBlocks must be at least 2,
    ne to save new folder name and one to save new folder content */
    if (availableBlocks >= 2) {
      for (i = 0; i < BLOCKS_COUNT; i++) {
        if (strcmp(disk.blocks[i], "") == 0) {
          usedBlocksCount++;
          // Modifier les blocs pour l'inode
          strcpy(disk.blocks[i], newFile);

          for (j = 0; j < BLOCKS_COUNT; newFileInode.usedBlocks[j++] = -1);
          newFileInode.rights[0] = fileType;
          strcpy(&newFileInode.rights[1], "rw-rw-rw-");
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


  for (i = 0; i < INODES_COUNT; i++) {
    if (disk.inodes[i].id == currentFolderInode.id) disk.inodes[i] = currentFolderInode;
    if (disk.inodes[i].id == newFileInode.id) disk.inodes[i] = newFileInode;
  }

  return newFileInode;
}

//to-do: add date save
void saveDisk() { 
  int i;

  diskFile = fopen("disk0", "wb");
  for (i = 0; i < INODES_COUNT; i++) fwrite(&disk.inodes[i], sizeof(inode_t), 1, diskFile);
  for (i = 0; i < BLOCKS_COUNT; i++) fwrite(&disk.blocks[i], sizeof(block_t), 1, diskFile);
  fclose(diskFile);    
}


void nstdError(const char *format, ...) {
    va_list args;
    
    fprintf(stderr, "Erreur: ");
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

int fileExists(char * fileName, char fileType, char * folderContent) {
  int i, j;
  if (strstr(folderContent, fileName) != NULL) {
    int folderItemsCount;
    char ** folderItems = splitStr(folderContent, "||", &folderItemsCount);
  
    for (i = 0; i < folderItemsCount; i++) {
      if (strstr(folderItems[i], fileName) != NULL) {
        int currentItemInodeID;

        sscanf(folderItems[i], "%*c %d", &currentItemInodeID);

        for (j = 0; j < INODES_COUNT; j++) {
          if (disk.inodes[i].id == currentItemInodeID) {
            if (disk.inodes[i].rights[0] == fileType) {
              return 1;
            }
          }
        }
      }
    }
  }
  return 0;
}
