#define _XOPEN_SOURCE 700
#include <err.h>
#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>              
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

#include "delimToString.h"
#include "types.h"
#include "debugPrint.h"
#include "safeAlloc.h"
#include "redir.h"
#include "pipeline.h"

#define WRITE_END 1
#define READ_END 0

void handle_sigint_pipe(int sig);

pid_t * pids;
int numofForked = 0;

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

    //alloc array  array of pids    
    SAFE_MALLOC(pids, numOfProcesses);
    //prepare storego for pipe
    int pip[2];
    
    //to store read end fd of previous pipe created
    int in = 0;

    //SIGNAL HANDLING
    struct sigaction act = { 0 };
	sigset_t sigset;
    sigset_t parentSigset;
	/* Block the SIGINT first. */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigprocmask(SIG_BLOCK, &sigset, &parentSigset);

for (int i = 0; i < numOfProcesses; i++)
{
    DEBUG_PRINT_TO_STDERR("parent, iteration %d, in = %d\n", i, (int)in);
    //create pipe to connect child process from this iteration with a child process from the next one
    pipe(pip);
    
    pids[i] = fork();
    if(pids[i] ==-1)
    {
        sigprocmask(SIG_SETMASK, &parentSigset, NULL);
        errx(1,"pipeline: forking child failed\n");
    }

    if(pids[i]==0)
    {
        //CHILD
        /* Install SIGINT handler. */
	    act.sa_handler = handle_sigint_pipe;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
	    sigaction(SIGINT, &act, NULL);
        sigprocmask(SIG_SETMASK, &parentSigset, NULL);

        DEBUG_PRINT_TO_STDERR("child %d, in: %d\n", i, (int)in);
        DEBUG_PRINT_TO_STDERR("child %d started\n",i);
        
        CMD * currCmd =  *(cmds+(startIndex+i));
        DEBUG_PRINT_GREEN("current cmd: %s", currCmd->tokens[1]);

        //REDIRECTON
        if(currCmd->flawedRedir)
        {
            DEBUG_PRINT_YELLOW("FLAWED REDIRECTION due to %c\n", currCmd->flawedRedir);
            warnx("error: %d : syntax error near unexpected token \'%c\'", lineNum, currCmd->flawedRedir);
            retVal = 73; //my special ret val for syntax error
            return retVal;
        }
        if (currCmd->redir != NULL) //redir is REDIR *
        {
            redir(currCmd->redir);
        }

        if(i<numOfPipes&&( currCmd->redir==NULL || currCmd->redir->output==NULL)) //do not rewrite redirected fds
            {
                //not in last
                close(1); //close stdout
                //redirect output to wr. end of pipe i
                int outCheck = dup(pip[WRITE_END]);
                if(outCheck!=1)
                {
                    DEBUG_PRINT_TO_STDERR("SOME AWEFUL MISTAKE out fd is %d instead of 1\n", outCheck);
                    errx(73,"pipeline: duplication of filedescriptor failed");
                }
                close(pip[WRITE_END]);
            }
        
        if(i != 0 && ( currCmd->redir==NULL || currCmd->redir->output==NULL)) //do not rewrite redirected fds
            {
                //not in first
                close(0); //close stdin
                int inCheck= dup(in);
                //close(pip[WRITE_END]);
                close(in);
                if(inCheck!=0)
                {
                    DEBUG_PRINT_TO_STDERR("SOME AWEFUL MISTAKE in fd is %d instead of 0\n", inCheck);
                    errx(73,"pipeline: duplication of filedescriptor failed");
                }                
            }
            close(pip[WRITE_END]);
            close(pip[READ_END]);
            close(in);

            DEBUG_PRINT_TO_STDERR("executing comamnd %d:\n",i);                
            DEBUG_PRINT_TO_STDERR("%d\n",currCmd->tokenCount);
            for(int j = 0; j < currCmd->tokenCount; j++)
            {
                DEBUG_PRINT_TO_STDERR("%s ", currCmd->tokens[j]);
            }

            int execErr = execvp(currCmd->tokens[0],currCmd->tokens);
            if(execErr==-1)
            {
                UNEXPECTED_PRINT("Pipeline: execvp(%s, args) failed\n", currCmd->tokens[0]);
                errx(127,"command not found: %s", currCmd->tokens[0] );
            }
        }
        //parent
        numofForked++;
        DEBUG_PRINT_TO_STDERR("PARENT AFTER FORKING CHILD %d\n",i);
        if(in!=0)
        {
            close(in);
        }
        in= pip[READ_END];
        close(pip[WRITE_END]);
        DEBUG_PRINT_TO_STDERR("parent, after forking child %d, in = %d\n", i, (int)in);
    }

    for (int i = 0; i < numOfProcesses; i++)
    {
        DEBUG_PRINT_GREEN("Pipeline Parent: waiting for child %d\n", i);
        int status;
        waitpid(pids[i],&status,0);
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

    //TO RESET SIGACTION
    //parent should ignore SIGINT that arrived while he had it blocked, as it was handeled by a child
    struct sigaction ignore = { 0 };
    struct sigaction original = { 0 };
    ignore.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore, &original);
    sigprocmask(SIG_SETMASK, &parentSigset, NULL);
    //after ublocking SIGINT, parent should no longer ignore it, blocked SIGINT had already been ignored
    sigaction(SIGINT, &original, NULL);


    //cleanup - is it correct?
    free(pids);

    return retVal;
}

void handle_sigint_pipe(int sig) 
{ 
  UNEXPECTED_PRINT("handle SIGINT FROM CHILD\n");
  warnx("Killed by signal %d", sig);
  for (int i = 0; i < numofForked; i++)
  {
      kill(pids[i], sig); 
  }
}