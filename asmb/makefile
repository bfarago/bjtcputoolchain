
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I. -I../libbjtcpu
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC -std=c99
LIBS=-lstdc++ -L../libbjtcpu -lbjtcpu

SRC= lex.yy.c  main.c  scanner.c  grammar.c version.c  loc.c  intrep.c \
	gen_bin.c gen_fpga.c gen_lst.c

SRCPP= error.cpp  util.cpp 

OBJ= $(SRC:.c=.o) $(SRCPP:.cpp=.o)

all:	asmb

asmb:	$(OBJ)
		$(CC) -o asmb $(OBJ) $(LIBS)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
