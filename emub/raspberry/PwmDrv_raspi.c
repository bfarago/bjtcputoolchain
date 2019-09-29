#define _BSD_SOURCE
#include "PwmDrv.h"
// #include <memory.h>
#include <unistd.h>
#include <time.h>
#include "Det.h"
#include <wiringPi.h>

PwmDrv_ConfigType PwmDrv_Default_Config={
	/*.gpioX= */1u, //GPIO18, PWM1, pin12, Wiring Pi pin 1 ??
	/*.gpioY= */23u, //GPIO13, PWM2, pin33, Wiring Pi pin 23 ??
	/*.initValueX=*/ 0u,
	/*.initValueY=*/ 0u,
	/*,clockId=*/2u,
	/*,range=*/256u
};

const PwmDrv_ConfigType* PwmDrv_Cfg= &PwmDrv_Default_Config;
PwmDrv_InfoType PwmDrv_Default_Info;
PwmDrv_InfoType* PwmDrv_Info= &PwmDrv_Default_Info;

Std_ReturnType
PwmDrvRPI_Init(){
	int ret=E_OK;
	ret= wiringPiSetup();
	if (-1==ret){
		Det_ReportRuntimeError(2,0, 2,2);
		return E_NOT_OK;
	}
	//wiringPiMode ?? WPI_MODE_PHYS, WPI_MODE_GPIO, WPI_MODE_PINS
	Pwm_StatusType status= PwmDrvRPI_GetStatus();
	if (PWM_IDLE != status){
		Det_ReportRuntimeError(2,0,2,1);
	}	
	pinMode(PwmDrv_Cfg->gpioX, PWM_OUTPUT);
	pinMode(PwmDrv_Cfg->gpioY, PWM_OUTPUT);
	pwmSetClock(PwmDrv_Cfg->clockId);
	pwmSetRange (PwmDrv_Cfg->range) ;
	pwmWrite (PwmDrv_Cfg->gpioX, PwmDrv_Cfg->initValueX);
	pwmWrite (PwmDrv_Cfg->gpioX, PwmDrv_Cfg->initValueY);
	//pwmSetMode(PWM_MODE_MS); //PWM_MODE_BAL
	pwmSetMode(PWM_MODE_BAL);
	
	// printf("gpios:%u, %u.\nClock:%u\nRange:%u\n",
	//	PwmDrv_Cfg->gpioX, PwmDrv_Cfg->gpioY, PwmDrv_Cfg->clockId, PwmDrv_Cfg->range);
	
	if (1 < 0){
		Det_ReportRuntimeError(2,0, 2,1);
		return E_NOT_OK;
	}
	return ret;
}

Std_ReturnType
PwmDrvRPI_DeInit(void){
	Std_ReturnType ret= E_OK;
	//return to default/init state
	pwmWrite (PwmDrv_Cfg->gpioX, PwmDrv_Cfg->initValueX);
	pwmWrite (PwmDrv_Cfg->gpioY, PwmDrv_Cfg->initValueY);
	return ret;
}

Pwm_StatusType
PwmDrvRPI_GetStatus(void){
	return PWM_IDLE;
}

Std_ReturnType
PwmDrvRPI_Write(uint16 x, uint16 y){
	Std_ReturnType ret;

	pwmWrite (PwmDrv_Cfg->gpioX, x + PwmDrv_Info->ofsX);
	pwmWrite (PwmDrv_Cfg->gpioY, y + PwmDrv_Info->ofsY);
	
	//printf("write:%d, %d.\n", x,y);
	if (-1 == 0){
		Det_ReportRuntimeError(2,0, 1,1);
		ret= E_NOT_OK;
	}else{
		ret= E_OK;
	}

	return ret;
}

Std_ReturnType
PwmDrvRPI_WritePos(Pwm_NumberOfDataType pos){
	Std_ReturnType ret;
	if (!PwmDrv_Info) return E_NOT_OK;
	if (!(PwmDrv_Info->data)) return E_NOT_OK;
	
	if (pos < PwmDrv_Info->data->len){
		Pwm_DataBufferType* pData=&PwmDrv_Info->data->buf[pos];
		//printf("pos:%d.\n", pos);
		PwmDrvRPI_Write(pData->x, pData->y);
		if (pData->time > 2)
			usleep( pData->time >>6);
		ret= E_OK;
	}
	else
	{
		Det_ReportRuntimeError(2,0, 1,1);
		ret= E_NOT_OK;
	}

	return ret;
}

Std_ReturnType
PwmDrvRPI_SetData(const Pwm_DataType* pd){
	Std_ReturnType ret=E_OK;
	if (!PwmDrv_Info) return E_NOT_OK;
	//if (!PwmDrv_Info->data) return E_NOT_OK;
	PwmDrv_Info->data = (Pwm_DataType*)pd;
	//printf("len:%d.\n", pd->len);
	
	return ret;
}

Std_ReturnType
PwmDrvRPI_WriteOfs(uint8 x, uint8 y){
	PwmDrv_Info->ofsX=x*16;
	PwmDrv_Info->ofsY=y*16;
	return E_OK;
}