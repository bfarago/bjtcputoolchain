#include <stdio.h>
#include "util.h"
#include "scanner.h"
#include "loc.h"

#define MAXMEMORY (16*1024)

char section[32];
char memory[MAXMEMORY];

void setAddress(int a) {
	address = a;
	if (a > maxaddress) maxaddress = a;
}
void chkAddress(int a) {
	if (a > maxaddress) maxaddress = a;
}
bool parse_org() {
	int t = yylex();
	switch(t) {
	case T_IntConstant:
		setAddress(yylval.integerConstant);
		break;
	default:
		Failure("syntax error: org 0x1234");
		break;
	}
	return true;
}

bool parse_section() {
	int t = yylex();
	switch (t) {
	case T_Identifier:
		strncpy(section, yylval.identifier);
		break;
	case T_StringConstant:
		strncpy(section, yylval.stringConstant);
		break;
	default:
		Failure("syntax error: section .code");
		return false;
		break;
	}
	return true;
}

bool parse_global() {
	int t = yylex();
	switch (t) {
	case T_Identifier:
		// strncpy(section, yylval.identifier);
		break;
	default:
		Failure("syntax error: global identifier");
		break;
	}
	return true;
}
bool parse_db() {
	int t = yylex();
	switch (t) {
	case T_Dollar:
		memory[address++] = address;
		break;
	case T_IntConstant:
		memory[address++] = yylval.integerConstant;
		chkAddress(address);
		break;
	default:
		Failure("syntax error: db 0xff");
		break;
	}
	return true;
}
bool parse_identifier() {
	char* name = strdup(yylval.identifier);
	int t = yylex();
	switch (t) {
	case ':':
		setSymbol(name, address);
		break;
	case T_Equ:
		t = yylex();
		switch (t) {
			case T_IntConstant:
				setSymbol(name, yylval.integerConstant);
				break;
			default:
				break;
		}
		break;
	default:
		Failure("syntax error: identifier: or identifier equ 0xff");
		return false;
		break;
	}
	return true;
}

bool parse_Op4_4(int mnemonic) {
	int v = 0;
	int opc = 0;
	setRelocType(RT_OP4_4);
	switch (mnemonic) {
		//#undef TOK
		#define TOK(x, xop) case x: opc=xop; break;
		TOK(T_mvi, 0)
	}
	memory[address++] = opc;
	memory[address++] = v & 0xf;

	chkAddress(address);
	return true;
}


bool parse_Op4_12(int mnemonic) {
	int t = yylex();
	int v = 0;
	int opc = 0;
	setRelocType(RT_OP4_12);
	switch (mnemonic) {
		//#undef TOK
		#define TOK(x, xop) case x: opc=xop; break;
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
	}
	switch (t) {
		case T_Identifier: v = getSymbol(yylval.identifier); break;
		case T_IntConstant: v = yylval.integerConstant; break;
		default:
			Failure("syntax error: lda 0x12 or sta 0x12");
			return false;
			break;
	}
	memory[address++] = opc;
	memory[address++] = v >> 8 & 0xf;
	memory[address++] = v >> 4 & 0xf;
	memory[address++] = v & 0xf;

	chkAddress(address);
	return true;
}
void relocation() {
	int n = getRelocs();
	for (int i = 0; i < n; i++) {
		const char* name;
		int addr;
		int relocType;
		getReloc(i, &name, &addr, &relocType);
		int ix= searchSymbol(name);
		if (ix >= 0) {
			int v = getSymbol(name);
			switch (relocType) {
			case RT_OP4_12:
				memory[addr] &= 0xf0;
				memory[addr] |= (v >> 8) & 0xf;
				memory[addr + 1] = v & 0xff;
				Debug("rel", "%s relocated %x:[op:%x %02x]", name, addr, (v>>8) & 0xf, v&0xff);
				break;
			case RT_OP4_4:
				memory[addr] &= 0xf0;
				memory[addr] |= v & 0xf;
				Debug("rel", "%s relocated %x:[op:%x]", name, addr, v&0xf);
				break;
			}
			
			
		}
		else {
			//not found, external
			Debug("ext", "extern %s relocate %x", name, addr);
		}
	}
}
#undef TOK
#define TOK(x,opc) case x:
int parseFile(FILE* f) {
	int t = 0; // token
	int inside = 1;

	while (!feof(stdin) & inside) {
		t = yylex();
		switch (t) {
		case T_Org: parse_org(); break;
		case T_Section: parse_section(); break;
		case T_Global: parse_global(); break;
		case T_Include: Failure("not implemented."); break;
		case T_DataByte: parse_db(); break;
		case T_Identifier: parse_identifier(); break;
		case T_IntConstant: parse_db(); break;
		TOK(T_mvi, 0)
			parse_Op4_4(t); break;

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
			parse_Op4_12(t); break;
		
		case T_End: inside = 0; break;
		default:
			Failure("syntax?");
			break;
		}
	}
	return t;
}
/*
TOK(T_mvi, 0)
TOK(T_sta, 1)
TOK(T_lda, 2)
TOK(T_ad0, 3)
TOK(T_ad1, 4)
TOK(T_adc, 5)
TOK(T_nand,6)
TOK(T_nor, 7)
TOK(T_rrm, 8)
TOK(T_jmp, 9)
TOK(T_jc, 10)
TOK(T_jnc,11)
TOK(T_jz, 12)
TOK(T_jnz,13)
TOK(T_jm, 14)
TOK(T_jp, 15)
*/
int parseFnmae(const char* fname) {
	FILE* f;
	f = fopen(fname, "r");
	if (!f) {
		Failure("Include file not found");
	}
	parseFile(f);
	fclose(f);
}
extern FILE* yyin;
int main(int argc, char** argv)
{
	printf("asm.bjtcpu v1.0");
	FILE* fin;
	ParseCommandLine(argc, argv);
	if (argc > 1) {
		fin = fopen(argv[1], "r");
		if (!fin) {
			Failure("File not found");
			return -1;
		}
		yyin= fin;
	}
	SetDebugForKey("lex", true);
	SetDebugForKey("rel", true);
	SetDebugForKey("ext", true);
	parseFile(yyin);
	relocation();
	FILE* f=fopen("a.out", "w+");
	if (!f) {
		Failure("Unable to create output file");
		return -2;
	}
	fwrite(memory, 1, maxaddress, f);
	fclose(f);
	if (argc > 1) {
		fclose(fin);
	}
	return 0;
}