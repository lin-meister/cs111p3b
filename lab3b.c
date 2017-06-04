



#define BLOCK_SIZE 1024

char * csv;
int csvFileSize;

int numBlocks, numInodes;

int fd;

//Free memory for all allocated blocks
void freeMemory()
{
  free(csv);
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
  while (tok != NULL) {
    if (num == 2) numBlocks = atoi(tok);
    if (num == 3) numInodes = atoi(tok);
    num++;
    tok = strtok(NULL, ",");
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
  
  int lineCount = 1;
  char * pch;
  pch = strtok(csv, "\n");
  while (pch != NULL) {
    if (lineCount == 1) getSuperblockInfo(pch);
    pch = strtok(NULL, "\n");
    lineCount++;
  }

  printf("%d\n", numInodes);
  
  // Free memory
  freeMemory();

  return 0;
}
