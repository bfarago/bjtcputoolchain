#include <stdio.h>
#include "util.h"
#include "scanner.h"
#include "loc.h"
#include <string.h>
#define MAXSECTIONNAME (32)
#define MAXMEMORY (1<<12)
typedef enum {
	MT_undef,
	MT_code,
	MT_data,
	MT_max
} memoryType;
char section[MAXSECTIONNAME];
memoryType sectionType=MT_code;

char memory[MAXMEMORY];
int lines[MAXMEMORY];
memoryType memoryTypes[MAXMEMORY];


void setAddress(int a) {
	address = a;
	if (a > maxaddress) maxaddress = a;
}
void chkAddress(int a) {
	if (a > maxaddress) maxaddress = a;
}
void addMemory(int data) {
	lines[address] = yylineno;
	memoryTypes[address] = sectionType;
	memory[address++] = data & 0xf;
	chkAddress(address);
}
int parse_org() {
	int t = yylex();
	switch(t) {
	case T_IntConstant:
		setAddress(yylval.integerConstant);
		break;
	default:
		Failure("syntax error: org 0x1234");
		break;
	}
	return 0;
}

int parse_section() {
	int t = yylex();
	switch (t) {
	case T_Identifier:
		strncpy(section, yylval.identifier, MAXSECTIONNAME);
		break;
	case T_StringConstant:
		strncpy(section, yylval.stringConstant, MAXSECTIONNAME);
		break;
	default:
		Failure("syntax error: section .code");
		return false;
		break;
	}
	if (0 != strstr(section, "code")) {
		sectionType = MT_code;
	} else {
		sectionType = MT_data;
	}
	return 0;
}

int parse_global() {
	int t = yylex();
	switch (t) {
	case T_Identifier:
		// strncpy(section, yylval.identifier);
		break;
	default:
		Failure("syntax error: global identifier");
		break;
	}
	return 0;
}
int parse_nibles() {
	addMemory(yylval.integerConstant);
	int t = yylex();
	bool inside = 1;
	while (inside) {
		if (!t) t = yylex();
		switch (t) {
		case ',':
			t = 0;
			break;
		case T_IntConstant:
			t = 0;
			addMemory(yylval.integerConstant);
			break;
		default:
			inside = false;
			break;
		}
	}
	return t;
}
int parse_db() {
	int t = yylex();
	GType_s grm;
	t = parse_exp(t, &grm);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: db expression");
	}
	else {
		addMemory(grm.s.integerConstant);
	}
	return t;
}
int parse_identifier() {
	char* name = strdup(yylval.identifier);
	
	int t = yylex();
	switch (t) {
	case ':':
		t = 0;
		setSymbol(name, address);
		Debug("pse", "%s: label %s", __FUNCTION__, name);
		break;
	case T_Equ:
		Debug("pse", "%s: %s equ", __FUNCTION__, name);
		t = yylex();
		GType_s grm;
		t = parse_exp(t, &grm);
		if (grm.t != S_Exp) {
			Debug("grm", "Expression not found: mvi a,expression");
		}
		else {
			setSymbol(name, grm.s.integerConstant);
		}
		break;
	default:
		Failure("syntax error: identifier: or identifier equ 0xff");
		return false;
		break;
	}
	return t;
}

int parse_Op4_4(int mnemonic) {
	int v = 0;
	int opc = 0;
	setRelocType(RT_OP4_4);
	switch (mnemonic) {
		#undef TOK
		#define TOK(x, xop) case x: opc=xop; break;
		TOK(T_mvi, 0)
	}
	int t = yylex();
	GType_s grm;
	t = parse_exp(t, &grm);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: mvi a,expression");
	}
	v = grm.s.integerConstant;
	addMemory(opc);
	addMemory(v);
	return t;
}


