#include <iostream>
#include <string>
#include <regex.h>
#include <stdlib.h>
#include <cstring>

// Maximum number of command line arguments to parse
#define MAXARGS 10

using namespace std;

// Builtin regexes
string ECHO = "\\s*echo";
string EXIT = "\\s*exit";
string ALL_ARGS = "\\s*echo\\s*(.*)$";

bool isMatch(string reg, string command)
{
	regex_t re;
	int status;

	if (regcomp(&re, reg.c_str(), REG_EXTENDED|REG_NOSUB) != 0)
	{
		cout << "Issue initializing regex." << endl;
		return 1;
	}
	status = regexec(&re, command.c_str(), (size_t) 0, NULL, 0);
	regfree(&re);

	return !status;
}

int echo(string command)
{
	regex_t re;
	int status;
	size_t numGroups = re.re_nsub + 1;
	regmatch_t *arguments = (regmatch_t *)malloc(sizeof(regmatch_t) * numGroups);

	if (regcomp(&re, ALL_ARGS.c_str(), REG_EXTENDED) != 0)
		return 1;

	status = regexec(&re, command.c_str(), numGroups, arguments, 0);

	if(!status) // If it was located
	{
		for (int x = 1; x < numGroups && arguments[x].rm_so != -1; x++)
		{
			size_t tempSize = (arguments[x].rm_eo - arguments[x].rm_so);
			char *tempStr = (char *)malloc(sizeof(char *) * (tempSize + 1));
			strncpy(tempStr, command.c_str() + arguments[x].rm_so, tempSize);
			tempStr[tempSize] = 0;
			cout << tempStr << endl;
		}
	}

	// Free the newly allocated memory
	regfree(&re);
	free(arguments);
}

int main(void)
{

	string prompt = "> "; // The prompt the user sees
	bool done = false; // If the user wants to exit
	string command; // The command that the user is inputting

	do
	{
		cout << prompt;
		getline(cin, command);

		if (isMatch(ECHO, command))
		{
			echo(command);
		}

		done = isMatch(EXIT, command);
	}
	while (!done);
}
