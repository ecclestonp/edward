#include <iostream>
#include <string>

using namespace std;
 
int main(void)
{
	string prompt = "> "; // The prompt the user sees
	bool done = false; // If the user wants to exit
	string command; // The command that the user is inputting

	do
	{
		cout << prompt;
		getline(cin, command);


		done = (command.compare("exit") == 0);
	}
	while (!done);
}


