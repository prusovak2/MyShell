#include <stdio.h>

#include "types.h"
#include "delimToString.h"

char * delimToString(delimiters delim)
{
    char * res;
    switch (delim)
    {
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