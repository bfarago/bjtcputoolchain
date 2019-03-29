#pragma once
#include "Std_Types.h"
typedef uint8 SimData_t;
typedef uint16 SimAddress_t;
typedef enum {
	bd_Tristate,
	bd_Read,
	bd_Write,
	bd_max
} busDirection_t;

#define SIM_MAXMEMORYSIZE ((1<<12))
extern SimData_t Sim_Memory[SIM_MAXMEMORYSIZE];

Std_ReturnType Sim_Step(void);
void Sim_OnKeyDown(void);