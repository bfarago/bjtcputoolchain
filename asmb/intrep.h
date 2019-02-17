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

//Memoryposition classification
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType;

//Actual section type
extern memoryType sectionType;

//Actual section name
extern char section[MAXSECTIONNAME];

//Holds memory type for all the location (to be easier)
extern memoryType memoryTypes[MAXMEMORY];

//holds memory content
extern char memory[MAXMEMORY];

//holds the input source line numbers
extern int lines[MAXMEMORY];

//actual address setter, keep the max value for later use
void setAddress(int a);

//checks the earlier maximum for addresses
void chkAddress(int a);

//to add code or data to the memory
void addMemory(int data);

#endif