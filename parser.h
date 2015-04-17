#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#ifndef __PARSER_H__
#define __PARSER_H__

using namespace std;

#define ALL_ARGS "\\s*echo\\s+(.*)$"

class Parser {
	private:
		vector<string> args;

	public:
		Parser(char *argStrn);
		bool Parse(char ***arguments);
};

#endif
