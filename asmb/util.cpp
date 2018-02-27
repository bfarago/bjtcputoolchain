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

static vector<const char*> reloc;
static vector<int> relocAddress;

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
  printf("+++ (%s): %s%s", key, buf, buf[strlen(buf)-1] != '\n'? "\n" : "");
}

void ParseCommandLine(int argc, char *argv[]) {
  if (argc == 1)
    return;
  
  if (strcmp(argv[1], "-d") != 0) { // first arg is not -d
    printf("Incorrect Use:   ");
    for (int i = 1; i < argc; i++) printf("%s ", argv[i]);
    printf("\n");
    printf("Correct Usage:   -d <debug-key-1> <debug-key-2> ... \n");
    exit(2);
  }

  for (int i = 2; i < argc; i++)
    SetDebugForKey(argv[i], true);
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
void addReloc(const char* name, int addr) {
	reloc.push_back(name);
	relocAddress.push_back(addr);
}
int getRelocs() {
	return reloc.size();
}
int getReloc(int index, const char**name, int* adr) {
	*name = reloc[index];
	*adr = relocAddress[index];
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
int getSymbol(const char* name) {
	int index = searchSymbol(name);
	if (index < 0) {
		addReloc(name, address);
		return -1;
	}
	return symbolValues[index];
}