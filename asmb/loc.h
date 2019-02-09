/** @file loc.h
*
* @brief Lexer Location storage and helper module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef YYLTYPE

/* Typedef: yyltype
 * Defines the struct type that is used by the scanner to store
 * position information about each lexeme scanned.
 */
typedef struct yyltype
{
    int timestamp;                 // you can ignore this field
    int first_line, first_column;
    int last_line, last_column;      
    char *text;                    // you can also ignore this field
} yyltype;

/* All of this module depends on that condition, if YYLTYPE is not yet definied in the lexer/grammar.
* Therefore, we use the same define to cheeck if all above is needed.
*/
#define YYLTYPE yyltype


/* Global variable: yylloc
 * The global variable holding the position information about the
 * lexeme just scanned.
 */
extern struct yyltype yylloc;


/* Function: Join
 * Takes two locations and returns a new location which represents
 * the span from first to last, inclusive.
 */
inline yyltype Join(yyltype first, yyltype last)
{
  yyltype combined;
  combined.first_column = first.first_column;
  combined.first_line = first.first_line;
  combined.last_column = last.last_column;
  combined.last_line = last.last_line;
  return combined;
}


/* Function: Joinp
 * Same as above Join, except operates on pointers as a convenience 
 */
inline yyltype Joinp(yyltype *firstPtr, yyltype *lastPtr)
{
  return Join(*firstPtr, *lastPtr);
}


#endif

