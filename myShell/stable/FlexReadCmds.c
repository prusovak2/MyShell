%option noinput
%option nounput
%option noyywrap
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
    CMD * commandItself;
};

/*define a structures that will act as the container for link list elements */
TAILQ_HEAD(tokens, token) tokenHead;
TAILQ_HEAD(commands, command) commandHead;


int tokCount=0;
int cmdCount=0;

//input / output file expected as another word
int inExpected=0;
int outExpected=0;

//global storage for redirection data
char * currInFile = NULL;
char * currOutFile = NULL;
char flawed=0;
short int append;

//is currently some relavant file name stored?
int inFileRead=0;
int outFileRead=0;

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

;|\| {
    /*delimiter -  ; or | */
    DEBUG_PRINT("delimiter: %s\n", yytext);

    char c = yytext[0];
    if(inExpected || outExpected)
    {
        DEBUG_PRINT_YELLOW("file name for redirection expected, delim %c found.\n", c);
        flawed =1;
    }
    delimiters delim;

    switch (c)
        {
        case ';':
            delim = semicolon;
            break;
        case '|':
            delim = pipeChar;
            break;    
        default:       
            UNEXPECTED_PRINT("wierd delim\n");
            break;
        }
    AddCommand(delim);
}

>>|<|> {
    /*redirection*/
    char c = yytext[0];
    if(inExpected || outExpected)
    {
        DEBUG_PRINT_YELLOW("unexpected redir char %c\n", c);
        //more redir chars - what to do?
        //TODO: set inEXpected, outExpected to 0?
        flawed = c;        
    }
    else if(0 == strcmp(yytext, ">>"))
    {
        DEBUG_PRINT_YELLOW("redir char >>\n");
        outExpected=1;
        append=1;
    }
    else if(c=='>')
    {
        DEBUG_PRINT_YELLOW("redir char %c\n",c);
        outExpected=1;
        append=0;
    }
    else if(c=='<')
    {
        DEBUG_PRINT_YELLOW("redir char %c\n",c);
        inExpected=1;
    }
    else
    {
        UNEXPECTED_PRINT("unexpected redirection: %c - regex problem\n", c);
    }
}

[^\n\r\t\f \v<>;\|]+ {
    /*word*/
    DEBUG_PRINT("word: %s\n", yytext);
   
    char * word = NULL;
    SAFE_MALLOC(word, yyleng+1);
    strcpy(word, yytext);  /*strcpy(destination, source)*/
    int lenght = strlen(word);

    if(inExpected)
    {
        //word is supossed to be an input file name
        DEBUG_PRINT_YELLOW("new input filename: %s\n", word);

        if(currInFile!=NULL)
        {
             free(currInFile);
        }
        SAFE_MALLOC(currInFile, lenght+1);
        strcpy(currInFile, word);
        
        DEBUG_PRINT_YELLOW("currInFile: %s\n", currInFile);
        inFileRead=1;
        inExpected=0;
        free(word);
    }
    else if(outExpected)
    {
        //word is supossed to be an output file name
        DEBUG_PRINT_YELLOW("new output filename: %s\n", word);

         if(currOutFile!=NULL)
        {
             free(currOutFile);
        }
        SAFE_MALLOC(currOutFile,lenght+1);
        strcpy(currOutFile, word);

        DEBUG_PRINT_YELLOW("currOutFile: %s\n", currOutFile);
        outFileRead=1;        
        outExpected=0;
        free(word);
    }
    else
    {
        //ordinary  token
        AddToken(word); 
        free(word);
    }
}

