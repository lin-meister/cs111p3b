//NAME: Nikhil Bhatia, Chaoran Lin
//UID: 104132751, 004674598
//EMAIL: nbhatia823@ucla.edu, linmc@ucla.edu


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

int numErrors = 0;

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
    unsigned int directoryLinks;
    unsigned int actualParent;
    unsigned int providedParentFromDotDot;
    char fileType;
};

struct dirEntry {
  unsigned int directoryInode;
  unsigned int inodeNum;
  char* name;
};

int * freeBlocks; 
struct allocatedBlock * allocatedBlocks;
int * freeInodes;
struct allocatedInode * allocatedInodes;
struct dirEntry * dirEntries; 


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

//Read superblock and setup initial data from it
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

//Confirm block number is valid
int isValidBlockNum(int block) {
    if(block < numBlocks && block > reservedBlockMax)
        return 1;
    else if (block <= reservedBlockMax && block > 0)
        return 0; //reserved block
    else return -1; //invalid block
}

//Confirm inode number is valid
int isValidInodeNum(int num) {
  if((num <= numInodes && num >= firstNonreservedInode) || num == 2  )
    return 1;
  else return 0;
}

//Read block from free list
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
	else if (rc == 0) {
	  fprintf(stdout, "RESERVED BLOCK %d IN FREE LIST\n", blockNum);
	  numErrors++;
	}
	else {
	  fprintf(stdout, "INVALID BLOCK %d IN FREE LIST\n", blockNum);
	  numErrors++;
	}
      }
  }
}

//Read inode from free list
void getInodeFreeListInfo(char* str) {
    char* tok = strtok(str, ",");
    int num = 1;
    for (; tok != NULL; tok = strtok(NULL, ","), num++) {
      if (num == 2) {
	int inodeNum = atoi(tok);
	if(isValidInodeNum(inodeNum))
	  freeInodes[inodeNum] = 1;
	else {
	  fprintf(stdout, "INVALID INODE %d IN FREE LIST\n", inodeNum);
	  numErrors++;
	}
      }
    }
}

