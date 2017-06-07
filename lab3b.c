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
int firstNonreservedInode;

struct allocatedBlock {
    unsigned int blockNum;
    unsigned int parentInode;
    unsigned int offset;
    unsigned int level;
    unsigned int isDuplicated;
};

struct allocatedInode {
    unsigned int inodeNum;
    unsigned int linkCount;
<<<<<<< HEAD
    unsigned int directoryLinks;
};
=======
    unsigned int directoryLinks;
    unsigned int actualParent;
    unsigned int providedParentFromDotDot;
    char fileType;
};
>>>>>>> 67fc0daeefd477366e45f5f26e7a5d5101d0e5ba

int * freeBlocks;
struct allocatedBlock * allocatedBlocks;
int * freeInodes;
struct allocatedInode * allocatedInodes;

int fd;

//Free memory for all allocated blocks
void freeMemory()
{
  if (csv != NULL)
    free(csv);
  if (freeBlocks != NULL)
    free(freeBlocks);
  if (freeInodes != NULL)
    free(freeInodes);

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
    else if (num == 8) firstNonreservedInode = atoi(tok);
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
  if((num <= numInodes && num >= firstNonreservedInode) || num == 2  )
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

void getInodeFreeListInfo(char* str) {
    char* tok = strtok(str, ",");
  int num = 1;
  for (; tok != NULL; tok = strtok(NULL, ","), num++) {
      if (num == 2) {
          int inodeNum = atoi(tok);
          if(isValidInodeNum(inodeNum))
              freeInodes[inodeNum] = 1;
          else
              fprintf(stdout, "INVALID INODE %d IN FREE LIST\n", inodeNum);
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

    char fileType;
    int num = 1, inodeNum = 0, blockNum = 0, linkCount = 0, level = 0, mode = 0;
    for (; num < 13; num++) {
        if (num == 2) inodeNum = atoi(tok);
        else if(num == 3) fileType = tok[0];
        else if (num == 4) mode = atoi(tok);
        else if (num == 7) linkCount = atoi(tok);
        tok = strtok(NULL, ",");
    }

<<<<<<< HEAD
    if(mode == 0)
        return;

=======

>>>>>>> 67fc0daeefd477366e45f5f26e7a5d5101d0e5ba
    if (isValidInodeNum(inodeNum)) {
        if(mode == 0)
            return;
        allocatedInodes[inodeNum].inodeNum = inodeNum;
        allocatedInodes[inodeNum].linkCount = linkCount;
        allocatedInodes[inodeNum].fileType = fileType;
    }
    else    {
        fprintf(stdout, "INVALID INODE %d\n", inodeNum);
<<<<<<< HEAD

=======
        return;
    }
>>>>>>> 67fc0daeefd477366e45f5f26e7a5d5101d0e5ba
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
                    char* thisIndirect = getIndirection(allocatedBlocks[blockNum].level);
                    fprintf(stdout, "DUPLICATE %sBLOCK %u IN INODE %u AT OFFSET %u\n", thisIndirect, allocatedBlocks[blockNum].blockNum, allocatedBlocks[blockNum].parentInode, allocatedBlocks[blockNum].offset);
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

/*void checkBlockStatus(int blockNum) {
    // Mark the block as allocated in our dictionary unless it is supposed to be free
    if (freeBlocks[blockNum] == 1)
      fprintf(stdout, "ALLOCATED BLOCK %d ON FREELIST\n", blockNum);
    else
        blockStatus[blockNum] = 'A';
}
*/

void getDirectoryEntry(char *str) {
    char * tok = strtok(str, ",");

    int num = 1, inodeNum = 0, directoryInodeNum = 0, nameLength = 0;
    char* name;
    for (; tok != NULL; tok = strtok(NULL, ","), num++) {
        if(num == 2) directoryInodeNum = atoi(tok);
        if(num == 4) inodeNum = atoi(tok);
        if(num == 6) nameLength = atoi(tok);
        if(num == 7) name = strdup(tok);
    }
    name[nameLength+2] = '\0';

    if(isValidInodeNum(inodeNum) == 0) {
        fprintf(stdout, "DIRECTORY INODE %d NAME %s INVALID INODE %d\n", directoryInodeNum, name, inodeNum);
        return;
    }
    if(freeInodes[inodeNum] == 1){
        fprintf(stdout, "DIRECTORY INODE %d NAME %s UNALLOCATED INODE %d\n", directoryInodeNum, name, inodeNum);
        return;
    }

    if(strcmp(name, ".") == 0 && inodeNum == 2)
        allocatedInodes[inodeNum].actualParent = 2;
    else if(strcmp(name, ".") == 0 && directoryInodeNum != inodeNum)
        fprintf(stdout, "DIRECTORY INODE %d NAME %s LINK TO INODE %d SHOULD BE %d\n", directoryInodeNum, name, inodeNum, directoryInodeNum);
    //else if(strcmp(name, "..") == 0 && directoryInodeNum == 2)
        //fprintf(stdout, "DIRECTORY INODE %d NAME %s LINK TO INODE %d SHOULD BE %d\n", directoryInodeNum, name, inodeNum, directoryInodeNum);
    else if(strcmp(name,"..") == 0) {
        printf("reached\n");
        allocatedInodes[directoryInodeNum].providedParentFromDotDot = inodeNum;
    }
    else
        allocatedInodes[inodeNum].actualParent = directoryInodeNum;


    allocatedInodes[inodeNum].directoryLinks++;

>>>>>>> 67fc0daeefd477366e45f5f26e7a5d5101d0e5ba
}

void testDirectBlocksInInode(char* str) {
    char * tok = strtok(str, ",");

  }

void testUnreferencedBlocks() {
    int i;
    for (i = 1; i < numBlocks; i++) {
        if (freeBlocks[i] == 0 && allocatedBlocks[i].blockNum == 0 && i > reservedBlockMax)
            fprintf(stdout, "UNREFERENCED BLOCK %d\n", i);
    }
}

void testAllocatedBlocksConsistency() {
    int i;
    for (i = 1; i < numBlocks; i++) {
        if(allocatedBlocks[i].blockNum != 0 && freeBlocks[i] == 1)
            fprintf(stdout, "ALLOCATED BLOCK %d ON FREELIST\n", i);
    }
}

void testInodeAllocation() {
    int i;
    for (i = 2; i <= numInodes; i++) {
        if (allocatedInodes[i].inodeNum != 0 && freeInodes[i] == 1)
            fprintf(stdout, "ALLOCATED INODE %d ON FREELIST\n", i);
        else if (allocatedInodes[i].inodeNum == 0 && freeInodes[i] == 0)
            fprintf(stdout, "UNALLOCATED INODE %d NOT ON FREELIST\n", i);

        if(allocatedInodes[i].linkCount != allocatedInodes[i].directoryLinks)
            fprintf(stdout, "INODE %d HAS %u LINKS BUT LINKCOUNT IS %u\n", i, allocatedInodes[i].directoryLinks, allocatedInodes[i].linkCount);

        if(allocatedInodes[i].fileType == 'd'){
            if(allocatedInodes[i].actualParent != allocatedInodes[i].providedParentFromDotDot)
                fprintf(stdout, "DIRECTORY INODE %u NAME '..' LINK TO INODE %u SHOULD BE %u\n", allocatedInodes[i].inodeNum, allocatedInodes[i].providedParentFromDotDot, allocatedInodes[i].inodeNum);
        }
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
  csv = NULL, allocatedBlocks = NULL, freeBlocks = NULL, allocatedInodes = NULL, freeInodes = NULL;

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
        if (numInodes > -1) {
            freeInodes = (int*) malloc(sizeof(int) * (numInodes+1));
            memset(freeInodes, 0, sizeof(int) * (numInodes+1));
            allocatedInodes = (struct allocatedInode *) malloc(sizeof(struct allocatedInode) * (numInodes+1));
            memset(allocatedInodes, 0, sizeof(struct allocatedInode) * (numInodes+1));

            int counter = 3;
            for (; counter < firstNonreservedInode; counter ++)
                allocatedInodes[counter].inodeNum = counter;
        }



        int inodesPerBlock = BLOCK_SIZE/INODE_SIZE;
        int inodeTableBlocks = numInodes / inodesPerBlock;
	reservedBlockMax = 4 + inodeTableBlocks;

      }
      else if (strcmp(pch, "BFREE") == 0) {
        getBlockFreeListInfo(line);
      }
      else if (strcmp(pch, "IFREE") == 0) {
        getInodeFreeListInfo(line);
      }
      else if (strcmp(pch, "INODE") == 0) {
          // the direct block info is all in the inode rows
        getInodeInfo(line);
      }
      else if (strcmp(pch, "DIRENT") == 0) {
        getDirectoryEntry(line);
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
    testInodeAllocation();
  // Free memory
  freeMemory();

  return 0;
}
