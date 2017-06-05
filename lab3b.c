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

int * freeBlocks;

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
  char temp[strlen(str)];
  strcpy(temp, str);
  char * tok = strtok(temp, ",");
  tok[strlen(tok)+1] = '\0';

  int num = 1;
  while (tok != NULL) {
    if (num == 2) numBlocks = atoi(tok);
    else if (num == 3) numInodes = atoi(tok);
    num++;
    tok = strtok(NULL, ",");
  }
}

void getBlockBitmapInfo(char* str) {
    printf("Hello\n");
  char temp[strlen(str)];
  strcpy(temp, str);
  char * tok = strtok(temp, ",");
  tok[strlen(tok)+1] = '\0';

  freeBlocks = (int*) malloc(numBlocks);
  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
    if (num == 2)
      freeBlocks[atoi(tok)] = 1;
  }
}

void testBlockConsistency(char* str) {
  char temp[strlen(str)];
  strcpy(temp, str);
  char * tok = strtok(temp, ",");
  tok[strlen(tok)+1] = '\0';

  int num = 1, parentInodeNum = 0, blockNum = 0, offset = 0;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
    if (num == 2) parentInodeNum = atoi(tok);
    else if (num == 3) offset = atoi(tok);
    else if (num == 4) blockNum = atoi(tok);
  }

  if (blockNum < 0 || blockNum > numBlocks - 1 || parentInodeNum < 1 || parentInodeNum > numInodes)
    fprintf(stdout, "INVALID BLOCK IN INODE %d AT OFFSET %d\n", parentInodeNum, offset);
}

int
main (int argc, char **argv)
{
  // File system image name is in argv
  if (argc != 2) {
    fprintf(stderr, "USAGE: ./lab3b FILENAME.csv\n");
    exit(1);
  }

  csv = NULL, freeBlocks = NULL;

  //Check to see if we can open provided csv
  char * csvFile = argv[1];
  fd = open(csvFile, O_RDONLY);
  if (fd == -1)
    error("Unable to open csv file");

  // Read the csv
  struct stat st;
  fstat(fd, &st);
  csvFileSize = st.st_size;

  csv = (char*) malloc(csvFileSize);
  if (read(fd, csv, csvFileSize) == -1)
    error("Unable to read csv file");

  char * pch;

  pch = strtok(csv, "\n,");
  while (pch != NULL) {
    printf("%s\n", pch);
    if (strcmp(pch, "SUPERBLOCK") == 0) {
      pch = strtok(NULL, "\n");
      getSuperblockInfo(pch);
    }
    else if (strcmp(pch, "BFREE") == 0) {
      pch = strtok(NULL, "\n");
    //   getBlockBitmapInfo(pch);
    }
    else if (strcmp(pch, "DIRENT") == 0) {
      pch = strtok(NULL, "\n");
    //   testBlockConsistency(pch);
    }
    else
        pch = strtok(NULL, "\n,");
  }

  int * temp = freeBlocks;
  for (; temp != NULL; temp++)
    printf("%d ", *temp);

  // Free memory
  freeMemory();

  return 0;
}
