/** @file gen_lst.c
*
* @brief Generator implementation for lst output file.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "gen_lst.h"
#include "config.h"
#include "intrep.h"

#ifndef _WIN32
#define FORMAT_PCHAR "s"
#else
#define FORMAT_PCHAR "S"
#endif

Std_ReturnType gen_lst(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	char bfname[MAXFNAMELEN];
	const char* pfname = NULL;
	Std_ReturnType ret = E_OK;
	if (!asmb_config->enable_lst) return ret;
	if (asmb_config->fname_out_lst) {
		pfname = asmb_config->fname_out_lst;
	}
	else {
		if (asmb_config->name_o) {
			snprintf(bfname, MAXFNAMELEN, "%s.lst", asmb_config->name_o);
			pfname = bfname;
		}
	}
	if (pfname) {
		FILE *f;
		FILE* fin;
		int i;
		f = fopen(pfname, "w+");
		printf("Output lst file:%s\n", pfname);
		fprintf(f, ";Generated list file\n");
		if (asmb_config->fname_in) {
			fin = fopen(asmb_config->fname_in, "r");
		}else{
			fin = 0;
		}
		int fin_line = 2;
		unsigned int cols = 0;
		char buflst[BUFLEN];
		char bufcom[BUFLEN];
		for (i = 0; i < maxaddress; i++) {
			memoryMetaData_t* meta = &memoryMeta[i];
			int m = memory[i];
			if (MT_data == meta->sectionType) {
				cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x", i, m);
			}
			else {
				switch (m) {
#undef TOK
#define TOK(x,opc) case opc:
					TOK(T_mvi, 0)
						cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x %x      %" FORMAT_PCHAR " 0x%x",
							i,
							m, memory[i + 1],
							gMnemonics[m], memory[i + 1]);
					i++; // skip data
					break;
					TOK(T_sta, 1)
						TOK(T_lda, 2)
						TOK(T_ad0, 3)
						TOK(T_ad1, 4)
						TOK(T_adc, 5)
						TOK(T_nand, 6)
						TOK(T_nor, 7)
						TOK(T_rrm, 8)
						TOK(T_jmp, 9)
						TOK(T_jc, 10)
						TOK(T_jnc, 11)
						TOK(T_jz, 12)
						TOK(T_jnz, 13)
						TOK(T_jm, 14)
						TOK(T_jp, 15)
						cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x %x %x %x  %" FORMAT_PCHAR " 0x%x%x%x",
							i,
							m, memory[i + 1], memory[i + 2], memory[i + 3],
							gMnemonics[m], memory[i + 3], memory[i + 2], memory[i + 1]);
					i += 3;
					break;
				default:
					cols += snprintf(buflst + cols, BUFLEN - cols, "; wrong binary content: %02x", m);
					break;
				}
			}
			while (fin_line <= meta->line) {

				if (!feof(fin)) {
					int colscom = 0;
					if (fin_line == meta->line) {
						strncpy(bufcom, buflst, BUFLEN);
						colscom = cols;
						cols = 0;
					}
					while (colscom < 35) {
						bufcom[colscom++] = ' ';
					}
					bufcom[colscom++] = ';';
					fgets(bufcom + colscom, BUFLEN - colscom, fin);
					if (strlen(bufcom)) {
						fprintf(f, "%s", bufcom);
						colscom = 0;
					}
				}
				else {
					break;
				}
				fin_line++;
				if (fin_line > 100000) break; //prevent endless
			}
			if (cols) {
				if (cols < BUFLEN) {
					buflst[cols] = 0;
					fprintf(f, "%s\n", buflst);
				}
				cols = 0;
			}
		}
		fprintf(f, "end\n");
		fclose(f);
		fclose(fin);
	}

	return ret;
}