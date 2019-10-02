/** @file intrep.h
*
* @brief Internal Representation module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _INTREP_H_
#define _INTREP_H_

#include "config.h"
#include "libbjtcpu.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//this holds all of the meta data for each memory location
extern memoryMetaData_t memoryMeta[MAXMEMORY];

//Actual section type
extern memoryType_t sectionType;
extern size_t sectionId;

//Actual section name
extern char section[MAXSECTIONNAME];

//holds memory content
extern char memory[MAXMEMORY];

//actual address setter, keep the max value for later use
void setAddress(SimAddress_t a);

//checks the earlier maximum for addresses
void chkAddress(SimAddress_t a);

//to add code or data to the memory
void addMemory(SimData_t data);

//to get the memory metadata
memoryMetaData_t* getMemoryMeta(SimAddress_t address);

#if (1==INTREP_UNUSED_ENABLED)
//to get memory usage kind
int getMemoryType(int address);

//to get the memory section id
int getMemorySectionId(int address);

//to get original source line
int getMemoryLineNo(int address);
#endif

#ifdef __cplusplus
}
#endif

#endif