#include "Sim.h"
#include "Keyboard.h"
#include "VideoDrv_fb.h"
#include <stdlib.h>

typedef struct {
	unsigned char x, y;
	char ch0, ch1;
	unsigned char buf[16][16];
	//TCHAR fakeChar;
} SimScreen_t;

//SimScreen global singleton. Todo: move to class if we wana run multiple instances...
SimScreen_t SimScreen;

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

uint32 Sim_ClockCount=0u;
double Sim_Time;
uint32 Sim_CpuHz = 1500000u;
uint16 Sim_Pc = 0u;
uint8 Sim_Acc = 0u;
uint8 Sim_Carry = 0u;
SimAddress_t Sim_Address=0;
SimData_t Sim_Memory[SIM_MAXMEMORYSIZE];

//uint8 VideoDrv_ScreenBuf[16][16];
inline Std_ReturnType Sim_OnScreenLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data) {
	Std_ReturnType processed = E_OK;
	if (bd_Write == dir) {
		switch (addr) {
		case ADDR_SCREEN_X: SimScreen.x = *data; break;
		case ADDR_SCREEN_Y: SimScreen.y = *data; break;
		case ADDR_SCREEN_CH0: SimScreen.ch0 = *data; break;
		case ADDR_SCREEN_CH1: SimScreen.ch1 = *data;
			;
		default: processed = E_NOT_OK;	break;
		}
		//activate shadow buffer write
		switch (addr) {
		case ADDR_SCREEN_CH0:
		{
			unsigned char code = (SimScreen.ch1 << 4) | SimScreen.ch0;
			SimScreen.buf[SimScreen.x][SimScreen.y] = code;
			VideoDrv_ScreenBuf[SimScreen.y][SimScreen.x] = code;
			//SimScreen.fakeChar = SimCode2Ascii[code];
		}
		break;
		}
	}
	if (bd_Read == dir) {
		switch (addr) {
		case ADDR_SCREEN_X: *data = SimScreen.x; break;
		case ADDR_SCREEN_Y: *data = SimScreen.y; break;
		case ADDR_SCREEN_CH0: *data = SimScreen.ch0; break;
		case ADDR_SCREEN_CH1: *data = SimScreen.ch1; break;
		default: processed = E_NOT_OK;	break;
		}
	}
	return processed;
}

inline Std_ReturnType Sim_OnPerifLoadStore(SimAddress_t addr, busDirection_t dir, SimData_t* data, SimData_t* mem) {
	SimAddress_t masked = (addr & 0xffc);
	if (ADDR_SCREEN_X == masked) {
		return Sim_OnScreenLoadStore(addr, dir, data);
	}
	else if (ADDR_UART_H == masked) {
	//	return OnUartLoadStore(addr, dir, data);
	}
	else {
		if (bd_Read == dir) {
			Std_ReturnType processed = E_OK;
			switch (addr) {
			case ADDR_RND: *mem = *data = rand() & 0x0f; break;
			case ADDR_ARR: *data = Sim_Memory[ADDR_ARR]; *mem = 0; break;
			case ADDR_KEY0: *data = Sim_Memory[ADDR_KEY0];  *mem = 0; break;
			case ADDR_KEY1: *data = Sim_Memory[ADDR_KEY1];  *mem = 0; break;
			default:processed = E_NOT_OK;	break;
			}
			if (processed) return E_OK;
		}
		else if (bd_Write == dir) {
			// not writable
		}
	}
	return E_NOT_OK;
}

inline void Sim_AddressBusDrive(SimAddress_t a) {
	Sim_Address = a;
}

SimData_t Sim_DataBusRead() {
	if (Sim_Address > SIM_MAXMEMORYSIZE) {
		Sim_Address = SIM_MAXMEMORYSIZE; //Invalid address
	}
	//m_HeatRead[m_CpuSnapshot_p->addr] = 0xff;
	if (Sim_Address >= 0xc00) {
		SimData_t data = 0;
		if (Sim_OnPerifLoadStore(Sim_Address, bd_Read, &data, &Sim_Memory[Sim_Address])) {
			return data;
		}
	}
	return  Sim_Memory[Sim_Address];
}

