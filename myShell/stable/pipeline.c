#include <err.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>              
#include <unistd.h>
#include <sys/wait.h>

#include "delimToString.h"
#include "types.h"
#include "debugPrint.h"
#include "safeAlloc.h"
#include "redir.h"

#define WRITE_END 1
#define READ_END 0

int pipeline(int numOfPipes, int startIndex, CMD ** cmds, int cmdCount, int lineNum)
{
    DEBUG_PRINT("Entering pipeline.c\n");
    //initialize variable containing return value somehow
    //is 1 suitable?
    int retVal=1;

    DEBUG_PRINT_GREEN("cmd count: %d, begining pipeline on index %d\n", cmdCount, startIndex);
    if(startIndex+numOfPipes+1 > cmdCount)
    {
        err(73, "syntax error in pipeline");
    }
    //just for debug print purposes
    CMD * firstCmd =  *(cmds+startIndex);
    DEBUG_PRINT("first cmd from pipeline:\n");
    for(int j = 0; j < firstCmd[0].tokenCount; j++)
    {
         DEBUG_PRINT("%s ", firstCmd[0].tokens[j]);
    }
    DEBUG_PRINT("\n");
    //end of debug print section

    int numOfProcesses = numOfPipes+1;

    //alloc array of pipes and array of pids
    pid_t * pids;
    SAFE_MALLOC(pids, numOfProcesses);
    int ** pipes;
    SAFE_MALLOC(pipes, numOfPipes);
    for (int i = 0; i < numOfPipes; i++)
    {
        SAFE_MALLOC(pipes[i],2);
    }
    
    for(int i = 0; i < numOfProcesses; i++)
    {
        if(i < numOfPipes)
        {
            DEBUG_PRINT("pipe %d created\n",i);
            pipe(pipes[i]);
        }
    
        pids[i] = fork();
        if(pids[i]==0)//child
        {
            DEBUG_PRINT("child %d started\n",i);
            CMD * currCmd =  *(cmds+(startIndex+i));
            DEBUG_PRINT_GREEN("current cmd: %s", currCmd->tokens[1]);
            if(i>=2)
            {
                //close all pipes that this child does not need
                for (int j = 0; j <= i-2; j++)
                {
                    DEBUG_PRINT("child %d closing fd %d\n", i, j);
                    close(pipes[j][WRITE_END]);
                    close(pipes[j][READ_END]);
                }
            }
            //REDIRECTON
            if(currCmd->flawedRedir)
            {
                DEBUG_PRINT_YELLOW("FLAWED REDIRECTION due to %c\n", currCmd->flawedRedir);
                warnx("error: %d : syntax error near unexpected token \'%c\'", lineNum, currCmd->flawedRedir);
                retVal = 73; //my special ret val for syntax error
                return retVal;
            }
            if (currCmd->redir != NULL) //redid is REDIR *
            {
                redir(currCmd->redir);
            }
            
            if(i<numOfPipes &&( currCmd->redir==NULL || currCmd->redir->output==NULL)) //do not rewrite redirected fds
            {
                //not in last child
                //for write end of each pipe
                close(1); //close stdout
                //redirect output to wr. end of pipe i
                //dup reopens fd 1
                int out = dup(pipes[i][WRITE_END]);
                if(out!=1)
                {
                    errx(73,"pipeline: duplication of filedescriptor failed");
                }
                close(pipes[i][READ_END]);
                
                //just trying it out
                close(pipes[i][WRITE_END]);
            }        
            if(i != 0 && ( currCmd->redir==NULL || currCmd->redir->output==NULL)) //do not rewrite redirected fds
            {
                //not in first child
                //for read end of each pipe
                close(0); //close stdin
                //dup reopens fd 0
                int in = dup(pipes[i-1][READ_END]);
                close(pipes[i-1][WRITE_END]);
                if(in!=0)
                {
                    errx(73,"pipeline: duplication of filedescriptor failed");
                }

                //just trying it out
                close(pipes[i-1][READ_END]);                
            }
                DEBUG_PRINT_GREEN("executing comamnd %d:\n",i);                
                DEBUG_PRINT("%d\n",currCmd->tokenCount);
                for(int j = 0; j < currCmd->tokenCount; j++)
                {
                    DEBUG_PRINT("%s ", currCmd->tokens[j]);
                }

            int execErr = execvp(currCmd->tokens[0],currCmd->tokens);
            if(execErr==-1)
            {
                UNEXPECTED_PRINT("Pipeline: execvp(%s, args) failed\n", currCmd->tokens[0]);
                errx(127,"command not found: %s", currCmd->tokens[0] );
                //return 127;
            }         
        }
    }
    for (int i = 0; i < numOfPipes; i++)
    {
        DEBUG_PRINT("parent: closing fd %d\n",i);
        close(pipes[i][WRITE_END]);
        close(pipes[i][READ_END]);
    }
    for (int i = 0; i < numOfProcesses; i++)
    {
        DEBUG_PRINT_GREEN("Pipeline Parent: waiting for child %d\n", i);
        int status;
        waitpid(pids[i],&status,0);
        /* while(0 == waitpid(pids[i] , &status ,0))
        {             
            sleep(1);
            printf("P: waitpid %d\n",i);
        }*/
        if(WIFEXITED(status))
        {
            DEBUG_PRINT("Child %d exited with val %d",i,retVal);
            //child exited normally- either by calling exit() or by returning from main
            retVal = WEXITSTATUS(status); //exit status of child
        }
        if(WIFSIGNALED(status))
        {
             DEBUG_PRINT("Child %d was terminated by a signal %d",i,retVal);
            //child terminated by signal
            retVal = WTERMSIG(status) + 128; //ret val= sinal num + 128
        }
        
    }

    //cleanup - is it correct?
    free(pids);
    for (int i = 0; i < numOfPipes; i++)
    {
        free(pipes[i]);
    }
    free(pipes);

    return retVal;
}