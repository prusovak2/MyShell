%{
//INITIALIZATION

#include <stdio.h>
#include <unistd.h>
#include <sys/queue.h>
#include <err.h>
#include <string.h>
#include "safeAlloc.h"
#include "debugPrint.h"
#include "types.h"
#include "FlexReadCmds.h"

/*queue for tokens of command just read */
struct token
{
    TAILQ_ENTRY(token) tailToken;
    char * tokenItself;
};

/*queue for all commands */
struct command
{
    TAILQ_ENTRY(command) tailComand;
    CMD commandItself;
};

/*define a structures that will act as the container for link list elements */
TAILQ_HEAD(tokens, token) tokenHead;
TAILQ_HEAD(commands, command) commandHead;

 


int tokCount=0;
int cmdCount=0;

%}

/*REGEX PART */
%%


#.* {
    /* comment */
    DEBUG_PRINT("Comment: %s\n", yytext);
}

[ \t]+ {
    /*white chars*/
    DEBUG_PRINT("White char\n");

}

>>|<|;|\||> {
    /*delimiter*/
    DEBUG_PRINT("delimiter: %s\n", yytext);
    delimiters delim;

   
    if(0 == strcmp(yytext, ">>"))
    {
        delim = reRightAppend;
    }
    else
    {
        char c = yytext[0];
        switch (c)
        {
            case '<':
                delim = reLeft;
                break;
            case '>':
                delim = reRight;
                break;
            case ';':
                delim = semicolon;
                break;
            case '|':
                delim = pipeChar;
                break;    
            default:       
                UNEXPECTED_PRINT("wier delim");
                break;
        }
    }

    
    

    AddCommand(delim);


}

[^\n\r\t\f \v<>;\|]+ {
    /*word*/
    DEBUG_PRINT("word: %s\n", yytext);
    //DEBUG_PRINT("%d\n", yytext[yyleng]); //yytext null terminated
   
    char * word = NULL;
    SAFE_MALLOC(word, yyleng);
    strcpy(word, yytext);  /*strcpy(destination, source)*/
    //DEBUG_PRINT("Copied string: %s\n", word);
    AddToken(word); 
    free(word);
    
}

\n {
    /*newline*/
    DEBUG_PRINT("newline\n");
}

. {
    UNEXPECTED_PRINT("unexpected char%s\n", yytext);
}
%%

/*Code part*/
int main()
{
   /*initialize queues*/
    TAILQ_INIT(&tokenHead);
    TAILQ_INIT(&commandHead);

    yylex();
    struct token * iterator;
    DEBUG_PRINT("\n");
    TAILQ_FOREACH(iterator, &tokenHead, tailToken)
    {
        DEBUG_PRINT("%s ", iterator->tokenItself);
    }

    DEBUG_PRINT("tokencount: %d\n", tokCount);
    yylex_destroy();
    
}

void AddToken(char* word)
{
    struct token * newTok;
    SAFE_MALLOC(newTok, 1);

    char * copiedWord = NULL;
    int lenght = strlen(word); /*returns lenght including terminating null char*/
    SAFE_MALLOC(copiedWord, lenght);
    strcpy(copiedWord, word);

    newTok->tokenItself=copiedWord;

    TAILQ_INSERT_TAIL(&tokenHead, newTok,tailToken );

    tokCount++;
}
//
void AddCommand(delimiters delimiter)
{
    /*prepare string array for tokens of which command consists*/
    char ** toks=NULL;
    SAFE_MALLOC(toks, tokCount+1);

    struct command * newCMD;
    SAFE_MALLOC(newCMD,1);

    int i =0;    
    struct token * iterator;
    TAILQ_FOREACH(iterator, &tokenHead, tailToken)
    {
        (*(toks+i))=(iterator->tokenItself);
        DEBUG_PRINT("iterator: %s toks: %s\n",iterator->tokenItself, *(toks+i));
        
    }
    (*(toks+tokCount))=NULL;

    struct token * p;
    while (!TAILQ_EMPTY(&tokenHead)) {
    p = TAILQ_FIRST(&tokenHead);
    TAILQ_REMOVE(&tokenHead, p, tailToken);
    free(p);
    p=NULL;
}

    //TODO:alloc command!


}





