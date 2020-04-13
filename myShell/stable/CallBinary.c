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

void handle_sigint_child(int sig);

//TODO: delete main

/*int main()
{
    DEBUG_PRINT("precall\n");
    char * arr[3]={"sleep", "20"};
    int ret = CallBinary(arr);   
    DEBUG_PRINT("after call\n");
    printf("is it gonna be white?\n");
    UNEXPECTED_PRINT("is this gonna be red?\n");
    return ret;
}
*/
pid_t childPid;

int CallBinary(char * const comandLine[], REDIR * redirP)
{
    signal(SIGINT, handle_sigint_child);
    pid_t pid = fork();
    
    childPid = pid;

    if(pid ==-1)
    {
        warnx("CallBinary: forking child failed\n");
        return 1;
    }
    else if(pid==0)    
    {
        //child
        if(redirP !=NULL)
        {
            //REDIRECTION
            if(redirP->input != NULL)
            {   
                DEBUG_PRINT_YELLOW("redirecting stdin to %s\n", redirP->input);
                int fd;            
	            if ((fd = open(redirP->input, O_RDONLY)) == -1)
                {
                    err(1,"%s: error while opening the file",redirP->output);
                    //warnx("%s: no such file",redirP->input);
                    //return or exit
                }
                DEBUG_PRINT_YELLOW("< opened sucessfully\n");   
                close(0); //close stdin
                dup(fd);  //duplicate file fd to stdin
                close(fd); //duplicated fd no longer needed
            }
            if(redirP->output!=NULL)
            {
                DEBUG_PRINT_YELLOW("redirecting stdout to %s\n", redirP->output);
                int fd;
                if(redirP->append)
                {
                    DEBUG_PRINT_YELLOW("Append\n");
                    if ((fd = open(redirP->output, O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
                    {
                        err(1,"%s: error while opening the file",redirP->output);
                         //warnx("%s: error while opening the file",redirP->output);
                         //exit(1);
                    }
                    DEBUG_PRINT_YELLOW(">> opened sucessfully\n");                  
                }
                else
                {
                    if ((fd = open(redirP->output, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
                    {
                        err(1,"%s: error while opening the file",redirP->output);
                         //warnx("%s: error while opening the file",redirP->output);
                         //exit(1);
                    } 
                    DEBUG_PRINT_YELLOW("> opened sucessfully\n");     
                }
                close(1); //close stdout
                dup(fd); //duplicate file fd to stdin
                close(fd); //duplicated fd no longer needed               
            }
        }
        DEBUG_PRINT_TO_STDERR("CallBin: preparing to execute child\n");
        int execErr =execvp(comandLine[0],comandLine);
        if(execErr==-1)
        {
            DEBUG_PRINT_TO_STDERR("CallBinary: execvp(%s, args) failed\n", comandLine[0]);
            warnx("command not found: %s", comandLine[0] );
            //return 127;
            exit (127);
        }        
        exit(127); //no exec happend, exit child - invalid command
    }
    //parent
    DEBUG_PRINT("Child pid is: %d\n", pid);
    int ret = WaitForChild(pid);
    signal(SIGINT, handle_sigint);
    return  ret;


}

int WaitForChild(pid_t childPID)
{
    //int timeout = 1000000;
    int wstatus;
    DEBUG_PRINT("start waiting for child\n");
    int ret = waitpid(childPID, &wstatus,0);
    
   // while (0 == waitpid(childPID , &wstatus , WNOHANG)) 
 // {
   /* timeout -= 1000;
    if ( timeout < 0 ) 
    {
            perror("timeout");
            return -1;
    }
    usleep(1000); */
 // }

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
        //child terminated by signal
        ret = WTERMSIG(wstatus) + 128; //ret val= sinal num + 128
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
