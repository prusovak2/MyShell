#include <unistd.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "debugPrint.h"
#include "ReadLineFromFile.h"
#include "safeAlloc.h"

#define CHAR_BUFFER_SIZE  1024
#define LINE_BUFFER_SIZE  256

int readCharFromFile(int fd)
{
    static char buffer[CHAR_BUFFER_SIZE];
    static int posRead = 0;
    static int posBuffed = 0;

    if(posRead >= posBuffed)
    {
        //we need to read new buffer from file
        int charsRead = read(fd, buffer, CHAR_BUFFER_SIZE);
        if(charsRead == (-1))
        {
            //errno is set
            err(42, "error while reading file");
        }
        if(charsRead == 0)
        {
            DEBUG_PRINT("END OF FILE\n");
            return -1;
        }
        else
        {
            DEBUG_PRINT("readChar: reading first char from a new buffer\n");
            posBuffed = charsRead;
            posRead = 0;
            DEBUG_PRINT("readChar: pos read: %d, char to return: %c\n", posRead, buffer[posRead]);
            return buffer[posRead++];
        }
    }
    else
    {
        DEBUG_PRINT("readChar: returning char\n");
        DEBUG_PRINT("readChar: pos read: %d, char to return: %c\n", posRead, buffer[posRead]);
        return buffer[posRead++];
    }
}

char *  ReadLineFromFile(int * retVal, int fd)
{
    char *line;
    SAFE_MALLOC( line, LINE_BUFFER_SIZE);
    size_t  buffSize = LINE_BUFFER_SIZE;

    int charCounter = 0;
    while(1)
    {
        int charRead = readCharFromFile(fd);
        if(charRead == (-1))
        {
            DEBUG_PRINT("Read line from file: end of FILE\n");
            line[charCounter] = '\0'; //terminating null
            DEBUG_PRINT("READLINE FROM FILE:line: %s\n", line);
            *retVal = -1;
            return line;
        }
        if(charRead == '\n')
        {
            DEBUG_PRINT("Read line from file: end of LINE\n");
            line[charCounter] = '\0'; //terminating null
            DEBUG_PRINT("READLINE FROM FILE: line: %s\n", line);
            *retVal = 0;
            return line;
        }
        if((size_t)charCounter >= buffSize)
        {
            DEBUG_PRINT("Read line from file: realoc buffer\n");
            buffSize *= 2;
            SAFE_REALLOC(line, buffSize );
            DEBUG_PRINT("new buffer size %d\n", (int)buffSize);
        }

        DEBUG_PRINT("adding char %c, charCounter: %d\n", charRead, charCounter);
        DEBUG_PRINT("line lenght %d\n", (int)strlen(line));
        line[charCounter] = charRead;
        charCounter++;
    }
}

    
    
    


