/** @file util.cpp
*
* @brief Lexer/Scanner and grammar utility module.
*
* @par
* COPYRIGHT NOTICE: (c) 2018 Barna Farago.  All rights reserved.
*/
#include <stdarg.h>
#include <string.h>
#include <vector>
#include "util.h"
#include "intrep.h"

using std::vector;

FILE* g_err_file = stdout;

//asmb debug and verbosity classes
static vector<const char*> debugKeys;

//Internal representation of the symbols
static vector<const char*> symbols;
static vector<int> symbolValues;
static vector<SType_e> symbolTypes;
static vector<SContexts_t> symbolContext;
static vector<const char*> sections;
static vector<const char*> includes;
static const int UTIL_BUFFERSIZE = 2048;

int address = 0;
int maxaddress = 0;
relocType_en actualRelocType = RT_OP4_12;

static vector<const char*> reloc;
static vector<int> relocAddress;
static vector<relocType_en> relocType;
static vector<SType_e> relocContext;
static vector<int> relocLine;
static vector<GType_s> gStack;
vector<tErrorRecord> g_ErrorList;
static int g_include_actual = 0;
static vector<int> g_include_stack;		// fileId
static vector<int> g_include_stack_line; // line nr yylinenr

void stackPush(int t, const YYSTYPE* s) {
	GType_s p ;
	p.t = t;
	p.s = *s;
	gStack.push_back(p);

}
void stackClear() {
	gStack.clear();
}
int ruleExp[] = 
{
	T_IntConstant, '+', T_IntConstant, T_Void, 
	T_IntConstant, '-', T_IntConstant, T_Void, 
	T_IntConstant, '*', T_IntConstant, T_Void,
	T_IntConstant, '/', T_IntConstant, T_Void,
	T_IntConstant, T_Ror, T_IntConstant, T_Void,
	T_IntConstant, T_Rol, T_IntConstant, T_Void,
	T_IntConstant, T_At, T_IntConstant, T_Void,
	// T_IntConstant, '&', T_IntConstant, T_Void,	//TODO: Lexer doesn't forwards the and sign
	T_IntConstant, T_Void
};

int parse_exp(int t, GType_s* res, SType_e* pContext) {
	int r = 0;
	size_t top = gStack.size();
	Debug("pse", __FUNCTION__);
	while (0 == r) {
		switch (t) {
		case '/':
		case '*':
		case '-':
		case '+':
		case T_At:
		case T_Ror:
		case T_Rol:
			stackPush(t, &yylval);
			break;
		case T_Identifier:
			t = T_IntConstant; //rewrire
			yylval.integerConstant = getSymbol(yylval.identifier, pContext, yylineno);
			stackPush(t, &yylval);
			break;
		case T_IntConstant:
			stackPush(t, &yylval);
			break;
		case T_Dollar:
			t = T_IntConstant; //rewrire
			yylval.integerConstant = address;
			stackPush(t, &yylval);
			break;
		default:
			r = t;
			break;
		}
		if (!r) t = yylex();
	}
	size_t len = gStack.size();
	do{
		int sp = 0;
		size_t rule = 0;
		size_t ruleNr = 0;
		for (size_t i = 0; i < sizeof(ruleExp) / sizeof(int); i++) {
			if (T_Void == ruleExp[i]) break;
			if ((i - rule) < (len-top)) {
				GType_s& s = gStack[top + sp];
				if (s.t == ruleExp[i]) {
					//accept next
					sp++;
					continue;
				}
			}
			//search next rule
			sp = 0;
			while (T_Void != ruleExp[i]) i++;
			rule = i + 1;
			ruleNr++;
		}
		res->t = S_Exp;
		switch (ruleNr) {
		//case 0: // exp= exp + 1  @ exp
		//	res->s.integerConstant = (gStack[top + 0].s.integerConstant + gStack[top + 2].s.integerConstant) >> (4 * gStack[top + 4].s.integerConstant);
		//	top += 4;
		//	gStack[top].s.integerConstant = res->s.integerConstant;
		//	break;
		case 0: // exp= exp + exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant + gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 1: // exp= exp - exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant - gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 2: // exp= exp * exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant * gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 3: // exp= exp / exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant / gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 4: // exp= exp >> exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant >> gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 5: // exp= exp << exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant << gStack[top + 2].s.integerConstant;
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		case 6: // exp= exp @ exp
			res->s.integerConstant = gStack[top + 0].s.integerConstant >> (4 * gStack[top + 2].s.integerConstant);
			top += 2;
			gStack[top].s.integerConstant = res->s.integerConstant;
			break;
		//case 7: // exp= exp & exp
		//	res->s.integerConstant = gStack[top + 0].s.integerConstant & (4 * gStack[top + 2].s.integerConstant);
		//	top += 2;
		//	gStack[top].s.integerConstant = res->s.integerConstant;
		//	break;
		case 7: // exp = i
			res->s.integerConstant = gStack[top + 0].s.integerConstant;
			top++;
			break;
		default:
			res->t = T_Void;
			top++;
			break;
		}
	}	while (top < len);
	stackClear();
	return r;
}

