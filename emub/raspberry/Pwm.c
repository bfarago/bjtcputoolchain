#include "Pwm.h"
#include "PwmDrv.h"
#include "Det.h"

Pwm_HwInfoType Pwm_Default_HwInfo;  //={ };

const Pwm_ConfigType Pwm_Default_Config =
{
	/*.hwInfo=(void*) */ &Pwm_Default_HwInfo,
	0, //data
	0, //x
	0  //y
};

const Pwm_ConfigType* Pwm_Config=0;

#define getHwInfoPtr() ((Pwm_HwInfoType*)(Pwm_Config->hwInfo))

Std_ReturnType 
Pwm_Init(const Pwm_ConfigType* c){
	Std_ReturnType ret;
	if (c){
		Pwm_Config=c;
	}else{
		Pwm_Config=&Pwm_Default_Config;
	}
	getHwInfoPtr()->status=PWM_IDLE;
	ret= PwmDrvRPI_Init();
	return ret;
}


Std_ReturnType
Pwm_DeInit(void){
	Std_ReturnType ret;
	ret= PwmDrvRPI_DeInit();
	
	Pwm_Config=0;
	return ret;
}


Pwm_StatusType
Pwm_GetStatus(void){
	return getHwInfoPtr()->status;
}

Std_ReturnType
Pwm_SetData(const Pwm_DataType* pd){
	Std_ReturnType ret=E_OK;
	
	
	if (!Pwm_Config) {
		Det_ReportRuntimeError(1,0, 1,1);
		return E_NOT_OK;
	}
	PwmDrvRPI_SetData(pd);
	//Pwm_Config->data=pd;
	return ret;
}

Std_ReturnType
Pwm_WritePos(Pwm_NumberOfDataType pos){
	return PwmDrvRPI_WritePos(pos);
}

Std_ReturnType
Pwm_WriteOfs(uint8 x, uint8 y){
	return PwmDrvRPI_WriteOfs(x,y);
}