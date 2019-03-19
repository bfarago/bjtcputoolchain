/** @file grammar.c
*
* @brief Grammar (not used bison/yacc grammar generator at this time)
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
#include "scanner.h"
#include "loc.h"
#include "intrep.h"
#include "grammar.h"

//grammar of the org syntax
int parse_org();
//grammer for the section syntax
int parse_section();
//grammar for the global syntax
int parse_global();
//grammar for nibble, or nibble list syntax
int parse_nibles();
//grammar for db syntax
int parse_db();
//grammar for identifiers
int parse_identifier();


//grammar of the org syntax
int parse_org() {
	int t = yylex();
	GType_s grm;
	SType_e context = ST_Label; // theres no org in contexts, and no rewriter is possible
	t = parse_exp(t, &grm, &context);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: org");
		Failure("Expression not found: org");
	}
	else {
		setAddress(grm.s.integerConstant);
	}
	/*
	int t = yylex();
	switch (t) {
	case T_IntConstant:
		setAddress(yylval.integerConstant);
		break;
	default:
		Failure("syntax error: org 0x1234");
		break;
	}*/
	return t;
}

//grammer for the section syntax
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
	checkSection(section);
	return 0;
}

//grammar for the global syntax
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

//grammar for nibble, or nibble list syntax
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
int convertAscii2BJTChar(int c) {
	if ((c >= '0') && (c <= '9')) {c=(c - '0'); }
	else if ((c >= 'A') && (c <= 'Z')) { c= (c - 'A' + 10); }
	else {
		switch (c) {
		case  '.': c = 0x24; break;
		case  ',': c = 0x25; break;
		case  ':': c = 0x26; break;
		case  ';': c = 0x27; break;
		case  '!': c = 0x28; break;
		case  '?': c = 0x29; break;
		case  '>': c = 0x2A; break;
		case  '<': c = 0x2B; break;
		case  '=': c = 0x2C; break;
		case  '+': c = 0x2D; break;
		case  '-': c = 0x2E; break;
		case  '/': c = 0x2F; break;
		case  '\\': c = 0x30; break;
		case  '(': c = 0x31; break;
		case  ')': c = 0x32; break;
		case  '#': c = 0x33; break;
		case  '@': c = 0x34; break;
		case  '�': c = 0x35; break;
		case  '$': c = 0x36; break;
		case  '"': c = 0x37; break; //use "": instead of \"
		case  '|': c = 0x38; break;
		case  '_': c = 0x39; break;
		case  '[': c = 0x3A; break;
		case L'�': c = 0x3B; break;
		case L'�': c = 0x3C; break;
		case L'�': c = 0x3D; break;
		case L'�': c = 0x3E; break;
		case  'l': c = 0x3F; break;
		case  L'�': c = 0x40; break;
		case  '{': c = 0x41; break;
		case  '}': c = 0x42; break;
		case  'o': c = 0x43; break;
		case  ' ': c = 0xFF; break;
		}
	}
	return c;
}
int parse_string() {
	int t = yylex();
	bool bHig = true;
	bool bLow = true;
	bool bQuo = false;
	if (t == ':') {
		t = yylex();
		bQuo = true;
	}else if (t == '+') {
		t = yylex();
		bLow = false;
	}else if (t == '-') {
		t = yylex();
		bHig = false;
	}
	int n = strlen(yylval.stringConstant);
	if (bLow)
	for (int i = 0; i < n; i++) {
		int c = yylval.stringConstant[i];
		c = convertAscii2BJTChar(c);
		if (bQuo) if (c == 0x26) c = 0x37;
		addMemory(c);
	}
	if (bHig)
	for (int i = 0; i < n; i++) {
		int c = yylval.stringConstant[i];
		c = convertAscii2BJTChar(c);
		if (bQuo) if (c == 0x26) c = 0x37;
		addMemory(c >> 4);
	}
	return t;
}
//grammar for db syntax
int parse_db() {
	int t = yylex();
	GType_s grm;
	SType_e context = ST_DB;
	t = parse_exp(t, &grm, &context);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: db expression");
		Failure("Expression not found:");
	}
	else {
		addMemory(grm.s.integerConstant);
	}
	return t;
}

