#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include<signal.h>

#define delay(ms) usleep( (ms)*1000)

#include "Std_Types.h"
#include "Pwm.h"
#include "VideoDrv.h"

uint8 g_KeepRunning = 1;

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("received SIGINT\n");
		g_KeepRunning = 0;
	}
}

int main (void)
{
	Std_ReturnType ret;

	if (signal(SIGINT, sig_handler) == SIG_ERR) {
		printf("Signal handler init error\n");
	}
	uint16 counter=0;
	if (VideoDrv_Enable_ScopeOut) {
		printf("Raspberry Pi PWM\n");
		ret = Pwm_Init(0);
		if (ret) {
			printf("PWM init error\n");
			return 1;
		}
	}
	ret= VideoDrv_Init(0);
	if (ret){
		printf("VideoDrv_bjtcpu init error\n");
		return 1;
	}
	for(; g_KeepRunning;){
		if ((counter%5000) ==0)
		{
			VideoDrv_MainFunctionDiagnostics();
			counter=0;
		}
		
		if ((counter%5) ==0){
			VideoDrv_MainFunctionTest();
		}
		
		VideoDrv_MainFunction();
		counter++;
		//delay(1);
	}

	if (VideoDrv_Enable_ScopeOut) {
		Pwm_DeInit();
	}
	VideoDrv_DeInit();
  return 0 ;
}