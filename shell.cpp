#include <iostream>
#include <string>
#include <regex.h>
#include <stdlib.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Maximum number of command line arguments to parse
#define MAXARGS 10

using namespace std;

// Builtin regexes
string ECHO = "\\s*echo";
string EXIT = "\\s*exit";
string ALL_ARGS = "\\s*echo\\s*(.*)$";
string NON_BUILTIN= "\\s*(\\S+)\\s+(.*)*$";

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

bool which(char *name)
{
	char *env = getenv("PATH");
	char *pch;
	struct stat filestat;

	pch = strtok(env, ":");
	while (pch != NULL)
	{
		char *tempPath = (char *)malloc(255);
		size_t index = 0;

		strncpy(tempPath, pch, strlen(pch));
		index += strlen(pch);
		strncpy(tempPath + index, "/", 1);
		index += 1;
		strncpy(tempPath + index, name, strlen(name));
		index += strlen(name);
		tempPath[index] = 0;

		// stat the file in the dir
		if(!stat(tempPath, &filestat))
		{
			strcpy(name, tempPath);
			return true;
		}
		free(tempPath);
		pch = strtok(NULL, ":");
	}

	
}

void ex(string command)
{
	regex_t re;
	int status;

	if (regcomp(&re, NON_BUILTIN.c_str(), REG_EXTENDED) != 0)
		return;

	size_t numGroups = re.re_nsub + 1;
	regmatch_t *arguments = (regmatch_t *)malloc(sizeof(regmatch_t) * numGroups);

	status = regexec(&re, command.c_str(), numGroups, arguments, 0);

	if(!status) // If it was located
	{
		char *executable = (char *)malloc(255);
		char *args = (char *)malloc(255);
		for (int x = 1; x < numGroups && arguments[x].rm_so != -1; x++)
		{
			size_t tempSize = (arguments[x].rm_eo - arguments[x].rm_so);
			char *tempStr;

			if(x == 1) // Copy the first group into the executable buffer
				tempStr = executable;
			else // Copy the other group into the args buffer
				tempStr = args;

			strncpy(tempStr, command.c_str() + arguments[x].rm_so, tempSize);
			tempStr[tempSize] = 0;
		}

		// Locate the full path to the executable
		if(!which(executable))
		{
			cout << "Could not locate " << executable << endl;
			return;
		}

		// Execute it.
		execl(executable, executable, args, NULL);
	}

	// Free the newly allocated memory
	regfree(&re);
	free(arguments);
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
			free(tempStr);
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
		done = isMatch(EXIT, command);

		if (isMatch(ECHO, command))
		{
			echo(command);
		}
		else if(!done) // not a builtin
		{
			ex(command);
		}

	}
	while (!done);
}
