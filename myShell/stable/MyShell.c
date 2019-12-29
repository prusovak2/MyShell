#include <unistd.h>
#include <getopt.h>

#include "debugPrint.h"
#include "types.h"
#include "FlexReadCmds.h"
#include "ExecCmd.h"
#include "delimToString.h"

int main(int argc, char ** argv)
{
    int lastRetVal =0;
    int option;
   // put ':' at the starting of the string so compiler can distinguish between '?' and ':'
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

   //TODO: solve cases with file arg and wihout args reading stdin 
   for(; optind < argc; optind++){ //when some extra arguments are passed
      printf("Given extra arguments: %s\n", argv[optind]);
   }
}

int ExecLine(char * line, int * lastRetVal, int lineNum)
{
    //convert command line to an array of CMDs
    int cmdCount = 0;
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
        char * del = delimToString((cmds+i)->delim);
        DEBUG_PRINT("delim:%s \n", del);
    }
    DEBUG_PRINT("\n\n");
    UNEXPECTED_PRINT("***********EXECUTING CMDS******************\n");

    //execute cmds
    for(int i = 0; i < cmdCount; i++)
    {
        int ret =ExecCmd(cmds, *(cmds+i), lastRetVal, lineNum);

        DEBUG_PRINT("ret val of execCmd: %d\n", ret);
    }

    free(cmds);
}