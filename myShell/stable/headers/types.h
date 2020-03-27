 #ifndef MY_TYPES_H
 #define MY_TYPES_H
 
 typedef enum Tdelimiters
{
    pipeChar, semicolon, newLine
} delimiters;

typedef struct TREDIR
{
    char * input;
    char * output;
    short int append;
} REDIR;

typedef struct TCMD
{
    char** tokens;
    int tokenCount;
    delimiters delim;

    char flawedRedir;
    REDIR * redir;
} CMD;



//version of cmd before redirections
/*typedef struct TCMD
{
    char** tokens;
    int tokenCount;
    delimiters delim;
    
} CMD;

 typedef enum Tdelimiters
{
    reLeft, reRight, reRightAppend, pipeChar, semicolon, newLine
} delimiters;*/

#endif 
