 #ifndef MY_CALLBIN_H
 #define MY_CALLBIN_H //to prevent double declaration of stuff
 
 #include "types.h"
 #include <sys/types.h>

int CallBinary(char* const comandLine[], REDIR * redir);
int WaitForChild(pid_t childPID);

#endif