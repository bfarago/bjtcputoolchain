
#DEBUG=-g -O0
DEBUG=-O2
CC=gcc
INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC
LIBS=-lm -lpthread -lwiringPi

SRC= Pwm.c PwmDrv_raspi.c VideoDrv_bjtcpu.c \
		Det.c  \
		playb.c

OBJ= $(SRC:.c=.o)

all:	playb

playb:	$(OBJ)
		$(CC) -o playb $(LIBS) $(OBJ)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)