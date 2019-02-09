#include "scanner.h"

//next usage of this macro will be not enum, but string constant
#ifdef TOK
#undef TOK
#endif
#define TOK(x, opc) #x
const char *gTokenNames[T_NumTokenTypes] = {
	"T_Void", "T_NewLine",
	"T_LessEqual", "T_GreaterEqual", "T_Equal", "T_NotEqual", "T_And", "T_Or",
	"T_While", "T_For", "T_If", "T_Else",
	"T_Identifier", "T_StringConstant", "T_IntConstant", "T_BoolConstant",
	"T_Section", "T_Global", "T_DataByte", "T_Equ", "T_Dollar",
	"T_Comment", "T_Org", "T_At", "T_Rol", "T_Ror",
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
	"T_End",
	"Exp"
};