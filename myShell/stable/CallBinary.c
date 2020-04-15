#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <err.h>
#include <sys/wait.h>

#include "delimToString.h"
#include "debugPrint.h"
#include "CallBinary.h"
#include "MyShell.h"
#include "types.h"
#include "redir.h"

void handle_sigint_child(int sig);

pid_t childPid;

int CallBinary(char * const comandLine[], REDIR * redirP)
{
    struct sigaction act = { 0 };

	sigset_t sigset;
    sigset_t parentSigset;

	/* Block the SIGINT first. */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigprocmask(SIG_BLOCK, &sigset, &parentSigset);

    pid_t pid = fork();
    
    childPid = pid;

    if(pid == -1)
    {
        sigprocmask(SIG_SETMASK, &parentSigset, NULL);
        warnx("CallBinary: forking child failed\n");
        return 1;
    }
    else if(pid == 0)    
    {
        //child
        /* Install SIGINT handler. */
	    act.sa_handler = handle_sigint_child;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
	    sigaction(SIGINT, &act, NULL);
        sigprocmask(SIG_SETMASK, &parentSigset, NULL);

        if(redirP != NULL)
        {
            //REDIRECTION
            redir(redirP);
        }

        DEBUG_PRINT_TO_STDERR("CallBin: preparing to execute child\n");
        int execErr =execvp(comandLine[0],comandLine);
        if(execErr==-1)
        {
            DEBUG_PRINT_TO_STDERR("CallBinary: execvp(%s, args) failed\n", comandLine[0]);
            warnx("command not found: %s", comandLine[0] );
            exit (127);
        }        
        exit(127); //no exec happend, exit child - invalid command
    }

    //parent
    DEBUG_PRINT("Child pid is: %d\n", pid);
    int ret = WaitForChild(pid);

    //parent should ignore SIGINT that arrived while he had it blocked, as it has already been handeled by a child
    struct sigaction ignore = { 0 };
    struct sigaction original = { 0 };
    ignore.sa_handler = SIG_IGN;
    sigaction(SIGINT, &ignore, &original);
    //unlock SIGINT
    sigprocmask(SIG_SETMASK, &parentSigset, NULL);
    //after ublocking SIGINT, parent should no longer ignore it, blocked SIGINT had already been ignored
    sigaction(SIGINT, &original, NULL);

    return  ret;
}

int WaitForChild(pid_t childPID)
{
    int wstatus;
    DEBUG_PRINT("start waiting for child\n");
    int ret = waitpid(childPID, &wstatus,0);
    
    if(ret == -1)
    {
        UNEXPECTED_PRINT("waitForChild: error while waiting\n");
        return 1;
    }
    DEBUG_PRINT("waiting finished\n");

    //take care of return value:
    if(WIFEXITED(wstatus))
    {
        //child exited normally- either by calling exit() or by returning from main
        ret = WEXITSTATUS(wstatus); //exit status of child
        return ret;
    }
    if(WIFSIGNALED(wstatus))
    {
        //child was terminated by a signal
        ret = WTERMSIG(wstatus) + 128; //ret val = sinal num + 128
        return ret;
    }
    UNEXPECTED_PRINT("Wait for child: end of main was reached even thou it should not be!!!!");
    return 1;
}

void handle_sigint_child(int sig) 
{ 
  UNEXPECTED_PRINT("handle SIGINT FROM CHILD\n");
  warnx("Killed by signal %d", sig);
  kill(childPid, sig); 
}