//Names stored on heap
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
    if (isValidInodeNum(inodeNum)) {
        if(mode == 0)
            return;
        allocatedInodes[inodeNum].inodeNum = inodeNum;
        allocatedInodes[inodeNum].linkCount = linkCount;
        allocatedInodes[inodeNum].fileType = fileType;
    }
    else {
        fprintf(stdout, "INVALID INODE %d\n", inodeNum);
        return;
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



void getDirectoryEntry(char *str) {
    char * tok = strtok(str, ",");

    //Populate variables from CSV line
    int num = 1, inodeNum = 0, directoryInodeNum = 0, nameLength = 0;
    char* name;
    for (; tok != NULL; tok = strtok(NULL, ","), num++) {
        if(num == 2) directoryInodeNum = atoi(tok);
        if(num == 4) inodeNum = atoi(tok);
        if(num == 6) nameLength = atoi(tok);
        if(num == 7) name = strdup(tok);
    }

    //Replace newline characrter with nullbyte at end of name
    name[nameLength+2] = '\0';    

    //Check if inodeNum is valid, if not print error
    if(isValidInodeNum(inodeNum) == 0) {
        fprintf(stdout, "DIRECTORY INODE %d NAME %s INVALID INODE %d\n", directoryInodeNum, name, inodeNum);
        return;
    }

    //If looking at CWD entry, ensure it is pointing to current directory
    if(strcmp(name, "\'.\'") == 0) {
      if (directoryInodeNum != inodeNum)
	fprintf(stdout, "DIRECTORY INODE %d NAME %s LINK TO INODE %d SHOULD BE %d\n", directoryInodeNum, name, inodeNum, directoryInodeNum);
    }
    //If looking at parent directory entry, store given parent 
    else if (strcmp(name, "\'..\'") == 0){
      allocatedInodes[directoryInodeNum].providedParentFromDotDot = inodeNum;
    }
    //If normal directory entry, store directory entry info and the parent of this inode as the directory entry
    else {
      allocatedInodes[inodeNum].actualParent = directoryInodeNum;
      dirEntries[inodeNum].name = name;
      dirEntries[inodeNum].directoryInode = directoryInodeNum;
      dirEntries[inodeNum].inodeNum = inodeNum;
    }
  
    //Add to link counts
    allocatedInodes[inodeNum].directoryLinks++;

}

//Go through all blocks and check if there are any that are not on free list and not on allocated and not reserved
void testUnreferencedBlocks() {
    int i;
    for (i = 1; i < numBlocks; i++) {
        if (freeBlocks[i] == 0 && allocatedBlocks[i].blockNum == 0 && i > reservedBlockMax)
            fprintf(stdout, "UNREFERENCED BLOCK %d\n", i);
    }
}

//Go through all blocks and ensure that there are no blocks on free list and allocated
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
      //If inode is on free list and is allocated, report error
        if (allocatedInodes[i].inodeNum != 0 && freeInodes[i] == 1)
            fprintf(stdout, "ALLOCATED INODE %d ON FREELIST\n", i);
	//If inode is not allocated but not on free list, report error
        else if (allocatedInodes[i].inodeNum == 0 && freeInodes[i] == 0)
            fprintf(stdout, "UNALLOCATED INODE %d NOT ON FREELIST\n", i);
	//If inode is allocated, and the link count provided does not match number of links in directory entries, report error
        if(allocatedInodes[i].inodeNum != 0 &&  allocatedInodes[i].linkCount != allocatedInodes[i].directoryLinks)
            fprintf(stdout, "INODE %d HAS %u LINKS BUT LINKCOUNT IS %u\n", i, allocatedInodes[i].directoryLinks, allocatedInodes[i].linkCount);
	//If inode is not allocated but there is a directory entry for the inode number
	if(dirEntries[i].inodeNum != 0 && allocatedInodes[i].inodeNum == 0)
	  fprintf(stdout, "DIRECTORY INODE %d NAME %s UNALLOCATED INODE %d\n", dirEntries[i].directoryInode, dirEntries[i].name, dirEntries[i].inodeNum);
	//If inode is directory, check to see if it's actual parent matches the parent provided by .. file
        if(allocatedInodes[i].fileType == 'd') {
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

  //Open file, if not opened, report error and exit
  FILE* stream = fopen(csvFile, "r");
  if(stream == NULL) {
    fprintf(stderr, "USAGE: ./lab3b FILENAME.csv\n");
    exit(1);
  }
 
  //Go line by line and interpret and store data from CSV
  char line[1024];
  while (fgets(line, 1024, stream)) {
      char * temp = strdup(line);
      char * pch = strtok(temp, ",");
      //Get the first word from read in line and then forward to proper function to handle line type
      if (strcmp(pch, "SUPERBLOCK") == 0) {
        getSuperblockInfo(line);
        if (numBlocks > -1)
        {
	  //Setup data structures for blocks
            freeBlocks = (int*) malloc(sizeof(int) * (numBlocks));
            memset(freeBlocks, 0, sizeof(int) * numBlocks);
            allocatedBlocks = (struct allocatedBlock *) malloc(sizeof(struct allocatedBlock) * numBlocks);
            memset(allocatedBlocks, 0, sizeof(struct allocatedBlock) * numBlocks);
        }
        if (numInodes > -1) {
	  //Setup data structures for blocks and inodes
            freeInodes = (int*) malloc(sizeof(int) * (numInodes+1));
            memset(freeInodes, 0, sizeof(int) * (numInodes+1));
            allocatedInodes = (struct allocatedInode *) malloc(sizeof(struct allocatedInode) * (numInodes+1));
            memset(allocatedInodes, 0, sizeof(struct allocatedInode) * (numInodes+1));
	    dirEntries = (struct dirEntry *) malloc(sizeof(struct dirEntry) * (numInodes + 1));
	    memset(dirEntries, 0, sizeof(struct dirEntry) * (numInodes+1));
	    //Set the actual parent for root inode to itself as there will be no dirents for the root
	    allocatedInodes[2].actualParent = 2;

	    //Set the reserved inodes as allocated so they are not marked as unallocated and not in free list
            int counter;
            for (counter = 3; counter < firstNonreservedInode; counter ++)
                allocatedInodes[counter].inodeNum = counter;
        }

	//Determine the number of blocks taken by inode table block
        int inodesPerBlock = BLOCK_SIZE/INODE_SIZE;
        int inodeTableBlocks = numInodes / inodesPerBlock;
	//Set reserved block max number, decrement by one if block size is larger than 1kb due to EXT2 structure
	reservedBlockMax = 4 + inodeTableBlocks;
	if(BLOCK_SIZE > 1024) 
	  reservedBlockMax--;
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

  //Test all data structures
    testUnreferencedBlocks();
    testAllocatedBlocksConsistency();
    testInodeAllocation();

  // Free memory
  freeMemory();

  return 0;
}