int getErrorListLength() { return (int)g_ErrorList.size(); }
const tErrorRecord* getErrorListItem(int index) { return &g_ErrorList[index]; }

void AddError(int errCode, short fileId, int line, char* errbuf, const char* quote)
{
	tErrorRecord e;
	const char* fileName;
	e.fileId = fileId;
	fileName = include_get(fileId);
	e.lineNr = line;
	e.errorCode = errCode;
	if (quote)
		snprintf(e.errorText, MAXFNAMELEN, "Failure in %s line:%d: %s '%s'", fileName, e.lineNr, errbuf, quote);
	else
		snprintf(e.errorText, MAXFNAMELEN, "Failure in %s line:%d: %s", fileName, e.lineNr, errbuf);
	for (size_t i = 0; i < strlen(e.errorText); i++) {
		if (e.errorText[i] == 0x0a) e.errorText[i] = 0x20; //replace new line to space
	}
	g_ErrorList.push_back(e);
	fprintf(g_err_file, "\n*** %s\n\n", e.errorText);
}
void Failure(const char *format, ...) {
  va_list args;
  char errbuf[UTIL_BUFFERSIZE];
  va_start(args, format);
  vsnprintf(errbuf, UTIL_BUFFERSIZE, format, args);
  va_end(args);
  
  AddError(0, include_actual_get(), yylineno, errbuf, yytext);
}
void WrongToken(int t, const char *format, ...) {
	va_list args;
	char errbufIn[UTIL_BUFFERSIZE];
	char errbufOut[UTIL_BUFFERSIZE];
	va_start(args, format);
	vsnprintf(errbufIn, UTIL_BUFFERSIZE, format, args);
	va_end(args);
	int lineno = yylineno;
	const char* tokname=0;
	if (t>=T_Void)
		tokname= gTokenNames[t - T_Void];

	switch (t) {
	case T_NewLine:
		lineno--;
		snprintf(errbufOut, MAXFNAMELEN, "Syntax error before end of line %s", errbufIn);
		break;
	case T_Identifier:
		snprintf(errbufOut, MAXFNAMELEN, "Syntax error Identifier '%s', %s ", yylval.identifier, errbufIn );
		break;
	default:
		if (tokname) {
			snprintf(errbufOut, MAXFNAMELEN, "Syntax error %s : %s", tokname, errbufIn);
		}
		else {
			snprintf(errbufOut, MAXFNAMELEN, "Syntax error %s", errbufIn);
		}
		break;
	}
	AddError(1, include_actual_get(), lineno, errbufOut, yytext);
}
int IndexOf(const char *key) {
  for (unsigned int i = 0; i < debugKeys.size(); i++)
    if (!strcmp(debugKeys[i], key)) 
      return i;

  return -1;
}

bool IsDebugOn(const char *key) {
  return (IndexOf(key) != -1);
}

void SetDebugForKey(const char *key, bool value) {
  int k = IndexOf(key);
  if (!value && k != -1)
    debugKeys.erase(debugKeys.begin() + k);
  else if (value && k == -1)
    debugKeys.push_back(key);
}

void Debug(const char *key, const char *format, ...) {
  va_list args;
  char buf[UTIL_BUFFERSIZE];

  if (!IsDebugOn(key))
     return;
  
  va_start(args, format);
  vsnprintf(buf, UTIL_BUFFERSIZE, format, args);
  va_end(args);
  fprintf(g_err_file, "+++ (%s)%i: %s%s", key, yylineno, buf, buf[strlen(buf)-1] != '\n'? "\n" : "");
}

