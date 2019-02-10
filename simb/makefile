
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-lstdc++

SRC=
SRCPP= simb.cpp

OBJ= $(SRC:.c=.o) $(SRCPP:.cpp=.o)

all:	simb

asmb:	$(OBJ)
		$(CC) -o simb $(LIBS) $(OBJ)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
