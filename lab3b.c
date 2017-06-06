#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

unsigned int BLOCK_SIZE = 1024;
unsigned int INODE_SIZE = 128;

char * csv;
int csvFileSize;

int numBlocks, numInodes;
int reservedBlockMax;

char * blockStatus;
char * inodeStatus;

int fd;



//Free memory for all allocated blocks
void freeMemory()
{
  if (csv != NULL)
    free(csv);
  if (blockStatus != NULL)
    free(blockStatus);
  if (inodeStatus != NULL)
    free(inodeStatus);
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
    else if (num == 4) BLOCK_SIZE = atoi(tok);
    else if (num == 5) INODE_SIZE = atoi(tok);
  }
}

int isValidBlockNum(int block) {
    if(block < numBlocks && block > reservedBlockMax)
        return 1;
    else return 0;
}

int isValidInodeNum(int inum) {
  if(num <= numInodes && num > 0)
    return 1;
  else return 0;
}

void getBlockFreeListInfo(char* str) {
    char* tok = strtok(str, ",");
  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
    if (num == 2)
      {
	int blockNum = atoi(tok);
	if(isValidBlockNum(blockNum))
	  blockStatus[blockNum] = 'F';
	else
	  printf("INVALID BLOCK %d IN FREE LIST\n", blockNum);
		
      }
  }
}

void getInodeFreeListInfo(char* str) {
    char* tok = strtok(str, ",");
  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
      if (num == 2) {
          int inodeNum = atoi(tok);
          if(isValidInodeNum(inodeNum))
              inodeStatus[inodeNum] = 'F';
          else
              printf("INVALID INODE %d IN FREE LIST\n", inodeNum);
        }
  }
}


void checkBlockStatus(int blockNum) {
    // Mark the block as allocated in our dictionary unless it is supposed to be free
    if (blockStatus[blockNum] == 'F')
      fprintf(stdout, "ALLOCATED BLOCK %d ON FREELIST\n", blockNum);
    else
        blockStatus[blockNum] = 'A';
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
    checkBlockStatus(blockNum);
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

  checkBlockStatus(referencedBlockNum);

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
        if (blockStatus[i] != 'F' && blockStatus[i] != 'A')
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
  csv = NULL, blockStatus = NULL, inodeStatus = NULL;

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
	  blockStatus = (char*) malloc(sizeof(char) * (numBlocks-1));
          
        int inodesPerBlock = BLOCK_SIZE/INODE_SIZE;
        int inodeTableBlocks = numInodes / inodesPerBlock;
	printf("%u\n", inodeTableBlocks);
	reservedBlockMax = 4 + inodeTableBlocks;
    
	inodeStatus = (char*) malloc(sizeof(char) * numInodes);

      }
      else if (strcmp(pch, "BFREE") == 0) {
        getBlockFreeListInfo(line);
      }
      else if (strcmp(pch, "IFREE") == 0) {
        // getInodeFreeListInfo(line);
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
  //     printf("%d:%c\n", i, blockStatus[i]);
  // }

  testUnreferencedBlocks();

  // Free memory
  freeMemory();

  return 0;
}
