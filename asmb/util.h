/** @file util.h
*
* @brief Utility functions and type declarations.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _UTIL_H
#define _UTIL_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//Eliminate warning in VisualStudio
#if defined(_WIN32) || defined(WIN32)
#define UTIL_STRDUP(x) _strdup(x)
#else
#define UTIL_STRDUP(x) strdup(x)
#define _fileno fileno
#endif

typedef enum {
	E_OK,
	E_NOT_OK
}Std_ReturnType;

#ifdef __cplusplus
extern "C" {
#endif
#include "scanner.h"

extern int address;
extern int maxaddress;

/*Typedef: relocType_en
*Grammar relocation enum.
*/
typedef enum {
	RT_OP4_12,
	RT_OP4_4,
	RT_MAX
}relocType_en;
extern relocType_en actualRelocType;

/*Typedef: GType
*Grammar minimal storage.
*/
typedef struct {
	int t;
	YYSTYPE s;
} GType_s;

typedef enum {
	ST_Unknown,
	ST_Label,
	ST_EQU,
	ST_DB,
	ST_JMP,
	ST_LOAD,
	ST_STORE
} SType_e;
typedef unsigned char SContexts_t;

void stackPush(int t, const YYSTYPE* s);

int parse_exp(int t, GType_s* res, SType_e* context);
/**
 * Function: Failure()
 * Usage: Failure("Out of memory!");
 * Reports an error and exits the program immediately.  You should not
 * need to call this since you should always try to continue parsing,
 * even after an error is encountered.  Some of the provided code calls
 * this in unrecoverable error situations (cannot allocate memory, etc.)
 * Failure accepts printf-style arguments in the message to be printed.
 */

void Failure(const char *format, ...);

/**
 * Macro: Assert()
 * Usage: Assert(num > 0);
 * This macro is designed to assert the truth of a necessary condition.
 * It tests the given expression, and if it evalutes true, nothing happens.
 * If it is false, it calls Failure to print a message and abort.
 * For example:  Assert(ptr != NULL)
 * will print something similar to the following if ptr is NULL:
 *   *** Failure: Assertion failed: hashtable.cc, line 55:
 *       ptr != NULL
 */ 

#define Assert(expr)  \
  ((expr) ? (void)0 : Failure("Assertion failed: %s, line %d:\n    %s", __FILE__, __LINE__, #expr))

/**
 * Function: Debug()
 * Usage: Debug("parser", "found ident %s\n", ident);
 * Print a message if we have turned debugging messages on for the given
 * key.  For example, the usage line shown above will only print a message
 * if the call is preceded by a call to SetDebugForKey("parser",true).
 * The function accepts printf arguments.  The provided main.cc parses
 * the command line to turn on debug flags. 
 */

void Debug(const char *key, const char *format, ...);

/**
 * Function: SetDebugForKey()
 * Usage: SetDebugForKey("scope", true);
 * Turn on debugging messages for the given key.  See PrintDebug
 * for an example. Can be called manually when desired and will
 * be called from the provided main for flags passed with -d.
 */

void SetDebugForKey(const char *key, bool val);

/**
 * Function: IsDebugOn()
 * Usage: if (IsDebugOn("scope")) ...
 * Return true/false based on whether this key is currently on
 * for debug printing.
 */

bool IsDebugOn(const char *key);

/**Command line parser sets this config
*/
typedef struct {
	const char* name_o; //means {name_o}.* files will be generated
	const char* fname_in;
	const char* fname_out_bin;
	const char* fname_out_lst;
	const char* fname_out_coe;
	const char* fname_out_verilog;
	const char* fname_err;
	unsigned char enable_fpga;
	unsigned char enable_lst;
	unsigned char enable_map;
	unsigned char enable_err;
	unsigned char enable_dbg;
}asmb_config_t;

/**
* Function: InitConfig
* Default config.
*/
void InitConfig(asmb_config_t* cfg);

/**
 * Function: ParseCommandLine
 * Turn on the debugging flags from the command line.  Verifies that
 * first argument is -d, and then interpret all the arguments that follow
 * as being flags to turn on.
 */
Std_ReturnType ParseCommandLine(int argc, char *argv[], asmb_config_t* cfg);

size_t SymbolLength();
const char* SymbolByIndex(size_t index);
int SymbolValueByIndex(size_t index);

int searchSymbol(const char *key);
void setSymbol(const char* name, int value, SType_e symbolType);
int getSymbol(const char* name, SType_e* pContext);
SType_e getSymbolType(int index);
SContexts_t getSymbolContexts(int index);
int getReloc(int index, const char**name, int* adr, relocType_en* rt, SType_e* context);
size_t getRelocs();

void setRelocType(relocType_en rt);
int yywrap();

int getMemorySectionNumbers();
const char* getMemorySectionName(int index);
void checkSection(char* secName);

int include_actual_get();
void include_add(const char* fname);
void include_eof();

int include_get_max();
const char* include_get(int index);

#ifdef __cplusplus
}
#endif

#endif
