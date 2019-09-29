
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I. -I../libbjtcpu
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC -std=c99
CPPFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-lstdc++ -L../lib -lbjtcpu

SRC= lex.yy.c  main.c  scanner.c  grammar.c version.c  loc.c  intrep.c \
	gen_bin.c gen_fpga.c gen_lst.c

SRCPP= error.cpp  util.cpp 

OBJ= $(SRC:.c=.o) $(SRCPP:.cpp=.o)
# OBJ=$(patsubst %,$(OBJ_PATH)%,$(OBJ1))

all:	asmb

asmb:	$(OBJ)
		$(CC) -o asmb $(OBJ) $(LIBS)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
		$(CC) -c $(CPPFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)

check:
	@echo [INFO] Check asmb
	@#cppcheck --quiet --enable=all --xml-version=2 . 1>/dev/null 2>check.xml.log
	@cppcheck --quiet --enable=all --suppressions-list=cppcheck.suppressions.lst .