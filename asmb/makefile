
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-lc++

SRC= error.cpp  lex.yy.c  main.c  scanner.c  util.cpp  version.c  loc.c

OBJ= $(SRC:.c=.o)

all:	simpletest

simpletest:	$(OBJ)
		$(CC) -o simpletest $(LIBS) $(OBJ)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
