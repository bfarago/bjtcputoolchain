#include "VideoDrv_bjtcpu.h"
#include "Pwm.h"
#include "PwmDrv.h"

#define SAMPLE_LEN (sizeof(g_buf)/sizeof(Pwm_DataBufferType) )
Pwm_DataBufferType g_buf[]=
{
	{  0,  0,100},
	{  1,  3,100},
	{  2,  6,100},
	{  3,  9,100},
	{  4,  12,100},
	{  5,  9,100},
	{  6,  6,100},
	{  2,  6,100},
	{  6,  6,100},
	{  7,  3,100},
	{  8,  0,100},
	{  8,  0,1}
/*	
	{ 16,  0,100},
	{ 16,  8,100},
	{ 16, 16,100},
	{ 24, 16,100},
	{ 31, 16,100},
	{ 31,  8,100},
	{ 31,  0,100},
	{ 24,  0,100},
	{ 16,  0,100}*/
};

Pwm_DataType g_data={ .len= SAMPLE_LEN, .pos=0, .buf=g_buf };

Std_ReturnType VideoDrv_Init(void* p){
	return Pwm_SetData(&g_data); 
}

void VideoDrv_MainFunctionDiagnostics(void){}
void VideoDrv_MainFunctionTest(void){}

void VideoDrv_MainFunction(void){
	static Pwm_NumberOfDataType pos=0;
	static uint8 g_x=0;
	static uint8 g_y=0;
	Pwm_WritePos(pos);
	pos++;
	if (pos >= SAMPLE_LEN){
		pos=0;
		g_x++;
		if (g_x>15){
			g_x=0;
			g_y++;
			if (g_y>15){
				g_y=0;
			}
		}
		Pwm_WriteOfs(g_x, g_y);
	}
	
}