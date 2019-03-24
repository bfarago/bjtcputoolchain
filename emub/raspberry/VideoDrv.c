#include "VideoDrv.h"
#include "VideoDrv_bjtcpu.h"
#include "VideoDrv_fb.h"

uint8 VideoDrv_Enable_ScopeOut=0;
uint8 VideoDrv_Enable_FbOut=1;

/**VideoDrv_Init
init screen memory, chargen table, call pwm hal.
*/
Std_ReturnType VideoDrv_Init(void* p) {
	Std_ReturnType ret=E_OK;
	if (VideoDrv_Enable_ScopeOut) ret= VideoDrv_BjtCpu_Init(p);
	if (VideoDrv_Enable_FbOut) ret= VideoDrv_Fb_Init(p);
	return ret;
}
Std_ReturnType VideoDrv_DeInit(){
	Std_ReturnType ret=E_OK;
	//if (VideoDrv_Enable_ScopeOut) ret= VideoDrv_BjtCpu_Init(p);
	if (VideoDrv_Enable_FbOut) ret= VideoDrv_Fb_DeInit();
	return ret;
}
void VideoDrv_MainFunctionDiagnostics(void){
	if (VideoDrv_Enable_ScopeOut) VideoDrv_BjtCpu_MainFunctionDiagnostics();
	if (VideoDrv_Enable_FbOut) VideoDrv_Fb_MainFunctionDiagnostics();
}
void VideoDrv_MainFunctionTest(void){
	if (VideoDrv_Enable_ScopeOut) VideoDrv_BjtCpu_MainFunctionTest();
	if (VideoDrv_Enable_FbOut) VideoDrv_Fb_MainFunctionTest();
}

/**VideoDrv_MainFunction
Cyclical main function
*/
void VideoDrv_MainFunction(void){
	if (VideoDrv_Enable_ScopeOut) VideoDrv_BjtCpu_MainFunction();
	if (VideoDrv_Enable_FbOut) VideoDrv_Fb_MainFunction();
}
