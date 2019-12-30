CC = gcc
CFLAGS = -Wall -Wextra
FLEXFLAGS = -lfl
DFLAGS = -D DEBUG
INCLUDE = -I headers 
READLINE = -lreadline

OBJ = CallBinary.o delimToString.o ExecCmd.o  MyCd.o MyExit.o MyShell.o lex.yy.o

all: mysh

.PHONY : all clean 

mysh: $(OBJ) 
	$(CC) $(CFLAGS) $(INCLUDE) $(FLEXFLAGS) $(OBJ) -o mysh $(READLINE)

lex.yy.o: FlexReadCmds.c
	flex FlexReadCmds.c
	$(CC) $(CFLAGS) $(INCLUDE) $(FLEXFLAGS) -c lex.yy.c -o lex.yy.o $(READLINE)

#specify dependencies between object files and source files
%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(READLINE)

clean:
	rm *.o mysh lex.yy.c

