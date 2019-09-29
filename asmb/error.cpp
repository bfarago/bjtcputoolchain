/** @file error.cpp
*
* @brief Error reporter class implementation.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include "error.h"
#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
using namespace std;
#define BUFFERSIZE (2048)

int Report::numErrors = 0;

#if (1==ReportAdvanced)
 
void Report::OutputError(yyltype *loc, const string& msg) {
    numErrors++;
    fflush(stdout); // make sure any buffered text has been output
    if (loc) {
        cerr << endl << "*** Error line " << loc->first_line << "." << endl;
    } else
        cerr << endl << "*** Error." << endl;
    cerr << "*** " << msg << endl << endl;
}


void Report::UntermComment() {
    OutputError(NULL, "Input ends with unterminated comment");
}

void Report::Formatted(yyltype *loc, const char *format, ...) {
	va_list args;
	char errbuf[BUFFERSIZE];

	va_start(args, format);
	vsnprintf(errbuf, BUFFERSIZE, format, args);
	va_end(args);
	OutputError(loc, errbuf);
}

void Report::LongIdentifier(yyltype *loc, const char *ident) {
    ostringstream s;
    s << "Identifier too long: \"" << ident << "\"";
    OutputError(loc, s.str());
}

void Report::UntermString(yyltype *loc, const char *str) {
    ostringstream s;
    s << "Unterminated string constant: " << str;
    OutputError(loc, s.str());
}

void Report::UnrecogChar(yyltype *loc, char ch) {
    ostringstream s;
    s << "Unrecognized char: '" << ch << "'";
    OutputError(loc, s.str());
}
#endif
