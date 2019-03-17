#ifndef _PWM_H_
#define _PWM_H_

#include "Pwm_Types.h"

Std_ReturnType 
Pwm_Init(const Pwm_ConfigType* c);

Std_ReturnType
Pwm_DeInit(void);

Pwm_StatusType
Pwm_GetStatus(void);

Std_ReturnType
Pwm_SetData(const Pwm_DataType* pd); 

Std_ReturnType
Pwm_WritePos(Pwm_NumberOfDataType pos);

Std_ReturnType
Pwm_WriteOfs(uint8 x, uint8 y);

#endif