/** @file gen_lst.h
*
* @brief Generator for the lst output.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _GEN_LST_H_
#define _GEN_LST_H_

#include "util.h"

Std_ReturnType gen_lst(asmb_config_t *asmb_config, int maxaddress, char* memory);

#endif