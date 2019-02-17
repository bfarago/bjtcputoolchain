/** @file gen_fpga.h
*
* @brief Generator for the fpga outputs.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _GEN_FPGA_H_
#define _GEN_FPGA_H_

#include "util.h"

Std_ReturnType gen_coe(asmb_config_t *asmb_config, int maxaddress, char* memory);
Std_ReturnType gen_verilog(asmb_config_t *asmb_config, int maxaddress, char* memory);

#endif
