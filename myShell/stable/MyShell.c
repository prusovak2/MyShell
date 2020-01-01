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


void handle_sigint(int sig);

int lastRetVal =0;
int main(int argc, char ** argv)
{
    signal(SIGINT, handle_sigint);

    int option;
   //  ':' at the starting of the string so compiler can distinguish between '?' and ':'
   while((option = getopt(argc, argv, ":c:")) != -1)
   { 
      switch(option)
      {
         case 'c':
            //*********** -C OPTION ****************************
            DEBUG_PRINT("Given -c, arg: %s\n", optarg);
            char * line = optarg;
            ExecLine(line, &lastRetVal, 1); //just one line as arg of -c
            return lastRetVal;
         case ':':
            errx(2,"-%c: option requires an argument", optopt); //2 is ret val of bash for unknown option or missing oprarg
            
         case '?': //some unknown options
            errx(2,"invalid option: %c", optopt);
            
      }
   }

   
   if (optind<argc)
   { 
       //***************************INPUT FROM FILE********************
      DEBUG_PRINT("READING FILE: %s\n", argv[optind]);
        //read file
        int fd = open(argv[optind], O_RDONLY);
        char * line=NULL;
        int retVal=1;
        int lineNum =1;
        while(retVal!=-1)
        {
            line = ReadLineFromFile(&retVal, fd);
            DEBUG_PRINT_GREEN("Myshell: line: %s\n", line);
            ExecLine(line,&lastRetVal,lineNum);
            free(line);
            lineNum++;
        }
        close(fd);
      return lastRetVal; //ret val of last cmd
   }
   

    //***************INTERACTIVE SHELL********************
   while (1)
   {
       //create prompt
       char * pwd = getenv("PWD");   
       char * prompt;
       char * mysh ="  <mySh> % ";
       int len = strlen(pwd);
       int len2 = strlen(mysh);
       
       prompt = strdup(pwd);
       SAFE_REALLOC(prompt, len +len2);
       DEBUG_PRINT("concatenating prompt\n");
       DEBUG_PRINT("pwd: %s\n", pwd);
       DEBUG_PRINT("prompt: %s\n", prompt);
       strcat(prompt, mysh);
       DEBUG_PRINT("prompt: %s\n", prompt); 

       char * line = readline(prompt);
       free(prompt);
       DEBUG_PRINT_GREEN("LINE: %s\n", line);

       if(line==NULL)
       {
           DEBUG_PRINT("CTRL+D at the beggining of line, exiting\n");
           exit(lastRetVal);
       }

       //add history
       add_history(line);
       //exec cmds
       ExecLine(line, &lastRetVal, 1);
   }

   return lastRetVal;
   
}

int ExecLine(char * line, int * lastRetVal, int lineNum)
{
    //convert command line to an array of CMDs    
    int cmdCount = 0;
    DEBUG_PRINT("Myshell: ENTERING execLine, cmdCount: %d\n", cmdCount);
    CMD * cmds = ReadCMDs(line, &cmdCount);

    //just debug print
    DEBUG_PRINT_GREEN("EXEC LINE: printing cmds read:");
    DEBUG_PRINT_GREEN("cmdCount: %d\n", cmdCount);
    for (int i = 0; i < cmdCount; i++)
    {
        DEBUG_PRINT("cmd #%d, tokenCount: %d\n", i, (cmds+i)->tokenCount);
        for(int j = 0; j < (cmds+i)->tokenCount; j++)
        {
            DEBUG_PRINT("%s ", (cmds+i)->tokens[j]);
        }
        #ifdef DEBUG
            char * del = delimToString((cmds+i)->delim);
        #endif // DEBUG
        
        DEBUG_PRINT("delim:%s \n", del);
    }
    DEBUG_PRINT("\n\n");
    UNEXPECTED_PRINT("***********EXECUTING CMDS******************\n");

    //execute cmds
    int ret;
    for(int i = 0; i < cmdCount; i++)
    {
        ret =ExecCmd(cmds, *(cmds+i), lastRetVal, lineNum);

        DEBUG_PRINT("ret val of execCmd: %d\n", ret);
    }

/*
    for(int i=cmdCount-1;i>=0; i-- )
    {
        int tokCount =cmds[i].tokenCount;
        for (int j = 0; j < tokCount+1; j++)
        {
            UNEXPECTED_PRINT("execline: freeing %s \n", cmds[i].tokens[j]);
            free(cmds[i].tokens[j]);
        }
        free(cmds[i].tokens);
        free(cmds+i);
    }*/

    free(cmds);
    return ret;
}



void handle_sigint(int sig) 
{ 
    UNEXPECTED_PRINT(" CTRL+C IN PROMPT");
    printf("\n"); // Move to a new line
    rl_on_new_line(); // Regenerate the prompt on  newline
    rl_replace_line("", 0); // Clear the previous text
    rl_redisplay();
    lastRetVal= 128 + sig; 
} 