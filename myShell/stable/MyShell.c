#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <getopt.h>
#include <err.h>
#include <stdlib.h>
#include <signal.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "debugPrint.h"
#include "safeAlloc.h"
#include "types.h"
#include "FlexReadCmds.h"
#include "ExecCmd.h"
#include "delimToString.h"
#include "MyShell.h"
#include "ReadLineFromFile.h"
#include "pipeline.h"


void handle_sigint(int sig);

int lastRetVal = 0;

int main(int argc, char ** argv)
{
    struct sigaction act = {0};
    act.sa_handler = handle_sigint;
    sigaction(SIGINT, &act, NULL);
    
    int option;
    //  ':' at the starting of the string so compiler can distinguish between '?' and ':'
   while((option = getopt(argc, argv, ":c:")) != -1)
   { 
      switch(option)
      {
         case 'c':
            //*********** -C OPTION ****************************
            DEBUG_PRINT("Given -c, arg: %s\n", optarg);
            char * line;
            int len = strlen(optarg);
            SAFE_MALLOC(line,(len+1));
            strcpy(line,optarg);
            ExecLine(line, &lastRetVal, 1); //just one line as arg of -c
            return lastRetVal;
         case ':':
            errx(2,"-%c: option requires an argument", optopt); //2 is ret val of bash for unknown option or missing oprarg            
         case '?': 
            errx(2,"invalid option: %c", optopt);
            
      }
   }
   if (optind<argc)
   { 
       //***************************INPUT FROM FILE********************
      DEBUG_PRINT("READING FILE: %s\n", argv[optind]);
        //read file
        int fd = open(argv[optind], O_RDONLY);
        if(fd == -1)
        {
            err(42,"cannot open file");
        }
        char * line = NULL;
        int retVal = 1;
        int lineNum = 1;
        while(retVal !=-1)
        {
            line = ReadLineFromFile(&retVal, fd);
            DEBUG_PRINT_GREEN("Myshell: line: %s\n", line);
            ExecLine(line,&lastRetVal,lineNum);
            lineNum++;
        }
        close(fd);
      return lastRetVal; 
   }
    //***************INTERACTIVE SHELL********************
   while (1)
   {
       //create prompt
       char * pwd = getenv("PWD");   
       char * prompt;
       char * mysh = "  <mySh> % ";
       int len = strlen(pwd);
       int len2 = strlen(mysh);
       
       prompt = strdup(pwd);
       SAFE_REALLOC(prompt, (len +len2+1));
       DEBUG_PRINT("concatenating prompt\n");
       DEBUG_PRINT("pwd: %s\n", pwd);
       DEBUG_PRINT("prompt: %s\n", prompt);
       strcat(prompt, mysh);
       DEBUG_PRINT("prompt: %s\n", prompt);

       char * line = readline(prompt);
       free(prompt);
       DEBUG_PRINT_GREEN("LINE: %s\n", line);

       if(line == NULL)
       {
           DEBUG_PRINT("CTRL+D at the beggining of line, exiting\n");
           exit(lastRetVal);
       }
       add_history(line);
       //exec cmds +free line
       ExecLine(line, &lastRetVal, 1);
   }
    rl_clear_history();
    DEBUG_PRINT_GREEN("RET VAL FROM SHELL: %d\n",lastRetVal);
    return lastRetVal;
}

