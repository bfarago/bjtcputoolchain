#pragma once

#include "Std_Types.h"


typedef enum {
	Key_Esc,
	Key_Space,
	Key_Up,
	Key_Dn,
	Key_Left,
	Key_Right,
	Key_Last
}Keyboard_scan_e;

Std_ReturnType Keyboard_Check(Keyboard_scan_e scan);

Std_ReturnType Keyboard_Init(void);
Std_ReturnType Keyboard_DeInit(void);
void Keyboard_ReadScan(void);
uint8 Keyboard_IsEscapePressed(void);
