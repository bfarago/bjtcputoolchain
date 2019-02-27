/** @file gen_bin.h
*
* @brief Generator for the bin output.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _GEN_BIN_H_
#define _GEN_BIN_H_

#include "util.h"

Std_ReturnType gen_bin(asmb_config_t *asmb_config, int maxaddress, char* memory);
Std_ReturnType gen_map(asmb_config_t *asmb_config);

#endif
