#include <err.h>
#include <stdio.h>
#include <string.h>

#include "CallBinary.h"
#include "MyCd.h"
#include "MyExit.h"
#include "debugPrint.h"
#include "delimToString.h"
#include "types.h"

int ExecCmd(CMD **cmdPointer, CMD cmd, int *lastRetVal, int lineNum,
            int cmdCount) {
  DEBUG_PRINT_GREEN("entering ExecCmd, linenum: %d\n", lineNum);
  int lrv = *lastRetVal;
  DEBUG_PRINT("lrv: %d\n", lrv);

  if (cmd.delim != semicolon && cmd.delim != newLine) {
    // TODO:remove for a phase 2!!!
    // delim < >> > |
    char *stringDelim = delimToString(cmd.delim);
    warnx("error: %d : syntax error near unexpected token \'%s\'", lineNum,
          stringDelim);
    *lastRetVal = 73; // my special ret val for syntax error
    return 73;
  }

  // syntax error
  if (cmd.tokenCount <= 0) {
    // TODO: exit on syntax error?
    // empty cmd - containts only delim - synax error
    char *stringDelim = delimToString(cmd.delim);
    warnx("error: %d : syntax error near unexpected token \'%s\'", lineNum,
          stringDelim);
    *lastRetVal = 73; // my special ret val for syntax error
    return 73;
  }

  int IsCd = strcmp(cmd.tokens[0], "cd");
  DEBUG_PRINT("execcmd: tokens[0]: %s, actuallyCd: %d\n", cmd.tokens[0], IsCd);
  int IsExit = strcmp(cmd.tokens[0], "exit");
  DEBUG_PRINT("execcmd: tokens[0]: %s, actuallyExit: %d\n", cmd.tokens[0],
              IsExit);

  if (IsExit == 0) {
    DEBUG_PRINT_GREEN("ExecCmd: caling myExit()");
    int ret = MyExit(cmdPointer, cmd, lrv, cmdCount);
    // exit failed
    *lastRetVal = ret;
    return ret;
  }
  if (IsCd == 0) {
    DEBUG_PRINT_GREEN("ExecCmd: caling MyCd()");
    int ret = MyCd(cmd);
    *lastRetVal = ret;
    return ret;
  } else {
    DEBUG_PRINT_GREEN("ExecCmd: caling binary");
    int ret = CallBinary(cmd.tokens);
    *lastRetVal = ret;
    return ret;
  }
}