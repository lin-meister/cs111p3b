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

struct allocatedBlock {
    unsigned int blockNum;
    unsigned int parentInode;
    unsigned int offset;
    unsigned int level;
    unsigned int isDuplicated;
};

int * freeBlocks;
struct allocatedBlock * allocatedBlocks;

char * inodeStatus;

int fd;

//Free memory for all allocated blocks
void freeMemory()
{
  if (csv != NULL)
    free(csv);
  if (freeBlocks != NULL)
    free(freeBlocks);
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
    else if (block <= reservedBlockMax && block > 0)
        return 0; //reserved block
    else return -1; //invalid block
}

int isValidInodeNum(int num) {
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
    int rc = isValidBlockNum(blockNum);
	if (rc == 1)
	  freeBlocks[blockNum] = 1;
	else if (rc == 0)
	  fprintf(stdout, "RESERVED BLOCK %d IN FREE LIST\n", blockNum);
    else
        fprintf(stdout, "INVALID BLOCK %d IN FREE LIST\n", blockNum);

      }
  }
}

char* indirect = "INDIRECT ";
char* doubleIndirect = "DOUBLE INDIRECT ";
char* trippleIndirect = "TRIPPLE INDIRECT ";
char* emptyString = "";

char* getIndirection (unsigned int offset)
{

    switch (offset) {
        case 1:
        case 12: return indirect;
        case 2:
        case 13: return doubleIndirect;
        case 14: return trippleIndirect;
        default: return emptyString;
    }
}

void getInodeInfo(char* str) {
    
    char* tok = strtok(str, ",");

    int num = 1, inodeNum = 0, blockNum = 0, level = 0;
    for (; num < 13; num++) {
        if (num == 2) inodeNum = atoi(tok);
        tok = strtok(NULL, ",");
    }
    
    int offset = 0;
    char * indirect = "";
    
    for (; tok != NULL; tok = strtok(NULL, ","), offset++) {
        indirect = getIndirection(offset);
        blockNum = atoi(tok);
        if (blockNum == 0)
            continue;
        
        switch (offset) {
            case 12: level = 1; break;
            case 13: level = 2; break;
            case 14: level = 3; break;
            default: level = 0; break;
        }
        switch (offset) {
            case 13: offset = 12 + BLOCK_SIZE/4; break;
            case 14: offset = 12 + BLOCK_SIZE/4 + (BLOCK_SIZE/4)*(BLOCK_SIZE/4); break;
            default: break;
        }
        
        int rc = isValidBlockNum(blockNum);
        if (rc == 1)
        {
            if(allocatedBlocks[blockNum].blockNum != 0)
            {
                if(allocatedBlocks[blockNum].isDuplicated == 0)
                {
                    allocatedBlocks[blockNum].isDuplicated = 1;
                    fprintf(stdout, "DUPLICATE %sBLOCK %u IN INODE %u AT OFFSET %u\n", indirect, allocatedBlocks[blockNum].blockNum, allocatedBlocks[blockNum].parentInode, allocatedBlocks[blockNum].offset);
                }
                fprintf(stdout, "DUPLICATE %sBLOCK %u IN INODE %u AT OFFSET %u\n", indirect, blockNum, inodeNum, offset);
            }
            else
            {
                allocatedBlocks[blockNum].blockNum = blockNum;
                allocatedBlocks[blockNum].parentInode = inodeNum;
                allocatedBlocks[blockNum].level = level;
                allocatedBlocks[blockNum].offset = offset;
            }
        }
        else if(rc == 0)
            fprintf(stdout, "RESERVED %sBLOCK %d IN INODE %d AT OFFSET %d\n", indirect, blockNum, inodeNum, offset);
        else
            fprintf(stdout, "INVALID %sBLOCK %d IN INODE %d AT OFFSET %d\n", indirect, blockNum, inodeNum, offset);
    }
    
}

