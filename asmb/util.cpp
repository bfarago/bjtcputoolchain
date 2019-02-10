#include "util.h"
#include <stdarg.h>
#include <string.h>
#include <vector>

using std::vector;


static vector<const char*> debugKeys;
static vector<const char*> symbols;
static vector<int> symbolValues;
static const int UTIL_BUFFERSIZE = 2048;

int address = 0;
int maxaddress = 0;
relocType_en actualRelocType = RT_OP4_12;

static vector<const char*> reloc;
static vector<int> relocAddress;
static vector<relocType_en> relocType;


static vector<GType_s> gStack;
void stackPush(int t, YYSTYPE s) {
	GType_s p ;
	p.t = t;
	p.s = s;
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
	T_IntConstant, T_Void
};

int parse_exp(int t, GType_s* res) {
	int r = 0;
	//int lval = 0;
	//int op = 0;
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
			stackPush(t, yylval);
			break;
		case T_Identifier:
			t = T_IntConstant; //rewrire
			yylval.integerConstant = getSymbol(yylval.identifier);
			stackPush(t, yylval);
			break;
		case T_IntConstant:
			stackPush(t, yylval);
			break;
		case T_Dollar:
			t = T_IntConstant; //rewrire
			yylval.integerConstant = address;
			stackPush(t, yylval);
			break;
		default:
			r = t;
			break;
		}
		if (!r) t = yylex();
	}
	size_t len = gStack.size();
	int sp = 0;
	size_t rule = 0;
	for (size_t i = 0; i < sizeof(ruleExp)/sizeof(int); i++) {
		if (T_Void == ruleExp[i]) break;
		if ( (i-rule) < len) {
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
		rule = i+1;
		
	}
	res->t = S_Exp;
	switch (rule) {
	case 0: // exp= exp + exp
		res->s.integerConstant = gStack[top+0].s.integerConstant + gStack[top+2].s.integerConstant;
		break;
	case 4: // exp= exp - exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant - gStack[top + 2].s.integerConstant;
		break;
	case 8: // exp= exp * exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant * gStack[top + 2].s.integerConstant;
		break;
	case 12: // exp= exp / exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant / gStack[top + 2].s.integerConstant;
		break;
	case 16: // exp= exp >> exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant >> gStack[top + 2].s.integerConstant;
		break;
	case 20: // exp= exp << exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant << gStack[top + 2].s.integerConstant;
		break;
	case 24: // exp= exp @ exp
		res->s.integerConstant = gStack[top + 0].s.integerConstant >> (4* gStack[top + 2].s.integerConstant);
		break;
	case 28: // exp = i
		res->s.integerConstant = gStack[top+0].s.integerConstant;
		break;
	default:
		res->t = T_Void;
		break;
	}
	stackClear();
	return r;
}

void Failure(const char *format, ...) {
  va_list args;
  char errbuf[UTIL_BUFFERSIZE];
  va_start(args, format);
  vsnprintf(errbuf, UTIL_BUFFERSIZE, format, args);
  va_end(args);
  fflush(stdout);
  fprintf(stderr,"\n*** Failure: %s\n\n", errbuf);
  abort();
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
  printf("+++ (%s)%i: %s%s", key, yylineno, buf, buf[strlen(buf)-1] != '\n'? "\n" : "");
}

void printHelp() {
	printf("See detailed help: http://github.com/bfarago/bjtcputoolchain");
	printf("\n");
	printf("asmb [-h] [-v] input.asm\n");
	printf(" -h : This help.\n");
	printf(" -v : Verbose lexer and grammar. Very noisy...\n");
	printf(" -i FileName.asm : input specification. This is the default switch, not needed...\n");
	printf(" -o FileName : not implemented ! This will be the output filename without suffices.\n");
	printf("\n");
}


Std_ReturnType ParseCommandLine(int argc, char *argv[], asmb_config_t* cfg) {
	char actualSwitch = 'i';
/*  
//replaced by -v now
  if (strcmp(argv[2], "-d") != 0) { // first arg is not -d
	if (argc < 3) return E_NOT_OK;
    printf("Incorrect Use:   ");
    for (int i = 1; i < argc; i++) printf("%s ", argv[i]);
    printf("\n");
    printf("Correct Usage:  fname.asm -d <debug-key-1> <debug-key-2> ... \n");
    exit(2); //Stop the whole program
  }
  for (int i = 3; i < argc; i++)
	SetDebugForKey(argv[i], true);
 */

	/*
	// Ok, I am working on this part now, actually -o switch doing nothing...
	cfg->fname_out_bin = 0;
	cfg->fname_out_coe = 0;
	cfg->fname_out_verilog = 0;
	cfg->fname_out_lst = 0;
	*/
	cfg->fname_out_bin = "a.out";
	cfg->fname_out_coe = "a.coe";
	cfg->fname_out_verilog = "a.v";
	cfg->fname_out_lst = "a.lst";

	yy_flex_debug = 0;
  if (argc > 1) {
	  for (int i = 1; i < argc; i++) {
		  if (argv[i][0] == '-') {
			  switch (argv[i][1]) {
			  case 'h':
			  case 'H':
				  printHelp();
				  exit(0); //need to exit here, because of the parsing will starts on standard input later.
				  break;
			  case 'o':
				  actualSwitch = 'o';
				  break;
			  case 'i':
				  actualSwitch = 'i';
				  //cfg->fname_in = &argv[i][1]; //optional: -ifilename.asm
				  break;
			  case 'v':
				  SetDebugForKey("lex", true);
				  SetDebugForKey("rel", true);
				  SetDebugForKey("ext", true);
				  SetDebugForKey("grm", true);
				  SetDebugForKey("pse", true);
				  yy_flex_debug = 1;
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
				case 'o':
				{
					cfg->name_o = argv[i];
				}
				break;
			  }
				
		  }

	  }

  }
  return E_OK;
}

int yywrap() {
	return 1;
}

int searchSymbol(const char *key) {
	for (unsigned int i = 0; i < symbols.size(); i++)
		if (!strcmp(symbols[i], key))
			return i;

	return -1;
}
void addReloc(const char* name, int addr, relocType_en rt) {
	reloc.push_back(UTIL_STRDUP(name));
	relocAddress.push_back(addr);
	relocType.push_back(rt);
}
size_t getRelocs() {
	return reloc.size();
}
int getReloc(int index, const char**name, int* adr, relocType_en* rt) {
	*name = reloc[index];
	*adr = relocAddress[index];
	*rt = relocType[index];
	return 1;
}
void setSymbol(const char* name, int value) {
	int index = searchSymbol(name);
	if (-1 == index) {
		symbols.push_back(name);
		symbolValues.push_back(value);
	}else {
		Failure("Symbol already declared");
	}
}
void setRelocType(relocType_en rt) {
	actualRelocType = rt;
}
int getSymbol(const char* name) {
	int index = searchSymbol(name);
	if (index < 0) {
		addReloc(name, address, actualRelocType);
		return 0; //this will be added to resolved value at pass2, relocation.
	}
	return symbolValues[index];
}