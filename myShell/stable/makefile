CC = gcc
CFLAGS = -Wall -Wextra -std=c99
FLEXFLAGS = -lfl
DFLAGS = -D DEBUG
INCLUDE = -I Includes -l readline

OBJ = CallBinary.o delimToString.o ExecCmd.o lex.yy.o MyCd.o MyExit.o MyShell.o

all: clean mysh

.PHONY : all clean 

mysh: $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDE) $(FLEXFLAGS) $(OBJ) -o mysh

lex.yy.o: FlexReadCmds.c
	flex FlexReadCmds.c
	$(CC) $(CFLAGS) $(INCLUDES) $(FLEXFLAGS) -c lex.yy.c -o lex.yy.o

#specify dependencies between object files and source files
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm $(OBJ) mysh lex.yy.c

