
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-lstdc++

SRC= lex.yy.c  main.c  scanner.c  version.c  loc.c 
SRCPP= error.cpp  util.cpp 

OBJ= $(SRC:.c=.o) $(SRCPP:.cpp=.o)

all:	asmb

asmb:	$(OBJ)
		$(CC) -o asmb $(LIBS) $(OBJ)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
