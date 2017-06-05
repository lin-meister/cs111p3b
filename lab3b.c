#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define BLOCK_SIZE 1024

char * csv;
int csvFileSize;

int numBlocks, numInodes;

char * freeBlocks;

int fd;

//Free memory for all allocated blocks
void freeMemory()
{
  if (csv != NULL)
    free(csv);
  if (freeBlocks != NULL)
    free(freeBlocks);
  //  free(superBlock);
  //  free(groupBlock);
  //  free(blockBitmap);
  //  free(inodeBitmap);
}

//Error return function
void error(char* msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(2);
}

void getSuperblockInfo(char* str) {
  char * tok = strtok(str, ",");

  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
    if (num == 2) numBlocks = atoi(tok);
    else if (num == 3) numInodes = atoi(tok);
  }
}

void getBlockFreeListInfo(char* str) {
    char* tok = strtok(str, ",");

  // char temp[strlen(str)];
  // strcpy(temp, str);
  // char * tok = strtok(temp, ",");
  // tok[strlen(tok)+1] = '\0';

  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
    if (num == 2) freeBlocks[atoi(tok)] = 'F';
  }
}

void testDirectBlocksInInode(char* str) {
    char * tok = strtok(str, ",");

  int num = 1, inodeNum = 0, blockNum = 0;
  for (; num < 13; num++) {
      if (num == 2) inodeNum = atoi(tok);
      tok = strtok(NULL, ",");
  }

  int offset = 0;
  for (; tok != NULL; tok = strtok(NULL, ","), offset++) {
    blockNum = atoi(tok);
    // Mark the block as allocated in our dictionary
    freeBlocks[blockNum] = 'A';
    if (blockNum < 0 || blockNum > numBlocks - 1 || inodeNum < 1 || inodeNum > numInodes)
      fprintf(stdout, "INVALID BLOCK %d IN INODE %d AT OFFSET %d\n", blockNum, inodeNum, offset);
  }
}

void testIndirectBlocks(char* str) {
    char * tok = strtok(str, ",");

  int num = 1, inodeNum = 0, offset = 0, referencedBlockNum = 0, level = 0;
  for (; num < 13; num++) {
      if (num == 2) inodeNum = atoi(tok);
      else if (num == 3) level = atoi(tok);
      else if (num == 4) offset = atoi(tok);
      else if (num == 6) referencedBlockNum = atoi(tok);
      tok = strtok(NULL, ",");
  }
  freeBlocks[referencedBlockNum] = 'A';

  char * levelDesc;
  switch (level) {
      case 1: levelDesc = ""; break;
      case 2: levelDesc = "DOUBLE "; break;
      case 3: levelDesc = "TRIPLE "; break;
  }
  if (referencedBlockNum < 0 || referencedBlockNum > numBlocks - 1 || inodeNum < 1 || inodeNum > numInodes)
    fprintf(stdout, "INVALID %s INDIRECT BLOCK %d IN INODE %d AT OFFSET %d\n", levelDesc, referencedBlockNum, inodeNum, offset);
}

void testUnreferencedBlocks() {
    int i;
    for (i = 0; i < numBlocks; i++) {
        if (freeBlocks[i] != 'F' && freeBlocks[i] != 'A')
            fprintf(stdout, "UNREFERENCED BLOCK %d\n", i);
    }
}

int
main (int argc, char **argv)
{
  // File system image name is in argv
  if (argc != 2) {
    fprintf(stderr, "USAGE: ./lab3b FILENAME.csv\n");
    exit(1);
  }

  numBlocks = -1, numInodes = -1;
  csv = NULL, freeBlocks = NULL;

  //Check to see if we can open provided csv
  char * csvFile = argv[1];

  FILE* stream = fopen(csvFile, "r");
  char line[1024];
  while (fgets(line, 1024, stream)) {
      char * temp = strdup(line);
      char * pch = strtok(temp, ",");
      if (strcmp(pch, "SUPERBLOCK") == 0) {
        getSuperblockInfo(line);
        if (numBlocks > -1)
            freeBlocks = (char*) malloc(sizeof(char) * numBlocks);
      }
      else if (strcmp(pch, "BFREE") == 0) {
        getBlockFreeListInfo(line);
      }
      else if (strcmp(pch, "INODE") == 0) {
          // the direct block info is all in the inode rows
        testDirectBlocksInInode(line);
      }
      else if (strcmp(pch, "INDIRECT") == 0) {
        testIndirectBlocks(line);
      }
      free(temp);
  }

  // printf("%d,%d\n", numBlocks, numInodes);
  // int i;
  // for (i = 0; i < numBlocks; i++) {
  //     printf("%d:%c\n", i, freeBlocks[i]);
  // }

  testUnreferencedBlocks();

  // Free memory
  freeMemory();

  return 0;
}
