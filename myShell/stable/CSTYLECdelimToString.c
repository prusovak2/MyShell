#include <stdio.h>

#include "delimToString.h"
#include "types.h"

char *delimToString(delimiters delim) {
  char *res;
  switch (delim) {
  case reLeft:
    res = "<";
    break;
  case reRight:
    res = ">";
    break;
  case reRightAppend:
    res = ">>";
    break;
  case pipeChar:
    res = "|";
    break;
  case semicolon:
    res = ";";
    break;
  case newLine:
    res = "\\n";
    break;

  default:
    break;
  }
  return res;
}