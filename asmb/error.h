/** @file error.h
*
* @brief Error Reporter module
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#ifndef _ERROR_H_
#define _ERROR_H_

#include <string>
#include "loc.h"
using namespace std;

/**
 * General notes on using this class
 * Each of the methods in thie class matches one of the standard Decaf
 * errors and reports a specific problem such as an unterminated string,
 * type mismatch, declaration conflict, etc. You will call these methods
 * to report problems encountered during the analysis phases. All methods
 * on this class are static, thus you can invoke methods directly via
 * the class name, e.g.
 *
 *    if (missingEnd) { 
 *       Report::UntermString(&yylloc, str);
 *    }
 *
 * For some methods, the first argument is the pointer to the location
 * structure that identifies where the problem is (usually this is the
 * location of the offending token). You can pass NULL for the argument
 * if there is no appropriate position to point out. For other methods,
 * location is accessed by messaging the node in error which is passed
 * as an argument. You cannot pass NULL for these arguments.
 */


class Report {
 public:

  // Errors used by scanner
  static void UntermComment(); 

  // Returns number of error messages printed
  static int NumErrors() { return numErrors; }

#ifdef ReportAdvanced
  static void LongIdentifier(yyltype *loc, const char *ident);

  static void UntermString(yyltype *loc, const char *str);
  static void UnrecogChar(yyltype *loc, char ch);

  // Generic method to report a printf-style error message
  static void Formatted(yyltype *loc, const char *format, ...);
#endif  
 private:
#ifdef ReportAdvanced
  static void UnderlineErrorInLine(const char *line, yyltype *pos);
#endif
  static void OutputError(yyltype *loc, const string& msg);
  static int numErrors;
};
#endif