int ExecLine(char * line, int * lastRetVal, int lineNum)
{
    //convert command line to an array of CMDs    
    int cmdCount = 0;
    DEBUG_PRINT("Myshell: ENTERING execLine, cmdCount: %d\n", cmdCount);
    CMD ** Pcmds = ReadCMDs(line, &cmdCount);
    free(line);
    
    //just debug print
    DEBUG_PRINT_GREEN("EXEC LINE: printing cmds read:");
    DEBUG_PRINT_GREEN("cmdCount: %d\n", cmdCount);
    for (int i = 0; i < cmdCount; i++)
    {
        CMD * curr = *(Pcmds+i);
        DEBUG_PRINT("cmd #%d, tokenCount: %d\n", i, curr->tokenCount);
        for(int j = 0; j < curr->tokenCount; j++)
        {
            DEBUG_PRINT("%s ", curr->tokens[j]);
        }
        #ifdef DEBUG
            char * del = delimToString(curr->delim);
        #endif       
        DEBUG_PRINT("delim:%s \n", del);
    } 
    DEBUG_PRINT("\n\n");

    UNEXPECTED_PRINT("***********EXECUTING CMDS******************\n");
    //pipeline check
    int numOfPipes = 0;
    int startIndex;
    int brokenPipe = 0;
    //execute cmds
    int ret;
    for(int i = 0; i < cmdCount; i++)
    {
        CMD * curr = *(Pcmds+i);
        if(curr->delim == pipeChar)
        {
            if(numOfPipes == 0)
            {
                startIndex = i;               
            }
            numOfPipes++;
            if(curr->tokenCount <= 0) //syntax error, only |, no cmd
            {
                brokenPipe = 1;
                if(i == cmdCount-1)
                {
                    warnx("error: %d : syntax error near unexpected token \'|\'", lineNum);
                    ret = 73; //syntax error return value
                    *lastRetVal=73;
                    break;
                }
            }
        }
        else if(numOfPipes != 0) //delim different from pipe and some pipeline was read
        {
            if(brokenPipe) //pipeline is flawed - its not gonna ge created
            {
                brokenPipe = 0;
                numOfPipes = 0;
                warnx("error: %d : syntax error near unexpected token \'|\'", lineNum);
                ret = 73; //syntax error return value
                *lastRetVal = 73;
            }
            else
            {
                DEBUG_PRINT("create pipeline containing %d pipes\n starting at index %d\n", numOfPipes, startIndex);
            
                ret = pipeline(numOfPipes,startIndex,Pcmds,cmdCount, lineNum);
                *lastRetVal = ret;
                numOfPipes = 0;
                DEBUG_PRINT("THAT ret val of piplene.c: %d\n", ret);
            }
        }
        else
        {
            //simple command -  no pipeline
            ret = ExecCmd(Pcmds, *curr, lastRetVal, lineNum, cmdCount);
            DEBUG_PRINT("THAT ret val of execCmd: %d\n", ret);
        }
    }
    freeCmds(Pcmds, cmdCount); 
    DEBUG_PRINT_GREEN("RETURN VAL: %d\n", *lastRetVal);
    return ret; //useles value that is never used... Why have I deciced to return it?
    //Im already using lastRetVal param to propagete retVal of last cmd to main
}

void freeCmds(CMD ** Pcmds, int cmdCount)
{
    for(int i = 0; i < cmdCount; i++)   
    {
        CMD * curr = *(Pcmds+i);
        int tokCount =curr->tokenCount;
        for (int j = 0; j < tokCount+1; j++)
        {
            UNEXPECTED_PRINT("execline: freeing %s \n", curr->tokens[j]);
            free(curr->tokens[j]);
        }
        free(curr->tokens);

        if(curr->redir != NULL)
        {
            if(curr->redir->input != NULL)
            {
                UNEXPECTED_PRINT("execline: freeing %s \n", curr->redir->input);
                free(curr->redir->input);
            }
            if(curr->redir->output != NULL)
            {
                UNEXPECTED_PRINT("execline: freeing %s \n", curr->redir->output);
                free(curr->redir->output);
            }
            free(curr->redir);
        }
        free(*(Pcmds+i));
    }
    free(Pcmds);
}

void handle_sigint(int sig) 
{ 
    UNEXPECTED_PRINT(" CTRL+C IN PROMPT");
    printf("\n"); // Move to a new line
    rl_on_new_line(); // Regenerate the prompt on  newline
    rl_replace_line("", 0); // Clear the previous text
    rl_redisplay();
    lastRetVal= 128 + sig; //should this be here? ctrl+c changes ret val of last cmd...
} 