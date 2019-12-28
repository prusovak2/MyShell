#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include "MyExit.h"
#include "types.h"
#include "debugPrint.h"
#include "safeAlloc.h"

/*int main()
{
    CMD * c1;
    SAFE_MALLOC(c1,1);
    c1->delim = semicolon;
    c1->tokenCount=1;
    char * toks[] ={"exit"};
    c1->tokens= toks;

    MyExit(*c1, 73); 
}*/

int MyExit(CMD cmd, int lastCmdRetVal)
{
    int actuallyExit = strcmp(cmd.tokens[0], "exit");
    DEBUG_PRINT("tokens[0]: %s, actuallyExit: %d\n", cmd.tokens[0], actuallyExit);
    if(actuallyExit!=0)
    {
        UNEXPECTED_PRINT("exit: another then exit cmd on input!\n");
        return 1;
    }
    else
    {
        if(cmd.tokenCount==1)
        {
            //exit without argument - exit with last cmd ret. val
            UNEXPECTED_PRINT("EXITING");
            exit(lastCmdRetVal);
        }
        /*else if(cmd.tokenCount==2)
        {
            exit with argument????
        }*/
        else
        {
            //invalid amount of args
            warnx("exit: too many arguments");
            return 1;
        }
    }
    
}