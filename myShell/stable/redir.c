#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <unistd.h>
#include <stdio.h>

#include "debugPrint.h"
#include "types.h"
#include "redir.h"

void redir(REDIR * redirP)
{
    //REDIRECTION
    if(redirP->input != NULL)
    {   
        DEBUG_PRINT_YELLOW("redirecting stdin to %s\n", redirP->input);
        int fd;            
        if ((fd = open(redirP->input, O_RDONLY)) == -1)
        {
            //open sets errno on failure
            err(1,"%s: error while opening the file",redirP->output);
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
            }
            DEBUG_PRINT_YELLOW(">> opened sucessfully\n");                  
        }
        else
        {
            if ((fd = open(redirP->output, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1)
            {
                err(1,"%s: error while opening the file",redirP->output);
            } 
            DEBUG_PRINT_YELLOW("> opened sucessfully\n");     
        }
        close(1); //close stdout
        dup(fd); //duplicate file fd to stdin
        close(fd); //duplicated fd no longer needed               
    }
}