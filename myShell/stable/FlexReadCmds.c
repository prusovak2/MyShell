%{
//INITIALIZATION

#include <stdio.h>
#include <unistd.h>
#include <sys/queue.h>
#include <err.h>
#include <string.h>
#include "safeAlloc.h"
#include "debugPrint.h"
#include "types.h"
#include "FlexReadCmds.h"

/*queue for tokens of command just read */
struct token
{
    TAILQ_ENTRY(token) tailToken;
    char * tokenItself;
};

/*queue for all commands */
struct command
{
    TAILQ_ENTRY(command) tailCommand;
    CMD commandItself;
};

/*define a structures that will act as the container for link list elements */
TAILQ_HEAD(tokens, token) tokenHead;
TAILQ_HEAD(commands, command) commandHead;


int tokCount=0;
int cmdCount=0;

%}

/*REGEX PART */
%%


#.* {
    /* comment */
    DEBUG_PRINT("Comment: %s\n", yytext);
}

[ \t]+ {
    /*white chars*/
    DEBUG_PRINT("White char\n");

}

>>|<|;|\||> {
    /*delimiter*/
    DEBUG_PRINT("delimiter: %s\n", yytext);
    delimiters delim;

   
    if(0 == strcmp(yytext, ">>"))
    {
        delim = reRightAppend;
    }
    else
    {
        char c = yytext[0];
        switch (c)
        {
            case '<':
                delim = reLeft;
                break;
            case '>':
                delim = reRight;
                break;
            case ';':
                delim = semicolon;
                break;
            case '|':
                delim = pipeChar;
                break;    
            default:       
                UNEXPECTED_PRINT("wier delim");
                break;
        }
    }

    AddCommand(delim);
}

[^\n\r\t\f \v<>;\|]+ {
    /*word*/
    DEBUG_PRINT("word: %s\n", yytext);
    //DEBUG_PRINT("%d\n", yytext[yyleng]); //yytext null terminated
   
    char * word = NULL;
    SAFE_MALLOC(word, yyleng);
    strcpy(word, yytext);  /*strcpy(destination, source)*/
    //DEBUG_PRINT("Copied string: %s\n", word);
    AddToken(word); 
    free(word);
    
}

\n {
    /*newline*/
    DEBUG_PRINT("newline\n");
    if(tokCount>0)
    {
        DEBUG_PRINT("last command ended with newline");
        AddCommand(newLine);
    }
}

. {
    UNEXPECTED_PRINT("unexpected char%s\n", yytext);
}
%%

/*Code part*/
/*int main()
{
   
   /TAILQ_INIT(&tokenHead);
    TAILQ_INIT(&commandHead);

    yylex();
    char* c;
    int i;
    ReadCMDs(c, &i);

  

    DEBUG_PRINT("Main: token count after readCmds: %d\n", tokCount);
    yylex_destroy();
    
} */

CMD * ReadCMDs(char * cmdLine, int * commandCount)
{
    TAILQ_INIT(&tokenHead);
    TAILQ_INIT(&commandHead);

    yy_scan_string(cmdLine);  //TODO: uncomment!
    yylex();

    /*no delim after the last cmd, some unprosessed tokens remaining in token que*/
    if(tokCount>0)
    {
        AddCommand(newLine);
    }

    /*extract CMDs from que to CMD array*/
    CMD * cmdArr;
    SAFE_MALLOC(cmdArr, (cmdCount+1)); /*alloc space for cmds including terminating null*/

    int i =0;    
    struct command * iterator;
    TAILQ_FOREACH(iterator, &commandHead, tailCommand)
    {
        *(cmdArr+i)=iterator->commandItself;
        i++;       
    }

    //carefull about this, cmdArr not mull terminated
    //cmdArr[cmdCount]=NULL;

    /*just debug print*/
    DEBUG_PRINT_GREEN("cmdCount: %d printing command\n", cmdCount);
    for (int i = 0; i < cmdCount; i++)
    {
        CMD c = *(cmdArr +i);
        DEBUG_PRINT("token count: %d\n", c.tokenCount);
        for (int j = 0; j < c.tokenCount; j++)
        {
            DEBUG_PRINT_GREEN("%s ", c.tokens[j]);
        }
        DEBUG_PRINT_GREEN("delim%d", (int)c.delim);
        DEBUG_PRINT("\n");       
    }

     /*free command queue*/
    struct command * p;
    while (!TAILQ_EMPTY(&commandHead))
     {
        p = TAILQ_FIRST(&commandHead);
        TAILQ_REMOVE(&commandHead, p, tailCommand);
        free(p);
        p=NULL;
    }


    *commandCount = cmdCount;
    yylex_destroy();
    return cmdArr;
}

void AddToken(char* word)
{
    struct token * newTok;
    SAFE_MALLOC(newTok, 1);

    char * copiedWord = NULL;
    int lenght = strlen(word); /*returns lenght including terminating null char*/
    SAFE_MALLOC(copiedWord, lenght);
    strcpy(copiedWord, word);

    newTok->tokenItself=copiedWord;

    TAILQ_INSERT_TAIL(&tokenHead, newTok,tailToken );

    tokCount++;
}

void AddCommand(delimiters delimiter)
{   
    /*prepare string array for tokens of which command consists*/
    char ** toks=NULL;
    SAFE_MALLOC(toks, tokCount+1);

    /*prepare CMD - structure to store command*/
    CMD  * newCMD;
    SAFE_MALLOC(newCMD,1);

    /*add token from token que to char** toks */
    int i =0;    
    struct token * iterator;
    TAILQ_FOREACH(iterator, &tokenHead, tailToken)
    {
        *(toks+i)=iterator->tokenItself;
        DEBUG_PRINT("iterator: %s toks: %s\n",iterator->tokenItself, *(toks+i));
        i++;       
    }
    
   // UNEXPECTED_PRINT("%s\n", *(toks+tokCount-1));
    (*(toks+tokCount))=NULL;

    /*free token queue*/
    struct token * p;
    while (!TAILQ_EMPTY(&tokenHead))
    {
        p = TAILQ_FIRST(&tokenHead);
        TAILQ_REMOVE(&tokenHead, p, tailToken);
        free(p);
        p=NULL;
    }

    /*just debud print*/
    UNEXPECTED_PRINT("tokcount: %d printing command\n", tokCount);
    for (int i = 0; i < tokCount; i++)
    {
        UNEXPECTED_PRINT("%s ", *(toks+i));
    }
    DEBUG_PRINT("\n");

    /*initialize new CMD*/
    newCMD->tokens = toks;
    newCMD->tokenCount = tokCount;
    newCMD->delim = delimiter;

    /*alloc structure to store CMD in que*/
    struct command  * newCommand;
    SAFE_MALLOC(newCommand,1);

    newCommand->commandItself=*newCMD;

    /*enqueue new command*/
    TAILQ_INSERT_TAIL(&commandHead, newCommand,tailCommand);

    tokCount=0;
    cmdCount++;
}





