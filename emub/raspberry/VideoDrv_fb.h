#ifndef VIDEODRV_FB_H
#define VIDEODRV_FB_H
#include "Std_Types.h"

Std_ReturnType VideoDrv_Fb_Init(void*);
void VideoDrv_Fb_MainFunctionDiagnostics(void);
void VideoDrv_Fb_MainFunctionTest(void);
void VideoDrv_Fb_MainFunction(void);


void VideoDrv_Fb_PutPixel_8(int x, int y, int c);
void VideoDrv_Fb_Copy_8(char* src, int sx, int sy, int sw, int sh, int sl, int dx, int dy);
void VideoDrv_Fb_DrawLine_8(int x0, int y0, int x1, int y1, int c);

#endif