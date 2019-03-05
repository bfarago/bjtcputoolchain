/** @file main.c
*
* @brief Asmb assembler main file
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "Version.h"
#include "util.h"

#include "intrep.h"
#include "scanner.h"
#include "grammar.h"

#include "gen_fpga.h"
#include "gen_bin.h"
#include "gen_lst.h"


void relocation() {
	size_t i;
	size_t n = getRelocs();
	for (i = 0; i < n; i++) {
		const char* name;
		int addr;
		relocType_en relocType;
		getReloc((int)i, &name, &addr, &relocType);
		int ix = searchSymbol(name);
		if (ix >= 0) {
			int v = getSymbol(name);
			int c;
			//todo: feature: reloctype can encode operation too
			switch (relocType) {
			case RT_OP4_12:
				c = (memory[addr + 3] << 8) | (memory[addr + 2]) << 4 | (memory[addr + 1]);
				v += c; //operation
				memory[addr + 3] = (v >> 8) & 0xf;
				memory[addr + 2] = (v >> 4) & 0xf;
				memory[addr + 1] = v & 0xf;
				Debug("rel", "%s relocated %x:[op%03x]", name, addr, v & 0xfff);
				break;
			case RT_OP4_4:
				c = memory[addr + 1] & 0x0f;
				v += c;
				memory[addr + 1] = v & 0xf;
				Debug("rel", "%s relocated %x:[op%x]", name, addr, v & 0xf);
				break;
			default:
				break;
			}
		}
		else {
			//not found, external
			Debug("ext", "extern %s relocate %x", name, addr);
		}
	}
}

//entry point of the asmb
int main(int argc, char** argv)
{
	FILE* fin;
	printf("%s %s\n", VersionName, VersionNumber);
	InitScanner();

	// Setup configurations (filename, switches, etc)
	asmb_config_t asmb_config;
	InitConfig(&asmb_config);
	ParseCommandLine(argc, argv, &asmb_config);
	if (asmb_config.fname_in) {
		fin = fopen(asmb_config.fname_in, "r"); // input file specified, open it
		if (!fin) {
			Failure("File not found: %s", asmb_config.fname_in);
			return -1;
		}
		else {
			yyin = fin;
		}
	}else{
		// pipe was used, there was no input filename specified
		fin= stdin;
	}

	// Start parsing, Pass#1
	parseFile(yyin);

	// End of Pass#1, close input file, if there was opened one
	if (asmb_config.fname_in) {
		fclose(fin);
	}

	// Start Relocation, Pass#2
	relocation();

	// Write out Memory dump (compiled binary)
	gen_bin(&asmb_config, maxaddress, memory);
	gen_map(&asmb_config);
	// Write out FPGA coe output (you can import as ROM init values)
	gen_coe(&asmb_config, maxaddress, memory);

	// Write out FPGA  verilog output (generates a ROM memory module)
	gen_verilog(&asmb_config, maxaddress, memory);

	// Write out list file
	gen_lst(&asmb_config, maxaddress, memory);
	return 0;
}
