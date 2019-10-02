#pragma once

#define MAX_ERROR_LEN (255)
#define MAXSYMBOLENAME (255)

typedef unsigned int SimAddress_t;
typedef unsigned char SimData_t;


//Memory Type: a memory location can be code or data type.
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType_t;

//Internal Symbol types
typedef enum {
	ST_Unknown,
	ST_Label,
	ST_EQU,
	ST_DB,
	ST_JMP,
	ST_LOAD,
	ST_STORE
} SType_e;

typedef unsigned char SContexts_t; // bitmask : 1<<SType_e

//Bus direction type: an actual state of a bus can be read,write or tristate.
typedef enum {
	bd_Tristate,
	bd_Read,
	bd_Write,
	bd_max
} busDirection_t;

//Cpu state type: Fetch, Load, Decode, Store states of the Cpu.
typedef enum {
	cs_Fetch = 'F',
	cs_Load = 'L',
	cs_Store = 'S',
	cs_Decode = 'D'
} cpuState_t;

//Key arrow type: simulated arrow keys can be in state of the directions and fire, it is like a retro joystick.
typedef enum {
	ka_Nothing,
	ka_Left,
	ka_Right,
	ka_Up,
	ka_Down,
	ka_Fire = 11, //todo: document arr periph, how it is works, meaning of the values, etc...
	ka_max
} keyArrow_t;

//Cpu simulator internal states: used internally, it describes every clock phase and states. Detailed than the cpuState_t.
typedef enum {
	ss_Halt = 0,
	ss_FetchOp,		//Fetch Operation/Instruction from actual PC
	ss_FetchImm0,	//Fetch Immediate from actual PC+n
	ss_FetchImm1,	//Fetch Immediate from actual PC+n
	ss_FetchImm2,	//Fetch Immediate from actual PC+n
	ss_Decode,		//Setup internal states
	ss_Load,		//Load Acc from memory pointed by Address register
	ss_Alu,			//Do the math.
	ss_Store,		//Store Acc to memory pointed by Address register
} SimState_t;

//tDbgFileSymbol: Symbol block format in dbg file.
typedef struct {
	unsigned int value;
	unsigned int lineno;
	unsigned short fileId;
	unsigned char memtype;
	unsigned char sectionid;
	unsigned char symtype;
	unsigned char symcontexts;
	unsigned char len;
	unsigned char name[MAXSYMBOLENAME];
}tDbgFileSymbol;

//DBG file Memory Meta Data structure
typedef struct {
	short sectionId;		// 
	memoryType_t sectionType; // todo:check the size!!!
	short fileId;			// 0:base asm, 1..0xffff: includes
	int line;				// 32 bits
} memoryMetaData_t;

//DBG file error record
typedef struct {
	unsigned int lineNr;
	unsigned short fileId;
	unsigned short errorCode;
	char errorText[MAX_ERROR_LEN];
}tErrorRecord;

//CPU peripheral addresses
enum {
	ADDR_ARR = 0xc00,
	ADDR_RND,
	ADDR_KEY0,
	ADDR_KEY1,
	ADDR_SCREEN_X,	//mask:0xc04
	ADDR_SCREEN_Y,
	ADDR_SCREEN_CH1, //High
	ADDR_SCREEN_CH0, //Low
	ADDR_UART_H,   // mask:0xc08
	ADDR_UART_L,
	ADDR_PERIPH_MAX
};
/*
arr		equ 3072
rnd		equ 3073
key0	equ 3074
key1	equ 3075
cord_x	equ 3076
cord_y	equ 3077
ch_h	equ 3078
ch_l	equ 3079
*/

//tDbgFileBlockId enum. IDs for the Dbg file blocks.
typedef enum {
	DF_VERSION,
	DF_NAME,
	DF_FNAME_BIN,
	DF_TIME_BIN,
	DF_FNAME_ASM,
	DF_TIME_ASM,
	DF_SYM,
	DF_LINES,//obsolate
	DF_MEMTYPES, //obsolate
	DF_SECTIONNAME,
	DF_MEMORYMETA,
	DF_ERROR
} tDbgFileBlockId;

typedef enum {
	E_OK,
	E_NOT_OK
}Std_ReturnType;

#ifdef _UNICODE

#ifndef TCHAR
#include <wtypes.h>
#include <tchar.h>
#endif

#else

#ifndef TCHAR
#define TCHAR char
#define _T
#endif

#endif