void printHelp() {
	printf("See detailed help: http://github.com/bfarago/bjtcputoolchain");
	printf("\n");
	printf("asmb [-h] [-v] input.asm\n");
	printf(" -h : This help.\n");
	printf(" -v : Verbose lexer and grammar. Very noisy...\n");
	printf(" -i FileName.asm : input specification. This is the default switch, not needed...\n");
	printf(" -b Filename.bin : Optional. Specify the bin output file name, in case of input and output name are different.\n");
	printf(" -o FileName : Optional. This will be the name part of the output file names, without the suffixes. Input file name will be used as base by default, when it is not specified.\n");
	printf(" -f : Fpga related outputs are enabled. (fname.coe and fname.v)\n");
	printf(" -l : lst output is enabled. (fname.lst)\n");
	printf(" -m : map output is enabled. (fname.map)\n");
	printf(" -g : dbg output is enabled. (fname.dbg)\n");
	printf(" -e : standard err output redirected to fname.log\n");
	//printf(" -E fname.log: standard err output redirected to the specified file (fname.log)\n");
	
	printf("\n");
}

void InitConfig(asmb_config_t* cfg) {
	cfg->fname_in = NULL;
	cfg->name_o = NULL;
	cfg->enable_fpga = 0;
	cfg->enable_lst = 0;
	cfg->enable_map = 0;
	cfg->fname_out_bin = 0;
	cfg->fname_out_coe = 0;
	cfg->fname_out_verilog = 0;
	cfg->fname_out_lst = 0;
	cfg->fname_err = 0;
	cfg->enable_err = 0;
	cfg->enable_dbg = 0;
}

Std_ReturnType ParseCommandLine(int argc, char *argv[], asmb_config_t* cfg) {
	yy_flex_debug = 0;
	if (argc > 1) {
	  char actualSwitch = 'i';
	  for (int i = 1; i < argc; i++) {
		  if (argv[i][0] == '-') {
			  switch (argv[i][1]) {
			  case 'h':
			  case 'H':
				  printHelp();
				  exit(0); //need to exit here, because of the parsing will starts on standard input later.
				  break;
			  case 'o': 
			  case 'b':
			  case 'i':
				  actualSwitch = argv[i][1];
				  break;
			  case 'v':
				  SetDebugForKey("lex", true);
				  SetDebugForKey("rel", true);
				  SetDebugForKey("ext", true);
				  SetDebugForKey("grm", true);
				  SetDebugForKey("pse", true);
				  yy_flex_debug = 1;
				  break;
			  case 'f':
				  cfg->enable_fpga = 1;
				  break;
			  case 'l':
				  cfg->enable_lst = 1;
				  break;
			  case 'm':
				  cfg->enable_map = 1;
				  break;
			  case 'e':
				  cfg->enable_err = 1;
				  break;
			  case 'g':
				  cfg->enable_dbg = 1;
				  break;
			  case 'd':
				  // TODO: debug switches ?
				  break;
			  default:
				  printf("Error: Wrong command line argument specified : '%c'.\n", argv[i][1]);
				  printHelp();
				  exit(-1);
				  break;
			  }
		  }
		  else {
				//other than switch char ( ^{\-.+} ), assume this is the input filename
			  switch (actualSwitch) {
				case 'i':cfg->fname_in = argv[i]; break;
				case 'b':cfg->fname_out_bin = argv[i]; break;
				case 'o':cfg->name_o = argv[i];	break;
			  }
				
		  }

	  }
	  if (cfg->fname_in) {
		  const char* p = strrchr(cfg->fname_in, '.');
		  if (!cfg->name_o) {
			  if (p) {
				  static char b[255];
				  strncpy_s(b, 255, cfg->fname_in, p- cfg->fname_in);
				  cfg->name_o = b;
			  } else {
				  //theres no . in the spec name.

			  }
		  }
	  }

	}
	if (cfg->enable_err) {
		static char s[255];
		sprintf_s(s, "%s.log", cfg->name_o);
		cfg->fname_err = s; // todo: check if this is needed  really
		// g_err_file = fopen(s, "w+");
		fopen_s(&g_err_file, s, "w+");
		//this would be fine on unix,linux
		//freopen(s, "a+", stderr);
		freopen(s, "a+", stdout); // not works with new windows crt lib
	}
	printf("Output name prefix:%s\n", cfg->name_o);
	return E_OK;
}

