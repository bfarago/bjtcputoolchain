#include <stdio.h>
#include <memory.h>
#include <unistd.h>

#define delay(ms) usleep( (ms)*1000)

#include "Std_Types.h"
#include "Pwm.h"
#include "VideoDrv_bjtcpu.h"

int main (void)
{
	Std_ReturnType ret;
	printf ("Raspberry Pi PWM\n") ;
	uint16 counter=0;
	ret= Pwm_Init(0);
	if (ret){
		printf("PWM init error\n");
		return 1;
	}
	ret= VideoDrv_Init(0);
	if (ret){
		printf("VideoDrv_bjtcpu init error\n");
		return 1;
	}
	for(;;){
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
	Pwm_DeInit();
	
	
  return 0 ;
}