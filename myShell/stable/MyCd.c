#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>
#include <string.h>

#include "types.h"
#include "MyCd.h"
#include "debugPrint.h"
#include "safeAlloc.h"

//TODO: check whether OLDPWD is initialized

/*returns 0 when change of dir successful, 1 otherwise*/
int MyCd(CMD cmd)
{
    DEBUG_PRINT_GREEN("entering MyCD\n");
    int actuallyCd = strcmp(cmd.tokens[0], "cd");
    DEBUG_PRINT("CD: tokens[0]: %s, actuallyCd: %d\n", cmd.tokens[0], actuallyCd);
    if(actuallyCd!=0)
    {
        UNEXPECTED_PRINT("CD: another then cd cmd on input!\n");
        DEBUG_PRINT_GREEN("leaving MyCD\n");
        return 1;
    }
    if(cmd.tokenCount==1)
    {
        //cd without argument - cd $HOME
        char * home = getenv("HOME");
        DEBUG_PRINT_GREEN("cd %s   -HOME\n", home);
        int ret = Cd(home);

        //free(home);
        DEBUG_PRINT_GREEN("leaving MyCD\n");
        return ret;
    }
    if(cmd.tokenCount==2)
    {
        //cd with one argument - either dir or "-" or "~"
        int cdToPwd = strcmp(cmd.tokens[1], "-");
        int cdToHome = strcmp(cmd.tokens[1], "~");
        if(cdToPwd==0)
        {
            //cd -     go to OLDPWD
            char * oldPwd = getenv("OLDPWD");
            if(oldPwd==NULL)
            {
                DEBUG_PRINT_GREEN("OLDPWD is null");
                oldPwd = "/";
            }
            DEBUG_PRINT_GREEN("cd %s   -OLDPWD\n", oldPwd);
            printf("%s\n", oldPwd);
            int ret = Cd(oldPwd);

            //free(oldPwd);
            DEBUG_PRINT_GREEN("leaving MyCD\n");
            return ret;            
        }
        else if(cdToHome==0)
        {
            //cd ~    - cd $HOME
            char * home = getenv("HOME");
            DEBUG_PRINT_GREEN("cd %s   - ~ \n", home);
            int ret = Cd(home);

            //free(home);
            DEBUG_PRINT_GREEN("leaving MyCD\n");
            return ret;
        }
        else
        {
            //cd dir
            char* dir = cmd.tokens[1];
            DEBUG_PRINT_GREEN("cd %s   -DIR\n", dir);
            int ret = Cd(dir);
            DEBUG_PRINT_GREEN("leaving MyCD\n");
            return ret;
        }
    }
    else
    {
        //more than two args for cd - error
        warnx("cd: too many arguments");
        DEBUG_PRINT_GREEN("leaving MyCD\n");
        return 1;
    }
    
}

/*returns 0 when change of dir successful, 1 otherwise*/
int Cd(char * dir)
{
    DEBUG_PRINT_GREEN("entering CD\n");
    char * dirBeforeChange= getCurrDir();
    DEBUG_PRINT("dirBeforeChange: %s\n", dirBeforeChange);

    int ret = chdir(dir); //change dir
    if(ret == -1)
    {
        //error while chdir(), errno is set
        warn("cd: %s", dir); 
        DEBUG_PRINT("error message based on errno should be appended\n");
        return 1;
    }
    else
    {
        //set coresponding PWD and OLDPWD
        char * dirAfterChange = getCurrDir();
        DEBUG_PRINT("dirAfterChange: %s\n", dirAfterChange);
        setenv("PWD", dirAfterChange, 1);
        setenv("OLDPWD", dirBeforeChange, 1);
        free(dirAfterChange);
    }

   
    free(dirBeforeChange);
    DEBUG_PRINT_GREEN("leaving cd\n");
    return 0;

}

char * getCurrDir()
{
    DEBUG_PRINT("entering getCurDirr()\n");
    long path_max;
    size_t size;
    char *curDir;
    char *ptr;

    path_max = pathconf(".", _PC_PATH_MAX);
    if (path_max == -1)
    {
        size = 1024;
    }    
    else if (path_max > 10240)
    {
        size = 10240;
    }
    else
    {
        size = path_max;
    }
    
    //try to call getcwd() with increasing buffer size until it succees
    for (curDir = ptr = NULL; ptr == NULL; size *= 2)
    {
        SAFE_REALLOC(curDir, size);

        ptr = getcwd(curDir, size);

        if (ptr == NULL && errno != ERANGE)
        {
            UNEXPECTED_PRINT("insufficient size\n");
        }
    }
    return curDir;
}