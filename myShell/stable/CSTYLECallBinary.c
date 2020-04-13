#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "CallBinary.h"
#include "MyShell.h"
#include "debugPrint.h"

void handle_sigint_child(int sig);

// TODO: delete main

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

int CallBinary(char *const comandLine[]) {
  signal(SIGINT, handle_sigint_child);
  pid_t pid = fork();

  childPid = pid;

  if (pid == -1) {
    warnx("CallBinary: forking child failed\n");
    return 1;
  } else if (pid == 0) {

    // child
    DEBUG_PRINT("CallBin: preparing to execute child\n");
    int execErr = execvp(comandLine[0], comandLine);
    if (execErr == -1) {
      DEBUG_PRINT("CallBinary: execvp(%s, args) failed\n", comandLine[0]);
      warnx("command not found: %s", comandLine[0]);
      // return 127;
      exit(127);
    }
    exit(127); // no exec happend, exit child - invalid command
  }
  // parent
  DEBUG_PRINT("Child pid is: %d\n", pid);
  int ret = WaitForChild(pid);
  signal(SIGINT, handle_sigint);
  return ret;
}

int WaitForChild(pid_t childPID) {
  // int timeout = 1000000;
  int wstatus;
  DEBUG_PRINT("start waiting for child\n");
  int ret = waitpid(childPID, &wstatus, WNOHANG);

  while (0 == waitpid(childPID, &wstatus, WNOHANG)) {
    /* timeout -= 1000;
     if ( timeout < 0 )
     {
             perror("timeout");
             return -1;
     }
     usleep(1000); */
  }

  if (ret == -1) {
    UNEXPECTED_PRINT("waitForChild: error while waiting\n");
    return 1;
  }

  DEBUG_PRINT("waiting finished\n");

  // take care of return value:
  if (WIFEXITED(wstatus)) {
    // child exited normally- either by calling exit() or by returning from main
    ret = WEXITSTATUS(wstatus); // exit status of child
    return ret;
  }
  if (WIFSIGNALED(wstatus)) {
    // child terminated by signal
    ret = WTERMSIG(wstatus) + 128; // ret val= sinal num + 128
    return ret;
  }
  UNEXPECTED_PRINT(
      "Wait for child: end of main was reached even thou it should not be!!!!");
  return 1;
}

void handle_sigint_child(int sig) {
  UNEXPECTED_PRINT("handle SIGINT FROM CHILD\n");
  warnx("Killed by signal %d", sig);
  kill(childPid, sig);
}