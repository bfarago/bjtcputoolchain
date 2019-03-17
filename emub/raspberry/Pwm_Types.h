#ifndef _PWM_TYPES_H_
#define _PWM_TYPES_H_

#include "Std_Types.h"

typedef enum {
	PWM_IDLE,
	PWM_BUSY
}Pwm_StatusType;

typedef uint16 Pwm_NumberOfDataType;
typedef struct {
	uint16 x;
	uint16 y;
	uint32 time;
}	Pwm_DataBufferType;

typedef struct{
	Pwm_NumberOfDataType len;
	Pwm_NumberOfDataType pos;
	Pwm_DataBufferType* buf;
}Pwm_DataType;

typedef struct{
	void* hwInfo; ////Pwm_HwInfoType* hwInfo;
	Pwm_DataType* data;
/*	uint8 gpioX;
	uint8 gpioY;
	uint16 initValueX;
	uint16 initValueY;
	uint16 clockId;
	uint16 range;*/
} Pwm_ConfigType;

#endif