int yywrap() {
	return 1;
}
size_t SymbolLength() {
	return symbols.size();
}
const char* SymbolByIndex(size_t index) {
	return symbols[index];
}
int SymbolValueByIndex(size_t index) {
	return symbolValues[index];
}
SType_e getSymbolType(int index) {
	return symbolTypes[index];
}
SContexts_t getSymbolContexts(int index) {
	return symbolContext[index];
}
/*
size_t RelocLength() {
	return reloc.size();
}
*/

int searchSymbol(const char *key) {
	for (unsigned int i = 0; i < symbols.size(); i++)
		if (!strcmp(symbols[i], key))
			return i;

	return -1;
}
void addReloc(const char* name, int addr, relocType_en rt, SType_e* pContext, int line) {
	reloc.push_back(UTIL_STRDUP(name));
	relocAddress.push_back(addr);
	relocType.push_back(rt);
	relocContext.push_back(*pContext);
	relocLine.push_back(line);
}
size_t getRelocs() {
	return reloc.size();
}
int getReloc(int index, const char**name, int* adr, relocType_en* rt, SType_e* pContext, int *line) {
	*name = reloc[index];
	*adr = relocAddress[index];
	*rt = relocType[index];
	*pContext = relocContext[index];
	*line = relocLine[index];
	return 1;
}
void setSymbol(const char* name, int value, SType_e symbolType) {
	int index = searchSymbol(name);
	if (-1 == index) {
		symbols.push_back(name);
		symbolValues.push_back(value);
		symbolTypes.push_back(symbolType);
		symbolContext.push_back(1 << symbolType);
	}else {
		Failure("Symbol already declared");
	}
}
void setRelocType(relocType_en rt) {
	actualRelocType = rt;
}
int getSymbol(const char* name, SType_e* pContext, int line) {
	int index = searchSymbol(name);
	if (index < 0) {
		addReloc(name, address, actualRelocType, pContext, line);
		return 0; //this will be added to resolved value at pass2, relocation.
	}
	symbolContext[index] |= 1 << *pContext;
	return symbolValues[index];
}
int getMemorySectionNumbers() {
	return (int)sections.size();
}
const char* getMemorySectionName(int index) {
	return sections[index];
}
void checkSection(char* secName) {
	if (
		(0 != strstr(secName, "code")) ||
		(0 != strstr(secName, "text"))
		)
	{
		sectionType = MT_code;
	}
	else {
		sectionType = MT_data;
	}
	for (size_t i = 0; i < sections.size(); i++) {
		if (strcmp(sections[i], secName) == 0) {
			sectionId = i;
			return;
		}
	}
	//new section
	sectionId = (int)sections.size();
	sections.push_back(_strdup(secName));
}

int include_actual_get()
{
	return g_include_actual;
}
int include_get_max() {
	return (int)includes.size();
}
const char* include_get(int index) {
	return includes[index];
}
int include_search(const char* fname) {
	int n = (int)includes.size();
	if (n < 1) return -1;
	for (int i = 0; i < n; i++)
	{
		const char* a = includes[i];
		if (0 == strcmp(a, fname)) {
			return i;
		}
	}
	return -2;
}


void include_add(const char* fname) {
	int index = include_search(fname);
	if (index < 0) {
		//add it, this is the first appeareance
		const char* fn = _strdup(fname);
		includes.push_back(fn);
		index = (int)includes.size()-1; // I hope :), this is the index.
	}
	g_include_stack.push_back(index);
	g_include_stack_line.push_back(yylineno);
	yylineno = 1;
	g_include_actual = index;//g_include_stack.back();
}

void include_eof() {
	g_include_stack.pop_back();
	if (g_include_stack.size()) {
		g_include_actual = g_include_stack.back(); //get previous;
		yylineno= g_include_stack_line.back();
	}
	else {
		g_include_actual = 0;
	}
	
}