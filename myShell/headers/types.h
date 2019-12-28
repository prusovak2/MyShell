 #ifndef MY_TYPES_H
 #define MY_TYPES_H
 
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

#endif 
