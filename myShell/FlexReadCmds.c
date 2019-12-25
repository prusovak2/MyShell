%{
//INITIALIZATION

#include <stdio.h>
#include <unistd.h>
#include <sys/queue.h>
#include <err.h>
#include "safeAlloc.h"
#include "debugPrint.h"
#include "types.h"

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

}

[^\n\r\t\f \v<>;\|]+ {
    /*word*/
    DEBUG_PRINT("word: %s\n", yytext);
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
    yylex();
    yylex_destroy();
}





