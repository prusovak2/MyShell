#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ReadLineFromFile.h"
#include "debugPrint.h"
#include "safeAlloc.h"

#define CHAR_BUFFER_SIZE 1024
#define LINE_BUFFER_SIZE 256

int readCharFromFile(int fd) {
  static char buffer[CHAR_BUFFER_SIZE];
  static int posRead = 0;
  static int posBuffed = 0;

  if (posRead >= posBuffed) {
    // we need to read new buffer from file
    int charsRead = read(fd, buffer, CHAR_BUFFER_SIZE);
    if (charsRead == (-1)) {
      // error while readning, errno is set
      err(42, "error while reading file");
    }
    if (charsRead == 0) {
      DEBUG_PRINT("END OF FILE\n");
      return -1;
    } else {
      DEBUG_PRINT("readChar: reading first char from a new buffer\n");
      posBuffed = charsRead;
      posRead = 0;
      DEBUG_PRINT("readChar: pos read: %d, char to return: %c\n", posRead,
                  buffer[posRead]);
      return buffer[posRead++];
    }
  } else {
    DEBUG_PRINT("readChar: returning char\n");
    DEBUG_PRINT("readChar: pos read: %d, char to return: %c\n", posRead,
                buffer[posRead]);
    return buffer[posRead++];
  }
}

char *ReadLineFromFile(int *retVal, int fd) {
  char *line;
  SAFE_MALLOC(line, LINE_BUFFER_SIZE);
  size_t buffSize = LINE_BUFFER_SIZE;

  int charCounter = 0;
  while (1) {
    int charRead = readCharFromFile(fd);

    if (charRead == (-1)) {
      DEBUG_PRINT("Read line from file: end of FILE\n");
      line[charCounter] = '\0'; // terminating null
      DEBUG_PRINT("READLINE FROM FILE:line: %s\n", line);
      *retVal = -1;
      return line;
    }
    if (charRead == '\n') {
      DEBUG_PRINT("Read line from file: end of LINE\n");
      line[charCounter] = '\0'; // terminating null
      DEBUG_PRINT("READLINE FROM FILE: line: %s\n", line);
      *retVal = 0;
      return line;
    }
    if ((size_t)charCounter >= buffSize) {
      DEBUG_PRINT("Read line from file: realoc buffer\n");
      buffSize *= 2;
      SAFE_REALLOC(line, buffSize);
      DEBUG_PRINT("new buffer size %d\n", (int)buffSize);
    }

    DEBUG_PRINT("adding char %c, charCounter: %d\n", charRead, charCounter);
    DEBUG_PRINT("line lenght %d\n", (int)strlen(line));
    line[charCounter] = charRead;
    charCounter++;
  }
}

/*
    int main(int argc, char ** argv)
    {
        if(argc!=2)
        {
            errx(111,"ASSHOLE - invalid amount of args of the test main");
        }

        int fd = open(argv[1], O_RDONLY);
        int ret;
        char * line=NULL;
        while((ret=ReadLineFromFile(&line, fd))!=-1)
        {
            printf("line: %s\n", line);
            free(line);
        }
    } */
