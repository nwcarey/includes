#include <string>
#include <vector>
#include <string.h>
using namespace std;
#include "includes.h"

symbol::symbol(std::string n, std::string v) : name(n), value(v)
{
}

symbol::~symbol()
{
}

bool symbol::CompareValue(string s)
{
	//return (strcmp(value.c_str(), s.c_str()));
	return (value == s);
}

bool symbol::CompareName(string s)
{
	//return (strcmp(name.c_str(), s.c_str()));
	return (name == s);
}

string symbol::getName(void)
{
	return name;
}

string symbol::getValue(void)
{
	return value;
}

symbol *findEntryByName(vector<symbol> vsym, string n)
{
	for (unsigned i = 0 ; i < vsym.size(); i++)
		if (vsym[i].CompareName(n))
				return &vsym[i];
	return NULL;
}

symbol *findEntryByValue(vector<symbol>vsym, string v)
{
	
	for (unsigned i = 0 ; i < vsym.size(); i++)
		if (vsym[i].CompareValue(v))
				return &vsym[i];
	return NULL;
}

