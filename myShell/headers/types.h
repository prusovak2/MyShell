 typedef enum Tdelimiters
{
    reLeft, reRight, reRightAppend, pipeChar, semicolon, newLine
} delimiters;

typedef struct TCMD
{
    char** tokens;
    int tokenCount;
    delimiters delim;
    
} CMD;


