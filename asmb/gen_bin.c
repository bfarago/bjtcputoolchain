/** @file gen_bin.c
*
* @brief Generator implementation for bin files.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "gen_bin.h"
#include "util.h"
#include "config.h"

Std_ReturnType gen_bin(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	FILE *f;
	char bfname[MAXFNAMELEN];
	const char* pfname=NULL;
	Std_ReturnType ret = E_OK;
	if (asmb_config->fname_out_bin) {
		pfname = asmb_config->fname_out_bin;
	}
	else {
		if (asmb_config->name_o) {
			snprintf(bfname, MAXFNAMELEN, "%s.bin", asmb_config->name_o);
			pfname = bfname;
		}
	}
	if (pfname) {
		// fopen_s(&f, asmb_config->fname_out_bin, "wb+");
		printf("Output binary:%s\n", pfname);
		f = fopen(pfname, "wb+");
		if (!f) {
			Failure("Unable to create output file:%s", pfname);
			return E_NOT_OK;
		}
		fwrite(memory, 1, maxaddress, f);
		fclose(f);
	}
	return ret;
}

Std_ReturnType gen_map(asmb_config_t *asmb_config)
{
	FILE *f;
	Std_ReturnType ret = E_OK;
	char b[MAXFNAMELEN];
	size_t len;
//	size_t i;
	if (!asmb_config->enable_map) return ret;
	if (asmb_config->name_o) {
		snprintf(b, MAXFNAMELEN, "%s.map", asmb_config->name_o);
		printf("Output memory map:%s\n", b);
		f = fopen(b, "wb+");
		if (!f) {
			Failure("Unable to create output file:%s", b);
			return E_NOT_OK;
		}
		len=SymbolLength();
		for (unsigned int i = 0; i < len; i++) {
			const char* s = SymbolByIndex(i);
			fprintf_s(f, "0x%03x %s\n", SymbolValueByIndex(i) , s);
		}
		/*
		fprintf(f, "\n");
		len= getRelocs();
		for (i = 0; i < len; i++) {
			const char* name;
			int addr;
			relocType_en relocType;
			getReloc((int)i, &name, &addr, &relocType);
			fprintf_s(f, "%s: 0x%03x ", name, addr);
			switch (relocType) {
			case RT_OP4_12:
				fprintf_s(f, "OP4_12\n", name, addr, relocType);
				break;
			case RT_OP4_4:
				fprintf_s(f, "OP4_4\n", name, addr, relocType);
				break;
			}
			
		}
		*/
		fclose(f);
	}
	return ret;
}