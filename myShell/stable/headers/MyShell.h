#ifndef MY_SHELL_H
#define MY_SHELL_H

int ExecLine(char * line, int * lastRetVal, int lineNum);
void handle_sigint(int sig);

#endif // !MY_SHELL_H