//grammar for identifiers
int parse_identifier() {
	char* name = UTIL_STRDUP(yylval.identifier);
	SType_e context = ST_Unknown;
	int t = yylex();
	switch (t) {
	case ':':
		t = 0;
		setSymbol(name, address, ST_Label);
		Debug("pse", "%s: label %s", __FUNCTION__, name);
		break;
	case T_Equ:
		Debug("pse", "%s: %s equ", __FUNCTION__, name);
		t = yylex();
		GType_s grm;
		context = ST_EQU;
		t = parse_exp(t, &grm, &context);
		if (grm.t != S_Exp) {
			Debug("grm", "Expression not found: equ");
			Failure("Expression not found: equ");
		}
		else {
			setSymbol(name, grm.s.integerConstant, ST_EQU);
		}
		break;
	default:
		Failure("syntax error: identifier:"); // or identifier equ 0xff
		return false;
		break;
	}
	return t;
}

//grammar of the OP 4, 4 mnemonic(s) actually there is one: mvi a, <nibble>
int parse_Op4_4(int mnemonic) {
	int v = 0;
	int opc = 0;
	SType_e symContext = ST_Unknown;
	setRelocType(RT_OP4_4);
	switch (mnemonic) {
#undef TOK
#define TOK(x, xop, context) case x: opc=xop; symContext= context; break;
		TOK(T_mvi, 0, ST_LOAD)
	}
	int t = yylex();
	GType_s grm;
	t = parse_exp(t, &grm, &symContext);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: mvi a,expression");
		Failure("Expression not found: op 4,4");
	}
	v = grm.s.integerConstant;
	addMemory(opc);
	addMemory(v);
	return t;
}

//grammar of the OP 4, 12 mnemonics
int parse_Op4_12(int mnemonic) {
	int t = yylex();
	int v = 0;
	int opc = 0;
	SType_e symContext = ST_Unknown;

	setRelocType(RT_OP4_12);
	switch (mnemonic) {
		//#undef TOK
#define TOK(x, xop, context) case x: opc=xop; symContext= context; break;
		TOK(T_sta, 1, ST_STORE)
		TOK(T_lda, 2, ST_LOAD)
		TOK(T_ad0, 3, ST_LOAD)
		TOK(T_ad1, 4, ST_LOAD)
		TOK(T_adc, 5, ST_LOAD)
		TOK(T_nand, 6, ST_LOAD)
		TOK(T_nor, 7, ST_LOAD)
		TOK(T_rrm, 8, ST_LOAD)
		TOK(T_jmp, 9, ST_JMP)
		TOK(T_jc, 10, ST_JMP)
		TOK(T_jnc, 11, ST_JMP)
		TOK(T_jz, 12, ST_JMP)
		TOK(T_jnz, 13, ST_JMP)
		TOK(T_jm, 14, ST_JMP)
		TOK(T_jp, 15, ST_JMP)
	}
	GType_s grm;
	t = parse_exp(t, &grm, &symContext);
	if (grm.t != S_Exp) {
		Debug("grm", "Expression not found: mvi a,expression");
		Failure("Expression not found: op 4,12");
	}
	v = grm.s.integerConstant;
	addMemory(opc);
	addMemory(v );
	addMemory(v >> 4);
	addMemory(v >> 8);
	return t;
}

#undef TOK
#define TOK(x,opc) case x:
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
		if (t >= T_Void)
			Debug("grm", "endterm: '%s'\n", gTokenNames[t - T_Void]);
		switch (t) {
		case T_NewLine: t = 0; break;
		case T_StringConstant: t=parse_string(); break;
		case T_Org: t = parse_org(); break;
		case T_Section: t = parse_section(); break;
		case T_Global: t = parse_global(); break;
			//		case T_Include: t = parse_include(); break;
		case T_DataByte: t = parse_db(); break;
		case T_Identifier: t = parse_identifier(); break;
		case T_IntConstant: t = parse_nibles(); break;
			TOK(T_mvi, 0)
				t = parse_Op4_4(t); break;

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
/*
//parse a new file (include)
int parseFname(const char* fname) {
	FILE* f;
	f = fopen(fname, "r");
	if (!f) {
		Failure("Include file not found");
	}
	int ret = parseFile(f);
	fclose(f);
	return ret;
}
*/
