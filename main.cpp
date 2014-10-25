#include <string>
#include <vector>
#include <string.h>
#include <stdlib.h>
using namespace std;
#include "includes.h"

vector <string> vsIncludePath;		//globally accessible vector of directories in INCLUDE env var or -I arg
vector <string> vsInputFiles;		//files to process - output is in makefile dependency format
vector <symbol> vsymSwap;
vector <symbol> vsymDefinitions;
ParseState ps;

static int parse_args(int ac, char **av);
static void setupInclude(void);

static void usage(char *cp = NULL)
{
	fprintf (stderr, "Includes generates a dependency listing for your source files\n");
	fprintf (stderr, "usage: includes <-X> <-I INCLUDEPATH> file1.cpp file2.c .. fileN.cpp\n");
	fprintf (stderr, "-I INCLUDEPATH\tdirectory to search for included files\n");
	fprintf (stderr, "-X\tignore standard places\n");
	fprintf (stderr, "-S swap\tsym1=sym2\te.g., swap sym1 for sym2 in output\n\t\t\tQuote entire S=D clause if needed.\n");
	fprintf (stderr, "-D define SYMBOL=definition\te.g., #define SYMBOL\n");
	if (cp)
		fprintf (stderr, "includes: %s\n", cp);
	exit(1);
}

int main (int ac, char **av)
{
	setupInclude();
	getchar();
	if (parse_args(ac, av))
		usage(NULL);

	//following for debuggery
	if (false && ps.bDebug) {
		for (unsigned i = 0; i < vsIncludePath.size(); i++) {
			printf ("%s\n", vsIncludePath[i].c_str());
		}
	}

	exit(processFileList(vsInputFiles));
}

static void setupInclude(void)
{
	char seps[]   = ";";
	char *token, *next_token;

	char *cpInclude;
	size_t	 nBytes;
	if(_dupenv_s(&cpInclude, &nBytes, "INCLUDE"))
		return;
	// Establish string and get the first token:
	token = strtok_s( cpInclude, seps, &next_token );
	while( token != NULL ) 
	{
	  vsIncludePath.push_back(token);

	  // Get next token: 
	  token = strtok_s( NULL, seps, &next_token );
	}
}

static int parse_args(int ac, char **av)
{
	enum NEXTARG {_none = 0, _definition, _include, _swap, _debug};
	NEXTARG next = _none;
	int nErrs = 0;
	
	if (ac < 2)
		usage();

	//start with first arg not program name
	for (int i = 1; i < ac; i++)
	{
		if (*av[i] == '-' || *av[i] == '/')
		{
			if (strlen(av[i]) != 2) usage(); //all flags by themselves

			switch (av[i][1]) {
				case 'I':
					//next arg is path for std include vector
					next = _include;
					continue;
				case 'X':
					{
						//must be first argument
						if (i != 1) usage("/X must be the first argument if it is used\n");
						//disregard default places
						//get rid of the include paths we've already collected from ENV
						size_t size = vsIncludePath.size();
						for (unsigned j = 0; j < size; j++) 
						{
							vsIncludePath.pop_back();
						}
					}
					continue;
				case 'd':
					ps.bDebug = true;
					continue;
				case 'S':
					next = _swap;
					continue;
				case 'D':
					next = _definition;
					continue;
				default:
					usage(); //no return
			}
		}

		switch (next) {
			case _include:
				vsIncludePath.push_back(av[i]);
				next = _none;
				break;
			case _swap:
			case _definition:
				{
					symbol *sy;
					//create new symbol by
					//1)parsing for '=' sign
					string s(av[i]);
					size_t u = s.find('=');
					if (u == 0) 
						usage();
					if (u == s.npos) {
						//no equal sign
						if (next == _swap) //bugbug: should we accept to eliminate chars?
							usage(); //an error if trying to modify output
						else {
							sy = new symbol(s, "1");
							vsymDefinitions.push_back(*sy); //but acceptable here
						}
					} else {
						if ((u + 1) == s.length())
							usage(); //e.g., /-[SD] foo=/ second symbol required

						string scratch1 = s.substr(0, u) ;
						string scratch2 = s.substr(u+1, s.length()-u);
						if (next == _swap) {
							sy = new symbol(scratch1, scratch2);
							vsymSwap.push_back(*sy);
						}
						else {
							sy = new symbol(scratch1, scratch2);
							vsymDefinitions.push_back(*sy);
						}
					}
					next = _none;
					break;
				}
			case _none:
				//gentlemen, we have a file argument
				vsInputFiles.push_back(av[i]);
				break;
			default:
				usage("unknown error in argument parsing");
		}
	}
	return nErrs;
}

