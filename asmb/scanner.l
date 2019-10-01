/* File:  scanner.l
 * ----------------
 * Lex input file to generate the scanner for the compiler.
 */
%top{
#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define YY_NO_UNISTD_H
#else

#ifndef _BSD_SOURCE
#warning On linux, please add -D_BSD_SOURCE to the compiler flags!
#endif

#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE 1
#endif

#include <unistd.h>
int isatty(int fd);

/*
One compiler warning comes from flex issue, no problem in our usecase:
https://github.com/westes/flex/issues/61
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
}
%x incl
%{

/* The text within this first region delimited by %{ and %} is assumed to
 * be C/C++ code and will be copied verbatim to the lex.yy.c file ahead
 * of the definitions of the yylex() function. Add other header file inclusions
 * or C++ variable declarations/prototypes that are needed by your code here.
 */

#include "scanner.h"
#include "util.h" // for PrintDebug()

/* Global variable: yylval
 * -----------------------
 * This global variable is how we get attribute information about the token
 * just scanned to the client. The scanner sets the global variable
 * appropriately and since it's global the client can just read it.  In the
 * future, this variable will be declared for us in the y.tab.c file
 * produced by Yacc, but for now, we declare it manually.
 */
YYSTYPE yylval;  // manually declared for pp1, later Yacc provides

/* Global variable: yylloc
 * -----------------------
 * This global variable is how we get position information about the token
 * just scanned to the client. (Operates similarly to yylval above)
 */
struct yyltype yylloc; // manually dclared for pp1, later Yacc provides

/* Macro: YY_USER_ACTION 
 * ---------------------
 * This flex built-in macro can be defined to provide an action which is
 * always executed prior to any matched rule's action. Basically, it is
 * a way of having a piece of code common to all actions factored out to
 * this routine.  We already defined it for you and left the empty
 * function DoBeforeEachAction ready for your use as needed. It will
 * be called once for each pattern scanned from the file, before
 * executing its action.
 */
static void Do_User_Action(); 
#define YY_USER_ACTION Do_User_Action();

#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

%}

%option yylineno nounput noinput caseless

 /* The section before the first %% is the Definitions section of the lex
  * input file. Here is where you set options for the scanner, define lex
  * states, and can set up definitions to give names to regular expressions
  * as a simple substitution mechanism that allows for more readable
  * entries in the Rules section later. 
  */


%%             /* BEGIN RULES SECTION */
 /* All patterns and actions should be placed between the start and stop
  * %% markers which delimit the Rules section. 
  */
[;][^\n]*[\n] {
		//yylloc.first_line++;
		return T_NewLine;
	}

[-()<>=+*/{}.:] {
	 return *yytext;
	}
include             BEGIN(incl);
<incl>[ \t]*      /* eat the whitespace */
<incl>["][^"]*["]   { /* got the include file name */
		yylval.stringConstant = UTIL_STRDUP(yytext+1);
		if (yylval.stringConstant[yyleng-2] != '"')
			Debug("lex","improperly terminated string");
		else
			yylval.stringConstant[yyleng-2] = 0;
		Debug("lex", "found INCLUDE", yylval.stringConstant);
		if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
            {
            Failure("Includes nested too deeply" );
            exit( 1 );
            }

        include_stack[include_stack_ptr++] =
            YY_CURRENT_BUFFER;

        yyin = include_fopen( yylval.stringConstant, "r" );

        if ( ! yyin )
            Failure( "File not found");

        yy_switch_to_buffer(
            yy_create_buffer( yyin, YY_BUF_SIZE ) );

        BEGIN(INITIAL);
		//return T_Include;
	}

["][^"]*["] {
		yylval.stringConstant = UTIL_STRDUP(yytext+1);
		if (yylval.stringConstant[yyleng-2] != '"')
			Debug("lex","improperly terminated string");
		else
			yylval.stringConstant[yyleng-2] = 0;
		Debug("lex","found '%s'\n", yylval.stringConstant);
		return T_StringConstant;
	}
"section" return T_Section;
"global" return T_Global;

"sta"	return T_sta;
"lda"	return T_lda;
"mvi a,"	return T_mvi;

"ad0"	return T_ad0;
"ad1"	return T_ad1;
"adc"	return T_adc;
"nand"	return T_nand;
"nor"	return T_nor;
"rrm"	return T_rrm;
"jmp"	return T_jmp;
"jz"	return T_jz;
"jnz"	return T_jnz;
"jm"	return T_jm;
"jp"	return T_jp;
"jc"	return T_jc;
"jnc"	return T_jnc;

"db"	return T_DataByte;
"equ"	return T_Equ;
"org"	return T_Org;
"end"	return T_End;
[$]		return T_Dollar;
[@]		return T_At;
">>"	return T_Ror;
"<<"	return T_Rol;
[a-z_\.]+[a-z_\.0-9]* {
		strncpy(yylval.identifier, yytext, MaxIdentLen);
		return T_Identifier;
	  }
0[x][0-9a-z]+ {
		yylval.integerConstant = strtol(yytext, NULL, 16);
		return T_IntConstant;
}

[0-9a-z]+[h] {
		yylval.integerConstant = strtol(yytext, NULL, 16);
		return T_IntConstant;
}


[0-9]+ {
		yylval.integerConstant = strtol(yytext, NULL, 10);
		return T_IntConstant;
}
[0-1]+"b" {
		yylval.integerConstant = strtol(yytext, NULL, 2);
		return T_IntConstant;
}
[\n]	{
		// yylloc.first_line++;
		return T_NewLine;
	}
[\t ] ;

<<EOF>> {
	if ( --include_stack_ptr < 0 )
            {
            yyterminate();
            }

        else
            {
			include_eof();
            yy_delete_buffer( YY_CURRENT_BUFFER );
            yy_switch_to_buffer(
                 include_stack[include_stack_ptr] );
            }
	}

. ;

%%
/* The closing %% above marks the end of the Rules section and the beginning
 * of the User Subroutines section. All text from here to the end of the
 * file is copied verbatim to the end of the generated lex.yy.c file.
 * This section is where you put definitions of helper functions.
 */

#ifndef _WIN32
//see prolog, flex related warnings reactivated...
#pragma GCC diagnostic pop
#endif

/* Function: InitScanner
 * ---------------------
 * This function will be called before any calls to yylex().  It is designed
 * to give you an opportunity to do anything that must be done to initialize
 * the scanner (set global variables, configure starting state, etc.). One
 * thing it already does for you is assign the value of the global variable
 * yy_flex_debug that controls whether flex prints debugging information
 * about each token and what rule was matched. If set to false, no information
 * is printed. Setting it to true will give you a running trail that might
 * be helpful when debugging your scanner. Please be sure the variable is
 * set to false when submitting your final version.
 */
void InitScanner()
{
    Debug("lex", "Initializing scanner");
    yy_flex_debug = false;
}


/* Function: Do_User_Action()
 * ------------------------------
 * This function is installed as the YY_USER_ACTION. This is a place
 * to group code common to all actions.
 */
static void Do_User_Action()
{
}