void DataBusDrive(SimData_t data) {
	if (Sim_Address > SIM_MAXMEMORYSIZE) {
		Sim_Address = SIM_MAXMEMORYSIZE; //Invalid address
	}
	//m_HeatWrite[m_CpuSnapshot_p->addr] = 0xff;
	if (Sim_Address >= 0xc00) {
		if (Sim_OnPerifLoadStore(Sim_Address, bd_Write, &data, &Sim_Memory[Sim_Address])) {
			return;
		}
	}

	Sim_Memory[Sim_Address] = data;
}

//AluSetAccumulator: simulation of the cpu's ALU operation, when Accumulator register will be written.
inline void AluSetAccumulator(SimData_t data) {
	Sim_Acc = data; //can be wider than 4 bit
	Sim_Carry = (Sim_Acc >= 0x10) ? 1 : 0; //store carry
	Sim_Acc &= 0x0f; //remove carry
}

inline void Sim_SetPc(SimAddress_t a) {
	Sim_Pc = a;
}

Std_ReturnType Sim_Step(void)
{

	Sim_ClockCount++;
	Sim_Time += 1 / (double)Sim_CpuHz;
	
	/*
	if (!(m_ClockCount%SIM_HEATMAP_PERIOD)) {
		ProcessHeatMaps();
	}
	*/
	/*
	if (Sim_Pc < 0) { //Not possible, only for development.
		Sim_Pc = 0; //Invalid address in PC
		return E_NOT_OK;
	}
	*/
	if (Sim_Pc >= SIM_MAXMEMORYSIZE) {
		Sim_Pc = 0; //Invalid address in PC
		return E_NOT_OK;
	}
	//m_HeatPc[Sim_Pc] = 0xFf;
	
	SimData_t op = Sim_Memory[Sim_Pc];
	SimData_t data = Sim_Memory[Sim_Pc + 1];
	SimAddress_t imm = data | (Sim_Memory[Sim_Pc + 2] << 4) | (Sim_Memory[Sim_Pc + 3] << 8);
	SimData_t acc = 0;

	acc = Sim_Acc;
	Sim_Pc += (op) ? 4 : 2;
	if ((2 <= op) && (op <= 8))//lda..rrm
	{
		Sim_AddressBusDrive(imm);
		data = Sim_DataBusRead();
	}
	switch (op)
	{
#undef TOK
#define TOK(x, xop) case xop:
		TOK(T_mvi, 0) Sim_Acc = data; break;
		TOK(T_lda, 2) AluSetAccumulator(data);  break;
		TOK(T_ad0, 3) AluSetAccumulator(acc + data); break;
		TOK(T_ad1, 4) AluSetAccumulator(acc + data + 1); break;
		TOK(T_adc, 5) AluSetAccumulator(acc + data + Sim_Carry); break;
		TOK(T_nand, 6) AluSetAccumulator(~(acc & data)); break;
		TOK(T_nor, 7) AluSetAccumulator(~(acc | data));	break;
		TOK(T_rrm, 8) AluSetAccumulator(data >> 1); break;
		TOK(T_jmp, 9) Sim_SetPc(imm);	break;
		TOK(T_jc, 10) if (Sim_Carry) Sim_SetPc(imm);	break;
		TOK(T_jnc, 11) if (!Sim_Carry) Sim_SetPc(imm); break;
		TOK(T_jz, 12)  if (!Sim_Acc) Sim_SetPc(imm); break;
		TOK(T_jnz, 13) if (Sim_Acc) Sim_SetPc(imm); break;
		TOK(T_jm, 14) if (Sim_Acc & 0x8) Sim_SetPc(imm); break;
		TOK(T_jp, 15) if (!(Sim_Acc & 0x8)) Sim_SetPc(imm); break;
	}
	if (1 == op)//sta
	{
		Sim_AddressBusDrive(imm);
		DataBusDrive(Sim_Acc);
	}
	// if (m_Break[Sim_Pc]) return FALSE;
	return E_OK;
}

void Sim_OnKeyDown(void) //UINT nChar, UINT nRepCnt, UINT nFlags
{
	uint8 hit = 0;
	if (Keyboard_Check(Key_Up)!= E_NOT_OK) {
		hit = 1;
		Sim_Memory[ADDR_ARR] = ka_Fire;
	}
	if (hit) return;
	if (Keyboard_Check(Key_Space) != E_NOT_OK) {
		unsigned char key = 0x11; //Todo: conversion?
		Sim_Memory[ADDR_KEY1] = (key >> 4) & 0x0f;
		Sim_Memory[ADDR_KEY0] = key & 0x0f;
	}
}

