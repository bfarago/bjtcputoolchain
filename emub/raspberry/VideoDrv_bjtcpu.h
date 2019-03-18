#ifndef VIDEODRV_BJTCPU_H
#define VIDEODRV_BJTCPU_H
#include "Std_Types.h"

typedef struct {
	uint8 x, y;
	uint8 ch0, ch1;
	uint8 buf[16][16];
} SimScreen_t;

Std_ReturnType VideoDrv_Init(void*);
void VideoDrv_MainFunctionDiagnostics(void);
void VideoDrv_MainFunctionTest(void);
void VideoDrv_MainFunction(void);

#endif