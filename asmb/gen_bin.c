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

Std_ReturnType gen_bin(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	FILE *f;
	Std_ReturnType ret = E_OK;
	if (asmb_config->fname_out_bin) {
		// fopen_s(&f, asmb_config->fname_out_bin, "wb+");
		f = fopen(asmb_config->fname_out_bin, "wb+");
		if (!f) {
			Failure("Unable to create output file:%s", asmb_config->fname_out_bin);
			return E_NOT_OK;
		}
		fwrite(memory, 1, maxaddress, f);
		fclose(f);
	}
	return ret;
}