void getIndirectBlock(char *str) {
    char * tok = strtok(str, ",");

    int num = 1, inodeNum = 0, offset = 0, referencedBlockNum = 0, level = 0;
    for (; tok != NULL; tok = strtok(NULL, ","), num++) {
        if (num == 2) inodeNum = atoi(tok);
        else if (num == 3) level = atoi(tok);
        else if (num == 4) offset = atoi(tok);
        else if (num == 6) referencedBlockNum = atoi(tok);
    }
    
    char * indirect = getIndirection(level - 1);
    int rc = isValidBlockNum(referencedBlockNum);
    if (rc == 1)
    {
        if(allocatedBlocks[referencedBlockNum].blockNum != 0)
        {
            if(allocatedBlocks[referencedBlockNum].isDuplicated == 0)
            {
                allocatedBlocks[referencedBlockNum].isDuplicated = 1;
                char* thisIndirect = getIndirection(allocatedBlocks[referencedBlockNum].level);
                fprintf(stdout, "DUPLICATE %sBLOCK %u IN INODE %u AT OFFSET %u\n", thisIndirect, allocatedBlocks[referencedBlockNum].blockNum, allocatedBlocks[referencedBlockNum].parentInode, allocatedBlocks[referencedBlockNum].offset);
            }
            fprintf(stdout, "DUPLICATE %sBLOCK %u IN INODE %u AT OFFSET %u\n", indirect, referencedBlockNum, inodeNum, offset);
        }
        else
        {
            allocatedBlocks[referencedBlockNum].blockNum = referencedBlockNum;
            allocatedBlocks[referencedBlockNum].parentInode = inodeNum;
            allocatedBlocks[referencedBlockNum].offset = offset;
            allocatedBlocks[referencedBlockNum].level = level - 1;
        }
    }
    else if(rc == 0)
        fprintf(stdout, "RESERVED %sBLOCK %d IN INODE %d AT OFFSET %d\n", indirect, referencedBlockNum, inodeNum, offset);
    else
        fprintf(stdout, "INVALID %sBLOCK %d IN INODE %d AT OFFSET %d\n", indirect, referencedBlockNum, inodeNum, offset);
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


/*void checkBlockStatus(int blockNum) {
    // Mark the block as allocated in our dictionary unless it is supposed to be free
    if (freeBlocks[blockNum] == 1)
      fprintf(stdout, "ALLOCATED BLOCK %d ON FREELIST\n", blockNum);
    else
        blockStatus[blockNum] = 'A';
}
*/

void testDirectBlocksInInode(char* str) {
    char * tok = strtok(str, ",");

  }

void testUnreferencedBlocks() {
    int i;
    for (i = 0; i < numBlocks; i++) {
        if (freeBlocks[i] == 0 && allocatedBlocks[i].blockNum == 0 && i > reservedBlockMax)
            fprintf(stdout, "UNREFERENCED BLOCK %d\n", i);
    }
}

void testAllocatedBlocksConsistency() {
    int i;
    for (i = 0; i < numBlocks; i++) {
        if(allocatedBlocks[i].blockNum != 0 && freeBlocks[i] == 1)
            fprintf(stdout, "ALLOCATED BLOCK %d ON FREELIST\n", i);
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
  csv = NULL, allocatedBlocks = NULL, freeBlocks = NULL, inodeStatus = NULL;

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
        {
            freeBlocks = (int*) malloc(sizeof(int) * (numBlocks));
            memset(freeBlocks, 0, sizeof(int) * numBlocks);
            allocatedBlocks = (struct allocatedBlock *) malloc(sizeof(struct allocatedBlock) * numBlocks);
            memset(allocatedBlocks, 0, sizeof(struct allocatedBlock) * numBlocks);
        }
          
        int inodesPerBlock = BLOCK_SIZE/INODE_SIZE;
        int inodeTableBlocks = numInodes / inodesPerBlock;
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
        getInodeInfo(line);
      }
      else if (strcmp(pch, "INDIRECT") == 0) {
        getIndirectBlock(line);
      }
      free(temp);
  }

  // printf("%d,%d\n", numBlocks, numInodes);
  // int i;
  // for (i = 0; i < numBlocks; i++) {
  //     printf("%d:%c\n", i, blockStatus[i]);
  // }

    testUnreferencedBlocks();
    testAllocatedBlocksConsistency();

  // Free memory
  freeMemory();

  return 0;
}
