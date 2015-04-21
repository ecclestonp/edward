#include "parser.h"

using namespace std;
//args = string []	

Parser::Parser(const char *argStrn)
{
	string temp(argStrn);
	size_t off;
	bool insideQuotes = false;

	// if there is an argument string
	if(argStrn[0])
	{
		do {
			string tstr;

			if(!insideQuotes)
				off = temp.find_first_of(" \"");
			else // If we're inside quotes, only look for next quote
				off = temp.find_first_of("\"");

			// KNOWN BUG: if argStrn == "hello world" -p
			// then it says invalid syntax
			if(off == string::npos && !temp.size())
			{
				cout << "Invalid syntax." << endl;
				// Clear out the args vector
				args.clear();
				break;
			}
			else if(off == string::npos && temp.size())
			{
				off = temp.size();
				if(temp.substr(0, off).size())
					args.push_back(temp.substr(0, off));
				break;
			}

			insideQuotes ^= !temp.substr(off,1).compare("\"");

			if(temp.substr(0, off).size())
				args.push_back(temp.substr(0, off));
			temp = temp.substr(off + 1);
		}while (off != string::npos && temp.size());
	}
}

/*
	brief: This function will create an array of char*'s, and set the "arguments" to the reference to that.
	given: arguments - uninitialized triple pointer
	given: path - the absolute path to the executable being executed
	WARNING: Does not free the memory it allocates
*/
bool Parser::Parse(char ***arguments, string path)
{


	// +1 for null +1 for path in first index = +2
	char **argArray = (char **)malloc((args.size() + 2) * sizeof(char *));

	argArray[0] = (char *)malloc(path.size() + 1);
	strcpy(argArray[0], path.c_str());

	argArray[0][path.size()] = 0; // Add null terminator
	argArray[args.size() + 1] = NULL;


	if(!args.size())
	{
		*arguments = argArray;
		return false;
	}
		
	for(int x = 0; x < args.size(); x++)
	{
		char *tempArg = (char *)malloc(args.at(x).size() + 1);
		strncpy(tempArg, args.at(x).c_str(), args.at(x).size());
		tempArg[args.at(x).size()] = 0;//add null terminator
		argArray[x + 1] = tempArg;
	}

	*arguments = argArray;
	return true;
}
