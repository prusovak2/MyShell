#ifndef MY_FLEX_READ_CMDS_H
#define MY_FLEX_READ_CMDS_H
#include "types.h"


void AddToken(char* word);
void AddCommand(delimiters delimiter);
CMD * ReadCMDs(char * cmdLine, int * cmdCount);

#endif 