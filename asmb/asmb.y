%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "calc3.h"
/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(int i);
nodeType *con(int value);
void freeNode(nodeType *p);
int ex(nodeType *p);
int yylex(void);
void yyerror(char *s);
int sym[26]; /* symbol table */
%}
%union {
 int iValue; /* integer value */
 char sIndex; /* symbol table index */
 nodeType *nPtr; /* node pointer */
};
%token <iValue> INTEGER
%token <sIndex> VARIABLE
%token WHILE IF PRINT
%nonassoc IFX
%nonassoc ELSE
%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%type <nPtr> stmt expr stmt_list
%%
program:
 function { exit(0); }
 ;
function:
 function stmt { ex($2); freeNode($2); }
 | /* NULL */
 ;
stmt:
 ';' { $$ = opr(';', 2, NULL, NULL); }
 | expr ';' { $$ = $1; }
 | PRINT expr ';' { $$ = opr(PRINT, 1, $2); }
 | VARIABLE '=' expr ';' { $$ = opr('=', 2, id($1), $3); }
 | WHILE '(' expr ')' stmt { $$ = opr(WHILE, 2, $3, $5); }
 | IF '(' expr ')' stmt %prec IFX { $$ = opr(IF, 2, $3, $5); }
 | IF '(' expr ')' stmt ELSE stmt
 { $$ = opr(IF, 3, $3, $5, $7); }
 | '{' stmt_list '}' { $$ = $2; }
 ;
stmt_list:
 stmt { $$ = $1; }
 | stmt_list stmt { $$ = opr(';', 2, $1, $2); }
 ;
expr:
 INTEGER { $$ = con($1); }
 | VARIABLE { $$ = id($1); }
 | '-' expr %prec UMINUS { $$ = opr(UMINUS, 1, $2); }
 | expr '+' expr { $$ = opr('+', 2, $1, $3); }
 | expr '-' expr { $$ = opr('-', 2, $1, $3); }
 | expr '*' expr { $$ = opr('*', 2, $1, $3); }
 | expr '/' expr { $$ = opr('/', 2, $1, $3); }
 | expr '<' expr { $$ = opr('<', 2, $1, $3); }
 | expr '>' expr { $$ = opr('>', 2, $1, $3); }
 | expr GE expr { $$ = opr(GE, 2, $1, $3); }
 | expr LE expr { $$ = opr(LE, 2, $1, $3); }
 | expr NE expr { $$ = opr(NE, 2, $1, $3); }
 | expr EQ expr { $$ = opr(EQ, 2, $1, $3); }
 | '(' expr ')' { $$ = $2; }
 ;
25
%%
