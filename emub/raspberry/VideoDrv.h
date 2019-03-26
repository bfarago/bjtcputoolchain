#ifndef VIDEODRV_H
#define VIDEODRV_H
#include "Std_Types.h"
extern uint8 VideoDrv_Enable_ScopeOut;

Std_ReturnType VideoDrv_Init(void*);
Std_ReturnType VideoDrv_DeInit();
void VideoDrv_MainFunctionDiagnostics(void);
void VideoDrv_MainFunctionTest(void);
void VideoDrv_MainFunction(void);

#endif