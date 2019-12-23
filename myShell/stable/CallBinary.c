#include <stdio.h>
#include "debugPrint.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>

int CallBinary(char* const comandLine[]);
int WaitForChild(pid_t childPID);

//TODO: improve return values

int main()
{
    DEBUG_PRINT("precall\n");
    char * arr[3]={"shit", "something"};
    CallBinary(arr);
    DEBUG_PRINT("after call\n");
}



int CallBinary(char * const comandLine[])
{
    pid_t pid = fork();
    if(pid ==-1)
    {
        DEBUG_PRINT("CallBInary: forking child failed\n");
        return -1;
    }
    else if(pid==0)
    {
        //child
        DEBUG_PRINT("preparing to execute child\n");
        int execErr =execvp(comandLine[0],comandLine);
        if(execErr==-1)
        {
            DEBUG_PRINT("CallBinary: execvp(%s, args) failed\n", comandLine[0]);
            return 127;
        }
        exit(0); //call succesfull
    }
    
    int ret = WaitForChild(pid);
    return  ret;


}

int WaitForChild(pid_t childPID)
{
    //int timeout = 1000000;
    int wstatus;
    DEBUG_PRINT("start waiting for child\n");
    int ret = waitpid(childPID, &wstatus, WNOHANG);
    
    while (0 == waitpid(childPID , &wstatus , WNOHANG)) 
  {
   /* timeout -= 1000;
    if ( timeout < 0 ) 
    {
            perror("timeout");
            return -1;
    }
    usleep(1000); */
  }

    if(ret == -1)
    {
        DEBUG_PRINT("waitForChild: error while waiting\n");
        return -1;
    }

    DEBUG_PRINT("waiting finished\n");

    if(! WIFEXITED(wstatus) ||  0!= WEXITSTATUS(wstatus))
    {
        DEBUG_PRINT("WaitForChild: child has terminated in unusual way\n");
        return -1;
    }
    return 0;


}