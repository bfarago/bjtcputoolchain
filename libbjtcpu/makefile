
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
OBJ_PATH := ../obj/
TARGET_PATH := ../lib/

TARGET1=libbjtcpu.a
OBJ1= $(SRC:.c=.o) $(SRCPP:.cpp=.o)


TARGET=$(patsubst %,$(TARGET_PATH)%,$(TARGET1))
OBJ=$(patsubst %,$(OBJ_PATH)%,$(OBJ1))

all: $(OBJ_PATH) $(TARGET_PATH) $(TARGET)

$(OBJ_PATH):
	@echo [INFO] Creating Directory [$(OBJ_PATH)]
	@mkdir $(OBJ_PATH)

$(TARGET_PATH):
	@mkdir $(TARGET_PATH)

$(TARGET): $(OBJ)
		@echo [INFO] Creating Static Library [$(TARGET)]
		@ar rcs $@ $(OBJ)
		
# .c.o:
#$(OBJ_PATH)%.o: $(SRC_PATH)%.c
#		$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp
		@echo [CPP] $<
		@$(CPP) -c $(CFLAGSPP) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJ)
	rm -f $(TARGET)
check:
	@echo [INFO] Check libbjtcpu
	@#cppcheck --quiet --enable=all --xml-version=2 . 1>/dev/null 2>$(OBJ_PATH)libbjtcpu.checkcpp.xml
	@cppcheck --quiet --enable=all .
