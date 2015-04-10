#include <iostream>
#include <string>
#include <regex.h>

using namespace std;

// Builtin regexes
string ECHO = "\\s*echo";
string EXIT = "\\s*exit";

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
			cout << "That was an echo!" << endl;
		}

		done = isMatch(EXIT, command);
	}
	while (!done);
}
