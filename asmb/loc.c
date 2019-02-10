/** @file loc.h
*
* @brief Lexer Location storage and helper module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "loc.h"

yyltype Join(yyltype first, yyltype last)
{
  yyltype combined;
  combined.first_column = first.first_column;
  combined.first_line = first.first_line;
  combined.last_column = last.last_column;
  combined.last_line = last.last_line;
  return combined;
}


yyltype Joinp(yyltype *firstPtr, yyltype *lastPtr)
{
  return Join(*firstPtr, *lastPtr);
}

