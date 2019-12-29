 #ifndef MY_CALLBIN_H
 #define MY_CALLBIN_H //to prevent double declaration of stuff

#include <sys/types.h>

int CallBinary(char* const comandLine[]);
int WaitForChild(pid_t childPID);

#endif