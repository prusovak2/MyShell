#ifndef MY_SHELL_H
#define MY_SHELL_H
#include "types.h"

int ExecLine(char * line, int * lastRetVal, int lineNum);
void handle_sigint(int sig);
void freeCmds(CMD ** Pcmds, int cmdCount);

#endif // !MY_SHELL_H
