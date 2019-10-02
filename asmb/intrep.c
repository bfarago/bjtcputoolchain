/** @file intrep.c
*
* @brief Internal Representation module.
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

//actual address setter, keep the max value for later use
void setAddress(SimAddress_t a) {
	if (a < MAXMEMORY) {
			address = a;
			if (a > maxaddress) maxaddress = a;
	}
}

//checks the earlier maximum for addresses
void chkAddress(SimAddress_t a) {
	if (a < MAXMEMORY) {
			if (a > maxaddress) maxaddress = a;
	}
}

memoryMetaData_t* getMemoryMeta(SimAddress_t address) {
	if (address >= MAXMEMORY) {
		return NULL;
	}
	return &memoryMeta[address];
}

//to add code or data to the memory
void addMemory(SimData_t data) {
	memoryMetaData_t* pMeta = getMemoryMeta(address);
	if (!pMeta) {
		//todo: error handling
		return;
	}
	pMeta->line= yylineno;
	if (sectionId > MAXSHORT) {
		//todo: error case handling
		sectionId = MAXSHORT;
	}
	pMeta->sectionId = (short)sectionId;
	
	pMeta->sectionType = sectionType;
	pMeta->fileId = include_actual_get();
	memory[address] = data & 0xf;
	if (address < MAXMEMORY-1) {
		address++;
	}
	else
	{
		//todo: error case handling
	}
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
