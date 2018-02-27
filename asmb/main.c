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
	switch (mnemonic) {
		case T_Inc: opc = 12; break;
	}
	memory[address++] = opc << 4 | (v  & 0xf);
	chkAddress(address);
	return true;
}
bool parse_Op4_12(int mnemonic) {
	int t = yylex();
	int v = 0;
	int opc = 0;
	switch (mnemonic) {
	    case T_Mfi: opc = 0; break;
		case T_Lda: opc = 1; break;
		case T_Sta: opc = 2; break;
		case T_Jmp: opc = 3; break;
		case T_Jz:  opc = 4; break;
		case T_Jnz: opc = 5; break;
		case T_Js:  opc = 6; break;
		case T_Jns: opc = 7; break;
		case T_Jc:  opc = 8; break;
		case T_Jnc: opc = 9; break;
		case T_Add: opc = 10; break;
		case T_Sub: opc = 11; break;
	}
	switch (t) {
		case T_Identifier: v = getSymbol(yylval.identifier); break;
		case T_IntConstant: v = yylval.integerConstant; break;
		default:
			Failure("syntax error: lda 0x12 or sta 0x12");
			return false;
			break;
	}
	memory[address++] = opc << 4 | ((v >> 8) & 0xf);
	memory[address++] = v& 0xff;
	chkAddress(address);
	return true;
}
void relocation() {
	int n = getRelocs();
	for (int i = 0; i < n; i++) {
		const char* name;
		int addr;
		getReloc(i, &name, &addr);
		int ix= searchSymbol(name);
		if (ix >= 0) {
			int v = getSymbol(name);
			memory[addr] &= 0xf0;
			memory[addr] |= (v>>8)&0xf;
			memory[addr +1] = v  & 0xff;
			printf("\n%s relocated [%x]=%x", name, addr, v);
		}
		else {
			//not found, external
			printf("\nextern %s relocate %x", name, addr);
		}
	}
}
extern FILE* yyin;
int main(int argc, char** argv)
{
	printf("asm.bjtcpu v1.0");
	FILE* fin;
	if (argc > 1) {
		fin = fopen(argv[1], "r");
		yyin= fin;
	}
	SetDebugForKey("lex", true);
	int t = 0;
	int inside = 1;
	while(!feof(stdin) & inside){
		t = yylex();
		switch(t) {
			case T_Org: parse_org(); break;
			case T_Section: parse_section(); break;
			case T_Global: parse_global(); break;
			case T_Include: Failure("not implemented."); break;
			case T_DataByte: parse_db(); break;
			case T_Identifier: parse_identifier(); break;
			case T_IntConstant: parse_db(); break;
			case T_Mfi:
			case T_Lda:
			case T_Sta:
			case T_Jmp:
			case T_Jz: 
			case T_Jnz:
			case T_Js: 
			case T_Jns:
			case T_Jc: 
			case T_Jnc:
			case T_Add:
			case T_Sub: parse_Op4_12(t); break;
			case T_Inc: parse_Op4_4(t); break;
				
			
			case T_End: inside = 0; break;
			default:
				Failure("syntax?");
				break;
		}
	}
	relocation();
	FILE* f=fopen("a.out", "w+");
	fwrite(memory, 1, maxaddress, f);
	fclose(f);
	if (argc > 1) {
		fclose(fin);
	}
//	yyparse();
	return 0;
}