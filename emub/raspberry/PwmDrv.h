#ifndef _PWMDRV_H_
#define _PWMDRV_H_

#include "Pwm_Types.h"

typedef struct{
	Pwm_StatusType status;
	uint8 initialized;
}Pwm_HwInfoType;


typedef struct{
	Pwm_HwInfoType hwInfo;
	Pwm_DataType* data; //char shape, vector list?
	uint8 ofsX;
	uint8 ofsY;
}PwmDrv_InfoType;


typedef struct{
	uint8 gpioX;
	uint8 gpioY;
	uint16 initValueX;
	uint16 initValueY;
	uint16 clockId;
	uint16 range;
}PwmDrv_ConfigType;


Std_ReturnType
PwmDrvRPI_Init();


Std_ReturnType
PwmDrvRPI_DeInit();

Std_ReturnType
PwmDrvRPI_WritePos(Pwm_NumberOfDataType pos);

Std_ReturnType
PwmDrvRPI_SetData(const Pwm_DataType* pd);

Std_ReturnType
PwmDrvRPI_WriteOfs(uint8 x, uint8 y);
#endif