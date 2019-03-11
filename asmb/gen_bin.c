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
#include "intrep.h"

Std_ReturnType gen_bin(asmb_config_t *asmb_config, int maxaddress, char* memory)
{
	static char bfname[MAXFNAMELEN];
	const char* pfname=NULL;
	Std_ReturnType ret = E_OK;
	if (asmb_config->fname_out_bin) {
		pfname = asmb_config->fname_out_bin;
	}
	else {
		if (asmb_config->name_o) {
			snprintf(bfname, MAXFNAMELEN, "%s.bin", asmb_config->name_o);
			pfname = bfname;
			asmb_config->fname_out_bin = pfname;
		}
	}
	if (pfname) {
		FILE *f;
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
	Std_ReturnType ret = E_OK;
	if (!asmb_config->enable_map) return ret;
	if (asmb_config->name_o) {
		FILE *f;
		size_t len;
		char b[MAXFNAMELEN];
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
typedef enum {
	DF_VERSION,
	DF_NAME,
	DF_FNAME_BIN,
	DF_FNAME_ASM,
	DF_SYM,
	DF_LINES,
	DF_MEMTYPES
} tDbgFileBlockId;

typedef struct {
	unsigned int value;
	unsigned int lineno;
	unsigned char memtype;
	unsigned char len;
	unsigned char name[MAXSYMBOLENAME];
}tDbgFileSymbol;

Std_ReturnType dbgfile_wr(FILE*f, tDbgFileBlockId id, const void* b, unsigned int len) {
	unsigned int v;
	v = id;
	fwrite(&v, 4, 1, f);
	v = len;
	fwrite(&v, 4, 1, f);
	fwrite(b, len, 1, f);
	return E_OK;
}
Std_ReturnType gen_dbg(asmb_config_t *asmb_config)
{
	Std_ReturnType ret = E_OK;
	if (!asmb_config->enable_dbg) return ret;
	if (asmb_config->name_o) {
		FILE *f;
		size_t len;
		char b[MAXFNAMELEN];
		snprintf(b, MAXFNAMELEN, "%s.dbg", asmb_config->name_o);
		printf("Output debug database:%s\n", b);
		f = fopen(b, "wb+");
		if (!f) {
			Failure("Unable to create output file:%s", b);
			return E_NOT_OK;
		}
		int v = 1;
		dbgfile_wr(f, DF_VERSION, &v, 4);
		dbgfile_wr(f, DF_NAME, asmb_config->name_o, strlen(asmb_config->name_o));
		dbgfile_wr(f, DF_FNAME_BIN, asmb_config->fname_out_bin, strlen(asmb_config->fname_out_bin));
		dbgfile_wr(f, DF_FNAME_ASM, asmb_config->fname_in, strlen(asmb_config->fname_in));
		len = SymbolLength();
		for (unsigned int i = 0; i < len; i++) {
			tDbgFileSymbol sym;
			
			sym.value = SymbolValueByIndex(i);
			sym.memtype = getMemoryType(sym.value);
			sym.lineno = getMemoryLineNo(sym.value);
			const char* s = SymbolByIndex(i);
			sym.len = strlen(s);
			for (int j = 0; j < sym.len; j++) sym.name[j] = s[j];
			
			dbgfile_wr(f, DF_SYM, &sym, 5+ sym.len);
		}
		dbgfile_wr(f, DF_LINES, lines, MAXMEMORY*4);
		dbgfile_wr(f, DF_MEMTYPES, memoryTypes, MAXMEMORY * 4);
	}
	return ret;
}