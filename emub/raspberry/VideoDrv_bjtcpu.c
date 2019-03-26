#include "VideoDrv_bjtcpu.h"
#include "Pwm.h"
#include "PwmDrv.h"

//Vector list time mask for character's last row
#define END_OF_DATABUFFER (1<<31)

//Vector list length
#define SAMPLE_LEN (sizeof(g_buf)/sizeof(Pwm_DataBufferType) )
//Vector list
Pwm_DataBufferType g_buf[]=
{
	{ 0,  0,  1}, //0
	{ 1,  0,100}, 
	{ 15, 0,100}, 
	{ 15, 0,1| END_OF_DATABUFFER },
	{ 0,  0,  1 }, //1
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //2
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //3
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //4
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //5
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //6
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //7
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //8
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{ 0,  0,  1 }, //9
	{ 1,  0,100 },
	{ 15, 0,100 },
	{ 15, 0,1 | END_OF_DATABUFFER },
	{  0,  0,100}, //A
	{  1,  3,100},
	{  2,  6,100},
	{  3,  9,100},
	{  4, 12,100},
	{  5,  9,100},
	{  6,  6,100},
	{  2,  6,100},
	{  6,  6,100},
	{  7,  3,100},
	{  8,  0,100 | END_OF_DATABUFFER },
	{ 16,  0,100}, //B
	{ 16,  8,100},
	{ 16, 16,100},
	{ 24, 16,100},
	{ 31, 16,100},
	{ 31,  8,100},
	{ 31,  0,100},
	{ 24,  0,100},
	{ 16,  0,100 | END_OF_DATABUFFER }
};

Pwm_DataType g_data={ .len= SAMPLE_LEN, .pos=0, .buf=g_buf };
SimScreen_t SimScreen;
int SimChargen[256];
Pwm_NumberOfDataType g_SimPos = 0;
uint8 g_x = 0;
uint8 g_y = 0;

/**VideoDrv_Init
init screen memory, chargen table, call pwm hal.
*/
Std_ReturnType VideoDrv_BjtCpu_Init(void* p) {
	int ch = 0;
	int y,x;
	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			unsigned char code = 0xff;
			SimScreen.buf[x][y] = code;
		}
	}

	SimChargen[ch] = 0;
	for (int i = 0; i < 256; i++)
	{
		SimChargen[i] = 0;
	}
	for (int i = 0; i < SAMPLE_LEN; i++) {
		if (g_buf[i].time & END_OF_DATABUFFER) {
			ch++;
			SimChargen[ch] = i + 1;
		}
	}
	uint8 chi = SimScreen.buf[g_x][g_y];
	g_SimPos = SimChargen[chi];
	return Pwm_SetData(&g_data); 
}

void VideoDrv_BjtCpu_MainFunctionDiagnostics(void){}
void VideoDrv_BjtCpu_MainFunctionTest(void){}

/**VideoDrv_MainFunction
Cyclical main function
*/
void VideoDrv_BjtCpu_MainFunction(void){
	Pwm_WritePos(g_SimPos);
	g_SimPos++;
	if ((g_SimPos >= SAMPLE_LEN)
		||(g_buf[g_SimPos].time&END_OF_DATABUFFER))
	{
		g_x++;
		if (g_x>15){
			g_x=0;
			g_y++;
			if (g_y>15){
				g_y=0;
			}
		}
		uint8 ch = SimScreen.buf[g_x][g_y];
		g_SimPos = SimChargen[ch]; //vector list start position
		Pwm_WriteOfs(g_x, g_y); //offset, char pos on screen
	}
}