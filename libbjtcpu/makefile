
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
CPP=g++
INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC -std=c99
CFLAGSPP= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC -std=c++0x
#LIBS=-lstdc++
LIBS=
SRC= 
SRCPP= constants.cpp

OBJ= $(SRC:.c=.o) $(SRCPP:.cpp=.o)

all:	libbjtcpu.a

libbjtcpu.a: $(OBJ)
		ar rcs libbjtcpu.a $(OBJ)
		
.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.cpp.o:
		$(CPP) -c $(CFLAGSPP) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
