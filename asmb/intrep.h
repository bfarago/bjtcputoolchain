/** @file grammar.h
*
* @brief Internal Representation module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _INTREP_H_
#define _INTREP_H_

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

//Memoryposition classification
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType;

//Memory Meta Data structure
typedef struct {
	short sectionId;		// 
	memoryType sectionType; // todo:check the size!!!
	short fileId;			// 0:base asm, 1..0xffff: includes
	int line;				// 32 bits
} memoryMetaData_t;

//this holds all of the meta data for each memory location
extern memoryMetaData_t memoryMeta[MAXMEMORY];

//#define OBSOLATE_ENABLED


//Actual section type
extern memoryType sectionType;
extern int sectionId;

//Actual section name
extern char section[MAXSECTIONNAME];

//holds memory content
extern char memory[MAXMEMORY];

#ifdef OBSOLATE_ENABLED
//Holds memory type for all the location (to be easier)
extern memoryType memoryTypes[MAXMEMORY];

//holds the input source line numbers
extern int lines[MAXMEMORY];
#endif


//actual address setter, keep the max value for later use
void setAddress(int a);

//checks the earlier maximum for addresses
void chkAddress(int a);

//to add code or data to the memory
void addMemory(int data);

//to get original source line
int getMemoryLineNo(int address);

//to get memory usage kind
int getMemoryType(int address);

//to get the memory section id
int getMemorySectionId(int address);
memoryMetaData_t* getMemoryMeta(int address);

#ifdef __cplusplus
}
#endif

#endif