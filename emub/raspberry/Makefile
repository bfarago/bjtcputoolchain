
#DEBUG=-g -O0
DEBUG=-O2 -ggdb

ifeq ($(OS), Windows_NT)
export PATH := g:\prg\SysGCC\Raspberry\bin\;$(PATH)
CC=arm-linux-gnueabihf-gcc.exe
else
CC=gcc
endif


INCLUDE=-I.
DEFS=-DDEBUG
CFLAGS= $(DEBUG) $(DEFS) -Wformat=2 -Wall -Wextra -Winline $(INCLUDE) -pipe -fPIC -std=c99
LIBS=-lm -lpthread -lwiringPi

SRC= Pwm.c PwmDrv_raspi.c VideoDrv_bjtcpu.c VideoDrv_fb.c VideoDrv.c \
		Keyboard.c \
		Det.c  \
		playb.c Sim.c

OBJ= $(SRC:.c=.o)

all:	playb

playb:	$(OBJ)
		$(CC) -o playb $(OBJ) $(LIBS)

.c.o:
		$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)

check:
	@#cppcheck --quiet --enable=all --xml-version=2 . 1>/dev/null 2>check.xml.log
	cppcheck --quiet --enable=all --suppressions-list=cppcheck.suppressions.lst .
installprereq:
	sudo apt-get install wiringpi