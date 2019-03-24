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
memoryType_t sectionType = MT_code;

//this holds all of the meta data for each memory location
memoryMetaData_t memoryMeta[MAXMEMORY];

//Actual section name
char section[MAXSECTIONNAME];
//Actual section id
int sectionId = 0;

//holds memory content
char memory[MAXMEMORY];


FILE* include_fopen(const char* fname, const char* mode) {
	include_add(fname);
	return fopen(fname, mode);
}

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
	memoryMetaData_t* pMeta = &memoryMeta[address];
	pMeta->line= yylineno;
	pMeta->sectionId = sectionId;
	
	pMeta->sectionType = sectionType;
	pMeta->fileId = include_actual_get();
	memory[address++] = data & 0xf;
	chkAddress(address);
}

int getMemoryType(int address) {
	return memoryMeta[address].sectionType;
}

int getMemorySectionId(int address) {
	return memoryMeta[address].sectionId;
}
int getMemoryLineNo(int address) {
	return memoryMeta[address].line;
}
memoryMetaData_t* getMemoryMeta(int address) {
	return &memoryMeta[address];
}