\n {
    /*newline*/
    DEBUG_PRINT("newline\n");
    if(inExpected || outExpected)
    {
        DEBUG_PRINT_YELLOW("file name for redirection expected, NEWLINE found.\n");
        flawed =1;
    }
    if(tokCount>0)
    {
        DEBUG_PRINT("last command ended with newline\n");
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

CMD ** ReadCMDs(char * cmdLine, int * commandCount)
{
    TAILQ_INIT(&tokenHead);
    TAILQ_INIT(&commandHead);



    yy_scan_string(cmdLine);  
    yylex();

    /*no delim after the last cmd, some unprocessed tokens remaining in token que*/
    if(tokCount>0)
    {
        AddCommand(newLine);
    }

    /*extract CMDs from que to CMD array*/
    CMD ** cmdArr;
    SAFE_MALLOC(cmdArr, (cmdCount)); /*+1 deleted!!! no terminating null*/

    int i =0;    
    struct command * iterator;
    TAILQ_FOREACH(iterator, &commandHead, tailCommand)
    {
        *(cmdArr+i)=(iterator->commandItself);
        i++;       
    }

    //carefull about this, cmdArr not mull terminated
    //cmdArr[cmdCount]=NULL;

    /*just debug print*/
    DEBUG_PRINT_GREEN("cmdCount: %d printing command\n", cmdCount);
    for (int i = 0; i < cmdCount; i++)
    {
        CMD c = **(cmdArr +i);
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
    //FREE REDIRECTION STRING STORAGE
    free(currOutFile);
    free(currInFile);
    currInFile = NULL;
    currOutFile = NULL;

    *commandCount = cmdCount;
    cmdCount=0;
    yylex_destroy();
    DEBUG_PRINT_GREEN("LEAVING READCMDs, CMD count local: %d, cmd count returned %d\n", cmdCount, *commandCount);
    return cmdArr;
}

void AddToken(char* word)
{
    struct token * newTok;
    SAFE_MALLOC(newTok, 1);

    char * copiedWord = NULL;
    int lenght = strlen(word); /*returns lenght excluding terminating null char*/
    SAFE_MALLOC(copiedWord, (lenght+1));
    strcpy(copiedWord, word);

    newTok->tokenItself=copiedWord;

    TAILQ_INSERT_TAIL(&tokenHead, newTok,tailToken );

    tokCount++;
}

void AddCommand(delimiters delimiter)
{   
    /*prepare string array for tokens of which command consists*/
    char ** toks=NULL;
    SAFE_MALLOC(toks, (tokCount+1));  //+1

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
    (*(toks+tokCount))=(char*)NULL;

    /*free token queue*/
    struct token * p;
    while (!TAILQ_EMPTY(&tokenHead))
    {
        p = TAILQ_FIRST(&tokenHead);        
        TAILQ_REMOVE(&tokenHead, p, tailToken);
        free(p);
        p=NULL;
    }

    /*just debug print*/
    UNEXPECTED_PRINT("tokcount: %d printing command\n", tokCount);
    for (int i = 0; i < tokCount; i++)
    {
        UNEXPECTED_PRINT("%s ", *(toks+i));
    }
    DEBUG_PRINT("\n");

    /*REDIRECTION*/
    if(inFileRead || outFileRead)
    {
        DEBUG_PRINT_YELLOW("creating REDIR STRUCT\n");
        REDIR  * newRedir;
        SAFE_MALLOC(newRedir,1);
        
        //output
         if(outFileRead)
        {
            char * copiedOutFile = NULL;
            int lenght = strlen(currOutFile); /*returns lenght excluding terminating null char*/
            SAFE_MALLOC(copiedOutFile, (lenght+1));
            DEBUG_PRINT_YELLOW("currOutFile: %s\n", currOutFile);
            strcpy(copiedOutFile, currOutFile);
            DEBUG_PRINT_YELLOW("copied output: %s\n", copiedOutFile);

            newRedir->output = copiedOutFile;
            DEBUG_PRINT_YELLOW("output file: %s\n", newRedir->output);

            //this should free word
            //free(*currOutFile);
            outFileRead=0;

            newRedir->append=append;
            DEBUG_PRINT_YELLOW("append: %d\n", newRedir->append);
            append=0;
        }
        else
        {
            newRedir->output = NULL;
        }

        //input
        if(inFileRead)
        {
            char * copiedInFile = NULL;
            int lenght = strlen(currInFile); /*returns lenght excluding terminating null char*/
            SAFE_MALLOC(copiedInFile, (lenght+1));
            DEBUG_PRINT_YELLOW("currInFile: %s\n", currInFile);
            strcpy(copiedInFile, currInFile);
            DEBUG_PRINT_YELLOW("copied input: %s\n", copiedInFile);

            newRedir->input = copiedInFile;
            DEBUG_PRINT_YELLOW("input file: %s\n", newRedir->input);

            //this should free word
            //free(*currInFile);
            //*currInFile=NULL;
            inFileRead=0;
        }
        else
        {
            newRedir->input = NULL;
        }

        newCMD->redir = newRedir;
    }
    else
    {
        newCMD->redir = NULL;
    }
    /*initialize new CMD's other parts then redir*/
    newCMD->tokens = toks;
    newCMD->tokenCount = tokCount;
    newCMD->delim = delimiter;
    newCMD->flawedRedir = flawed;

    DEBUG_PRINT_YELLOW("flawed: %d\n", newCMD->flawedRedir);

    /*alloc structure to store CMD in que*/
    struct command  * newCommand;
    SAFE_MALLOC(newCommand,1);

    newCommand->commandItself=newCMD;

    /*enqueue new command*/
    TAILQ_INSERT_TAIL(&commandHead, newCommand,tailCommand);

    flawed=0;
    tokCount=0;
    cmdCount++;
}





