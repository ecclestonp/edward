#include <stdio.h>
#include "parser.h"

// Maximum number of command line arguments to parse
#define MAXARGS 10

using namespace std;

// Builtin regexes
string CD = "\\s*cd";
string EXIT = "\\s*exit";
string NON_BUILTIN= "\\s*(\\S+)\\s*(.*)*$";
string Q_COMPILE= "(\\S+)(\\.cpp$|\\.c$)";

string PWD = getenv("HOME");

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

//check if file name begins with / or .
	if (name[0] == '/' || name[0] == '.') {
		//if it does, check if the file exists
		if (check_exists(name)) {
			*out = name;
			return true;
		}

		// if file does not exist
		return false;
	}

	//if file was not found, search the path for file
	envp = getenv("PATH");

	if (envp == NULL)
		//path was not found
		return false;

	//save the path
	env = envp;

	off = env.find_first_of(':');

	//search through all directories in the PATH for the file
	while (off != string::npos) 
	{
		string tenv;

		off = env.find_first_of(':');

		//tenv holds the first directing in the "PATH"
		tenv = env.substr(0, off);

		//lop off the first directory
		env = env.substr(off + 1);

		//append the name of the file to the path directory you are in 
		tenv.append("/");
		tenv.append(name);

		//check of file exists in that directory
		if (check_exists(tenv.c_str())) 
		{
			*out = tenv;
			return true;
		}
	}

	//file not found anywhere
	return false;
}

void ex(string command)
{
	regex_t re;
	int status;
	bool QC_flag;

	if (regcomp(&re, NON_BUILTIN.c_str(), REG_EXTENDED) != 0)
		return;

	//re_nsub returns how many groups there are in re
	//add 1 for null
	size_t numGroups = re.re_nsub + 1;
	regmatch_t *arguments = (regmatch_t *)malloc(sizeof(regmatch_t) * numGroups);

	//regexec gets the substring offsets and stores it in arguments
	status = regexec(&re, command.c_str(), numGroups, arguments, 0);

	if(!status) // If regexec was successful
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
		//Q_COMPILE = "(\\S+)(\\.cpp$|\\.c$)"
		if (regcomp(&re, Q_COMPILE.c_str(), REG_EXTENDED) != 0)
			return; 

		//get the number of groups there are 
		numGroups = re.re_nsub +1;
		//allocate enough memory for each group in the string array
		regmatch_t *QC_Offsets = (regmatch_t *)malloc(sizeof(regmatch_t)*numGroups);
		
		//offsets will be saved in QC_Offsets
		//status = 0 if matches found
		//there will be two capture groups (\\S+)(\\.cpp$|\\.c$)
		status = regexec(&re, path.c_str(), numGroups, QC_Offsets, 0);

		if(!status) //regex works then path ends in .cpp or .c
		{
			QC_flag = true; //enter quick compile mode
		}
		
		char **arguments;
		//Parser constructor
		Parser p(args);

		//pid_t pid = fork();
		//if(!pid) // child
		// Execute it.
		// Parse the arguments
		(p.Parse(&arguments, path));

		char *environ[] = { NULL };

		if (!QC_flag)
		{
			char *environ[] = { NULL };
			execve(path.c_str(), arguments, environ);
		}
		else //QC_flag true
		{
		// get arguments for the compiler
			char **c_args = (char**)malloc(sizeof(char *) * numGroups);
			c_args[0] = (char*)malloc(path.size() + 1);
			strcpy(c_args[0], path.c_str());
		// get path of compiler
			char *compiler =(char*)malloc(sizeof(char)* 3);
			strcpy(compiler, "g++");
			//store the  path to the compiler in "path"
			if(which(compiler, &path))
			{
				// compile
				execve(path.c_str(), c_args, environ);
			}

		// set up arguments for execution
			


		// execute source file




			// char **path_as_arg;
			// char *compiler;
			// strcpy(*path_as_arg, path.c_str());
			// strcpy(compiler, "g++");
			// string CompilerPath; //to save the path of the compiler
			
			// // Locate the full path to the executable and save the path to QCpath if it exists
			// if(!which(compiler, &CompilerPath))
			// {
			// 	cout << "Could not locate compiler" << endl;
			// 	return;
			// }

			// //Param CompilerPath -- gcc/g++
			// //Param path -- path to sourcefile	
			// //if compiling is successful then execute binary file 
			// if(!execve(CompilerPath.c_str(), path_as_arg, environ)) 
			// {	
	
			// 	execve(path.substr((size_t)QC_Offsets[0].rm_so, (size_t)QC_Offsets[0].rm_eo - QC_Offsets[0].rm_so).c_str(), arguments, environ);
			// }			
		}//end of Qcompile
	}

	// Free the newly allocated memory
	regfree(&re);
	//free(arguments);
}

void cd(string command)
{
	cout << "home dir is " << PWD << endl;
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

        if (isMatch(CD, command))
		{
			cd(command);
		}
		else if(!done) // not a builtin
		{
			ex(command);
		}

	}
	while (!done);
}