int parse_Op4_12(int mnemonic) {
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
	GType_s grm;
	t = parse_exp(t, &grm);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: mvi a,expression");
	}
	v = grm.s.integerConstant;
	addMemory(opc);
	addMemory(v >> 8);
	addMemory(v >> 4);
	addMemory(v);
	return t;
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
				memory[addr+1] += (v >> 8) & 0xf;
				memory[addr+2] += (v >> 4) & 0xf;
				memory[addr+3] += v & 0xf;
				Debug("rel", "%s relocated %x:[op%03x]", name, addr, v&0xfff);
				break;
			case RT_OP4_4:
				memory[addr+1] += v & 0xf;
				Debug("rel", "%s relocated %x:[op%x]", name, addr, v&0xf);
				break;
			}
		} else {
			//not found, external
			Debug("ext", "extern %s relocate %x", name, addr);
		}
	}
}
#undef TOK
#define TOK(x,opc) case x:
extern FILE* yyin;
int parseFile(FILE* f) {
	int t = 0; // token
	int inside = 1;
	int save_lineno = yylineno;
	FILE* save_yyin = yyin;
	yyin = f;
	yylineno = 1;
	while (inside) {
		//if (feof(yyin)) break;
		if (!t) t = yylex();
		if (t>=T_Void) Debug("grm", "endterm: '%s'\n", gTokenNames[t- T_Void]);
		switch (t) {
		case T_NewLine: t = 0; break;
		case T_Org: t=parse_org(); break;
		case T_Section: t = parse_section(); break;
		case T_Global: t = parse_global(); break;
//		case T_Include: t = parse_include(); break;
		case T_DataByte: t = parse_db(); break;
		case T_Identifier: t = parse_identifier(); break;
		case T_IntConstant: t = parse_nibles(); break;
		TOK(T_mvi, 0)
			t=parse_Op4_4(t); break;

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
			t = parse_Op4_12(t); break;

		case T_End:
			//inside = 0;
			t = 0;
			break;
		case 0:
			inside = 0;
			break;
		default:
			Failure("syntax?");
			t = 0;
			break;
		}
	}
	yylineno = save_lineno;
	yyin = save_yyin;
	return t;
}

int parseFnmae(const char* fname) {
	FILE* f;
	f = fopen(fname, "r");
	if (!f) {
		Failure("Include file not found");
	}
	int ret = parseFile(f);
	fclose(f);
	return ret;
}

static const char *gMnemonics[16] =
{ "mvi a,","sta","lda","ad0","ad1","adc","nand","nor","rrm","jmp","jc","jnc","jz","jnz","jm","jp" };


int main(int argc, char** argv)
{
	printf("asm.bjtcpu v1.0");
	FILE* fin=stdin;
	ParseCommandLine(argc, argv);
	if (argc > 1) {
		fin = fopen(argv[1], "r");
		if (!fin) {
			Failure("File not found");
			return -1;
		}
		yyin = fin;
	}
	SetDebugForKey("lex", true);
	SetDebugForKey("rel", true);
	SetDebugForKey("ext", true);
	SetDebugForKey("grm", true);
	SetDebugForKey("pse", true);
	parseFile(yyin);
	if (argc > 1) {
		fclose(fin);
	}
	relocation();
	FILE* f = fopen("a.out", "wb+");
	if (!f) {
		Failure("Unable to create output file");
		return -2;
	}
	fwrite(memory, 1, maxaddress, f);
	fclose(f);

	f = fopen("a.coe", "w+");
	if (!f) {
		Failure("Unable to create coe output file");
		return -2;
	}
	fprintf(f, "memory_initialization_radix=16;\n");
	fprintf(f, "memory_initialization_vector=");
	for (int i = 0; i < maxaddress; i++) {
		if (i) fprintf(f, ",");
		fprintf(f,"%02x", memory[i]);
	}
	fprintf(f, ";\n");
	fclose(f);

	f = fopen("a.lst", "w+");
	fprintf(f, ";Generated list file\n");
	if (argc > 1) {
		fin = fopen(argv[1], "r");
	}
	int fin_line = 2;
	int cols = 0;
	#define BUFLEN 512
	char buflst[BUFLEN];
	char bufcom[BUFLEN];
	for (int i = 0; i < maxaddress; i++) {
		int m = memory[i];
		if (MT_data == memoryTypes[i]) {
			cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x", i, m);
		} else {
			switch (m) {
#undef TOK
#define TOK(x,opc) case opc:
				TOK(T_mvi, 0)
					cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x %x      %s 0x%x",
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
					cols += snprintf(buflst + cols, BUFLEN - cols, "%03x  %x %x %x %x  %s 0x%x%x%x",
						i,
						m, memory[i + 1], memory[i + 2], memory[i + 3],
						gMnemonics[m], memory[i + 1], memory[i + 2], memory[i + 3]);
				i += 3;
				break;
			default:
				cols += snprintf(buflst + cols, BUFLEN - cols, "; wrong binary content: %02x", m);
				break;
			}
		}
		while (fin_line<= lines[i]){
			
			if (!feof(fin)) {
				int colscom = 0;
				if (fin_line == lines[i]) {
					strncpy(bufcom, buflst, BUFLEN);
					colscom = cols;
					cols = 0;
				}
				while (colscom<35) {
					bufcom[colscom++] = ' ';
				}
				bufcom[colscom++] = ';';
				fgets(bufcom+colscom, BUFLEN- colscom, fin);
				if (strlen(bufcom)) {
					fprintf(f, "%s", bufcom);
					colscom = 0;
				}
			}
			fin_line++;
		}
		if (cols){
			buflst[cols] = 0;
			fprintf(f, "%s\n", buflst);
			cols = 0;
		}
	}
	fprintf(f, "end\n");
	fclose(f);
	return 0;
}
