/** @file gen_fpga.c
*
* @brief Generator implementation for fpga output files.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "gen_fpga.h"

Std_ReturnType gen_coe(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	FILE *f;
	int i;
	Std_ReturnType ret = E_OK;
	if (asmb_config->fname_out_coe) {
		f = fopen(asmb_config->fname_out_coe, "w+");
		if (!f) {
			Failure("Unable to create coe output file:%s", asmb_config->fname_out_coe);
			return E_NOT_OK;
		}
		fprintf(f, "memory_initialization_radix=16;\n");
		fprintf(f, "memory_initialization_vector=");
		for (i = 0; i < maxaddress; i++) {
			if (i) fprintf(f, ",");
			fprintf(f, "%02x", memory[i]);
		}
		fprintf(f, ";\n");
		fclose(f);
	}
	return ret;
}

Std_ReturnType gen_verilog(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	FILE *f;
	int i;
	Std_ReturnType ret = E_OK;
	if (asmb_config->fname_out_verilog) {
		f = fopen(asmb_config->fname_out_verilog, "w+");
		if (!f) {
			Failure("Unable to create .v output file:%s", asmb_config->fname_out_verilog);
			return E_NOT_OK;
		}
		fprintf(f, "module rom_content(\n");
		fprintf(f, " i_clk, i_read, i_counter, o_data\n");
		fprintf(f, ")\n");
		fprintf(f, "input wire i_clk;\n");
		fprintf(f, "input wire i_read;\n");
		fprintf(f, "input reg [8:0] i_counter;\n");
		fprintf(f, "output reg [31:0] o_data;\n");
		fprintf(f, "always@(posedge i_clk)\n");
		fprintf(f, " case(init_counter)\n");
		for (i = 0; i < maxaddress; i++) {
			if (0 == (i & 7)) fprintf(f, "  9'h%02x: o_data= 32'h", i / 8);
			fprintf(f, "%x", memory[i] & 0xf);
			if (7 == (i & 7)) fprintf(f, "; \n");
		}
		
		for (i = (7-maxaddress) & 7; i >= 0; i--) {
			fprintf(f, "0");
		}
		fprintf(f, ";\n endcase\n");
		fprintf(f, "endmodule\n");
		fclose(f);
	}
	return ret;
}