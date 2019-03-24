#ifndef VIDEODRV_FB_H
#define VIDEODRV_FB_H
#include "Std_Types.h"

Std_ReturnType VideoDrv_Fb_Init(void*);
void VideoDrv_Fb_MainFunctionDiagnostics(void);
void VideoDrv_Fb_MainFunctionTest(void);
void VideoDrv_Fb_MainFunction(void);


void VideoDrv_Fb_PutPixel_8(int x, int y, int c);

#endif