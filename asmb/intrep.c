/** @file intrep.c
*
* @brief Internal representation modul.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "intrep.h"
#include "util.h"

//Actual section type
memoryType sectionType = MT_code;

//Actual section name
char section[MAXSECTIONNAME];

//Holds memory type for all the location (to be easier)
memoryType memoryTypes[MAXMEMORY];

//holds memory content
char memory[MAXMEMORY];

//holds the input source line numbers
int lines[MAXMEMORY];


//actual address setter, keep the max value for later use
void setAddress(int a) {
	address = a;
	if (a > maxaddress) maxaddress = a;
}

//checks the earlier maximum for addresses
void chkAddress(int a) {
	if (a > maxaddress) maxaddress = a;
}

//to add code or data to the memory
void addMemory(int data) {
	lines[address] = yylineno;
	memoryTypes[address] = sectionType;
	memory[address++] = data & 0xf;
	chkAddress(address);
}

int getMemoryType(int address) {
	return memoryTypes[address];
}
int getMemoryLineNo(int address) {
	return lines[address];
}