/** @file intrep.c
*
* @brief Internal representation modul.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "intrep.h"
#include "util.h"

#ifndef MAXSHORT
#include <limits.h>
#define MAXSHORT SHRT_MAX
#endif

//Actual section type
memoryType_t sectionType = MT_code;

//this holds all of the meta data for each memory location
memoryMetaData_t memoryMeta[MAXMEMORY];

//Actual section name
char section[MAXSECTIONNAME];
//Actual section id
size_t sectionId = 0u;

//holds memory content
char memory[MAXMEMORY];


FILE* include_fopen(const char* fname, const char* mode) {
	include_add(fname);
#ifndef _WIN32
	return fopen(fname, mode);
#else
	FILE* f = NULL;
	fopen_s(&f, fname, mode);
	return f;
#endif
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
	if (sectionId > MAXSHORT) {
		//error
		sectionId = MAXSHORT;
	}
	pMeta->sectionId = (short)sectionId;
	
	pMeta->sectionType = sectionType;
	pMeta->fileId = include_actual_get();
	memory[address++] = data & 0xf;
	chkAddress(address);
}


#if (1==INTREP_UNUSED_ENABLED)

int getMemoryType(int address) {
	return memoryMeta[address].sectionType;
}

int getMemorySectionId(int address) {
	return memoryMeta[address].sectionId;
}
int getMemoryLineNo(int address) {
	return memoryMeta[address].line;
}
#endif

memoryMetaData_t* getMemoryMeta(int address) {
	return &memoryMeta[address];
}
