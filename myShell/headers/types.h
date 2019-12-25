 typedef enum Tdelimiters
{
    reLeft, reRigt, reRigtAppend, pipeChar, semicolumn, newLine
} delimiters;

typedef struct TCMD
{
    char** tokens;
    int tokenCount;
    delimiters delim;
    
} CMD;


