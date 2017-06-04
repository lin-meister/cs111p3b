#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "ext2_fs.h"

#define BUF_SIZE 1024

struct ext2_super_block * superBlock;
struct ext2_group_desc * groupBlock;

unsigned char *blockBitmap;
unsigned char *inodeBitmap;

unsigned int numberOfInodes;

int fd;

//Free memory for all allocated blocks
void freeMemory()
{
  free(superBlock);
  free(groupBlock);
  free(blockBitmap);
  free(inodeBitmap);  
}

//Error return function
void error(char* msg)
{
  fprintf(stderr, "%s\n", msg);
  exit(2);
}

int
main (int argc, char **argv)
{
  // File system image name is in argv
  if (argc != 2) {
    fprintf(stderr, "USAGE: ./lab3b FILENAME.csv\n");
    exit(1);
  }


  return 0;
}
