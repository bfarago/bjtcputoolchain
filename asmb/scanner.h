/** @file scanner.h
*
* @brief Lexer/Scanner interface module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _SCANNER_H
#define _SCANNER_H

#include <stdio.h>
#include <stdbool.h>
#include "loc.h"

#define MaxIdentLen (31)    //!< Maximum length for identifiers

/*
EXAMPLE:
sta addr; m(addr)<-a
lda addr; a<-m(addr)
mvi a, n; a<-n

ad0 addr; a<-a + m(addr)
ad1 addr; a<-a + m(addr) + 1
adc addr; a<-a + m(addr) + c
nand addr; a<-~(a & m(addr))
nor addr; a<-~(a | m(addr))
rrm addr; a<-m(addr) >> 1

jmp addr; un conditional jump. Ugr�s
jc addr; if Cy = 1
jnc addr; if Cy = 0
jz addr; if Z = 1
jnz addr; if Z = 0
jm addr; if S = 1
jp addr; if S = 0
*/

#ifdef _BEFORE_C99_AGES_
#define false 0
#define true 1
#endif

/* Typedef: TokenType enum
 * This enumeration defines the constants for the different token types.
 * The scanner should return these values for the associated key words
 * and token types.  The numbers start at 256 because 0-255 are reserved
 * for single character token values. After pp1, we will rely on
 * y.tab.h generated by yacc for these constants.
 */
#define TOK(x, opc) x
typedef enum { 
    T_Void = 256, T_NewLine,
    T_LessEqual, T_GreaterEqual, T_Equal, T_NotEqual, T_And, T_Or,
    T_While, T_For, T_If, T_Else, 
    T_Identifier, T_StringConstant, T_IntConstant, T_BoolConstant, 
	T_Section, T_Global, T_DataByte, T_Equ, T_Dollar,
	T_Comment, T_Org, T_At, T_Rol, T_Ror,
	TOK(T_mvi, 0),
	TOK(T_sta, 1),
	TOK(T_lda, 2),
	TOK(T_ad0, 3),
	TOK(T_ad1, 4),
	TOK(T_adc, 5),
	TOK(T_nand,6),
	TOK(T_nor, 7),
	TOK(T_rrm, 8),
	TOK(T_jmp, 9),
	TOK(T_jc, 10),
	TOK(T_jnc,11),
	TOK(T_jz, 12),
	TOK(T_jnz,13),
	TOK(T_jm, 14),
	TOK(T_jp, 15),
	T_End,
	S_Exp,
    T_NumTokenTypes
} TokenType_e;
#undef TOK

/* Global variable gTokenNames: 
 * These are a list of printable names for each token value defined
 * above.  The strings should match in position to the types. They
 * are used in our main program to verify output from your scanner.
 */
extern const char *gTokenNames[T_NumTokenTypes];
extern int yy_flex_debug;
 
/* Typedef: YYSTYPE
 * Defines the union type that is used by the scanner to store
 * attibute information about tokens as they are scanned. This
 * type definition will be generated by Yacc in the later assignments.
 */
typedef union {
    int integerConstant;
    bool boolConstant;
    char *stringConstant;
    char identifier[MaxIdentLen+1]; // +1 for terminating null
} YYSTYPE;


 
/* Global variable: yylval
 * Strange name, but by convention, this is the name of union that
 * is written to by scanner and read by parser containing information
 * about the lexeme just scanned.
 */
extern YYSTYPE yylval;

extern char *yytext;      //!< Text of lexeme just scanned

/** Following declarations can be used from c and c++ as well.
 */
#ifdef __cplusplus
extern "C" {
#endif

int yylex(void);              //!< Defined in the generated lex.yy.c file

void InitScanner(void);       //!< Defined in scanner.l user subroutines

extern int yylineno;	  //!< Defined in the generated lex.yy.c file

extern FILE* yyin;		  //!< used by scanner

#ifdef __cplusplus
}
#endif


#endif
