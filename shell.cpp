#include <stdio.h>
#include "parser.h"

// Maximum number of command line arguments to parse
#define MAXARGS 10

using namespace std;

// Builtin regexes
string ECHO = "\\s*echo";
string EXIT = "\\s*exit";
string NON_BUILTIN= "\\s*(\\S+)\\s+(.*)*$";
string Q_COMPILE= "(\\S+)(\\.cpp$|\\.c$)";

bool which(char *name, string *out);

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

bool check_exists(const char *name)
{
	struct stat filestat;

	return (stat(name, &filestat) == 0) ? true : false;
}

bool which(char *name, string *out)
{
	size_t off;
	string env;
	char *envp;

	if (name[0] == '/' || name[0] == '.') {
		if (check_exists(name)) {
			*out = name;
			return true;
		}

		return false;
	}

	envp = getenv("PATH");

	if (envp == NULL)
		return false;

	env = envp;

	off = env.find_first_of(':');

	while (off != string::npos) {
		string tenv;

		off = env.find_first_of(':');

		tenv = env.substr(0, off);
		env = env.substr(off + 1);

		tenv.append("/");
		tenv.append(name);

		if (check_exists(tenv.c_str())) {
			*out = tenv;
			return true;
		}
	}

	return false;
}

void ex(string command)
{
	regex_t re;
	int status;
	bool QC_flag;

	if (regcomp(&re, NON_BUILTIN.c_str(), REG_EXTENDED) != 0)
		return;

	size_t numGroups = re.re_nsub + 1;
	regmatch_t *arguments = (regmatch_t *)malloc(sizeof(regmatch_t) * numGroups);

	status = regexec(&re, command.c_str(), numGroups, arguments, 0);

	if(!status) // If it was located
	{
		char *executable = (char *)malloc(255);
		char *args = (char *)malloc(255);
		string path;
		for (int x = 1; x < numGroups && arguments[x].rm_so != -1; x++)
		{
			size_t tempSize = (arguments[x].rm_eo - arguments[x].rm_so);
			char *tempStr;

			if(x == 1) // Copy the first group into the executable buffer
				tempStr = executable;
			else	   // Copy the other group into the args buffer
				tempStr = args;

			strncpy(tempStr, command.c_str() + arguments[x].rm_so, tempSize);
			tempStr[tempSize] = 0;
		}
		
		// Locate the full path to the executable
		if(!which(executable, &path))
		{
			cout << "Could not locate " << executable << endl;
			return;
		}

		//If .cpp or .c then QuickCompile

		if (regcomp(&re, Q_COMPILE.c_str(), REG_EXTENDED) != 0)
			return; 
		regmatch_t *QC_Offsets = (regmatch_t *)malloc(sizeof(regmatch_t)*2);
		
		status = regexec(&re, path.c_str(), (size_t) 2, QC_Offsets, 0);

		if(!status) //If path ends in .cpp or .c
		{
			QC_flag = true; //enter quick compile mode
		}
		
		char **arguments;
		Parser p(args);

		//pid_t pid = fork();
		//if(!pid) // child
		// Execute it.
		// If we were able to parse the arguments
		if(p.Parse(&arguments, path))
		{
			char *environ[] = { NULL };

			if (!QC_flag)
			{
				cout << "About to execute: " << path.c_str() << endl;
				execve(path.c_str(), arguments, environ);
			}
			else
			{
				char **path_as_arg;
				char *compiler;
				strcpy(*path_as_arg, path.c_str());
				strcpy(compiler, "g++");
				string CompilerPath; //to save the path of the compiler
				
				// Locate the full path to the executable and save the path to QCpath if it exists
				if(!which(compiler, &CompilerPath))
				{
					cout << "Could not locate compiler" << endl;
					return;
				}

				//Param CompilerPath -- gcc/g++
				//Param path -- path to sourcefile	
				//if compiling is successful then execute binary file 
				if(!execve(CompilerPath.c_str(), path_as_arg, environ)) 
				{	
		
					execve(path.substr((size_t)QC_Offsets[0].rm_so, (size_t)QC_Offsets[0].rm_eo - QC_Offsets[0].rm_so).c_str(), arguments, environ);
				}			
			}

		}
		else
		{
			cout << "Unable to execute because you fucked up." << endl;
		}
	}

	// Free the newly allocated memory
	regfree(&re);
	//free(arguments);
}

int echo(string command)
{
	regex_t re;
	int status;
	size_t numGroups;
	regmatch_t *arguments;


	if (regcomp(&re, ALL_ARGS, REG_EXTENDED) != 0)
		return 1;

	numGroups = re.re_nsub + 1;
	arguments = (regmatch_t *)malloc(sizeof(regmatch_t) * numGroups);

	if (arguments == NULL) {
		cerr << "malloc broke" << endl;
		return 1;
	}

	status = regexec(&re, command.c_str(), numGroups, arguments, 0);

	if(!status) // If it was located
	{
		for (int x = 1; x < numGroups && arguments[x].rm_so != -1; x++)
			cout << command.c_str() + arguments[x].rm_so << endl;
	}

	// Free the newly allocated memory
	regfree(&re);
	free(arguments);
	return 0;
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
