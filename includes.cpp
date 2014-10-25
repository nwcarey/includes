#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <string.h>
#include <io.h>
using namespace std;
#include "includes.h"

vector <LineRecord::FileRecord> vInputFileRec;
vector <LineRecord::FileRecord> vIncludedFileRec;
static int nErrors = 0;

LineRecord::LineRecord(string inString) : s(inString)
{
	if (strstr(s.c_str(), cpInclude) != NULL)
	{
		fIndex = lookupIncludedFile(s);
		if (fIndex == (LookupError))
			nErrors++;
		kind = _include;
	}
	else if (strstr(s.c_str(), cpDefine) != NULL)
	{
		//create new Definition
		dp = new Definition (s);
		kind = _define;
		dp->bDefining = true;
	}
	else if (strstr(s.c_str(), cpUndef) != NULL)
	{
		//create new Definition
		dp = new Definition (s);
		kind = _define;
		dp->bDefining = false;
	}
	else 
	{
		//some sort of control directive - #if, #ifdef, #elif, #endif, etc.
		cp = new Conditional(s);
		kind = _conditional;
	}
}

LineRecord::FileRecord::FileRecord(string sFile, string sPath, bool bAngle = true) : filename(sFile), path(sPath), bDefaultPath(bAngle)
{
	//could be an input file - if so we have verified existence and line doesn't start with '#'
	//	in this case filename and path are cool
	//else 
	//	parse whether quoted by "" or <>
	//	find actual file
	//	error if not found
	//	create entry in universal dictionary of included files - if not already there
	
	
}

LineRecord::Conditional::Conditional(string inString)
{
	text = inString; //use string copy constructor
	if (strstr(inString.c_str(), cpIf))
	{
		cType = _if;
	}
	//etc. 
	//todo: set cType for other kinds
}

LineRecord::Definition::Definition(string inString)
{
	text = inString; //use string copy constructor
	if (strstr(inString.c_str(), cpUndef))
		bDefining = false;
	else
		bDefining = true;
}


LineRecord::FileRecord *processFile(ifstream &in, string& sFile, string& sCurDir)
{
	LineRecord *lrp;
	LineRecord::FileRecord *frptr = new LineRecord::FileRecord(sFile, sCurDir);
	string s;
	while (getline(in, s))
	{
		if (s.c_str()[0] == '#') 
		{
			if (((strstr(s.c_str(), cpInclude) != NULL)
				|| (strstr(s.c_str(), cpDefine) != NULL)
				|| (strstr(s.c_str(), cpUndef) != NULL)
				|| (strstr(s.c_str(), cpUndef) != NULL))
				&& (strstr(s.c_str(), cpPragma) == NULL)) //not for pragmas
			{
				//create new FileRecord
				lrp = new LineRecord(s);
				frptr->v.push_back(*lrp);
			}
		}
	}
	return frptr;
}

///processFileList (sInput - vector of filenames)
///return: number of errors
int processFileList(vector<string> sInput)
{
	size_t	 tsize;
	LineRecord::FileRecord *frptr;
	for (unsigned i = 0; i < vsInputFiles.size(); i++) 
	{
		//printf ("%s\n", vsInputFiles[i].c_str());
		ifstream in (vsInputFiles[i].c_str());
		//does file exist?
		if(!in) 
		{
		  fprintf(stderr, "Could not open file %s\n", vsInputFiles[i].c_str());
		  nErrors++;
		  return NULL;
		}
		//does it have a path at the beginning? (this changes where we start search for "file.h")
		if ((tsize = vsInputFiles[i].find_last_of("\\")) != vsInputFiles[i].npos)
		{
			ps.sCurDir = vsInputFiles[i].substr(0, tsize+1); //use copy constructor
			ps.sFile = vsInputFiles[i].substr(tsize+1, vsInputFiles[i].length() - tsize-1);
		}
		else 
		{
			ps.sFile = vsInputFiles[i];
			ps.sCurDir = ""; //length = 0
		}
		if (false && ps.bDebug) 
		{
			fprintf (stdout, "file:%s folder:%s \n", ps.sFile.c_str(), ps.sCurDir.c_str());
		}
		//
		frptr = processFile(in, ps.sFile, ps.sCurDir);
		if (frptr == NULL)
			continue;
		vInputFileRec.push_back(*frptr);
		//
		if (false && ps.bDebug) 
		{
			printf ("%p:%s:size of frptr->v is %d\n", &frptr->v, frptr->filename.c_str(), frptr->v.size());
			for (unsigned k = 0; k < frptr->v.size(); k++) 
			{
				fprintf (stdout, "\t%s\n", frptr->v[k].s.c_str());
			}
			fprintf (stdout, "\n");
		}
		
	}
	if (ps.bDebug) {
		if (true) 
		{
			printf ("size of vsymSwap is %d\n", vsymSwap.size());
			for (unsigned k = 0; k < vsymSwap.size(); k++) 
			{
				fprintf (stdout, "[%s] = [%s]\n", vsymSwap[k].getName().c_str(), vsymSwap[k].getValue().c_str());
			}
			printf ("size of vsymDefinitions is %d\n", vsymDefinitions.size());
			for (unsigned k = 0; k < vsymDefinitions.size(); k++) 
			{
				fprintf (stdout, "[%s] = [%s]\n", vsymDefinitions[k].getName().c_str(), vsymDefinitions[k].getValue().c_str());
			}
		}
		printf ("size of vInputFileRec is %d\n", vInputFileRec.size());
		for (unsigned i = 0; i < vInputFileRec.size(); i++)
		{
			frptr = &vInputFileRec[i];
			printf ("%p:%s:size of frptr->v is %d\n", &frptr->v, frptr->filename.c_str(), frptr->v.size());
			fprintf (stdout, "%s%s%s:\n", frptr->path.c_str(), "", frptr->filename.c_str());
			for (unsigned k = 0; k < frptr->v.size(); k++) 
			{
				fprintf (stdout, "\t%s\n", frptr->v[k].s.c_str());
			}
			fprintf (stdout, "\n");
		}

	}
	printFileList();
	return nErrors;
}

/// lookupIncludedFile(string s) - s - complete input line- "#include <string>"
/// caller handles all errors
static int lookupHelper(string s1, string filename, string path)
{
	ifstream in (s1.c_str());
	if(!in) 
	{
	  fprintf(stderr, "Could not open file %s\n", s1.c_str());
	  nErrors++;
	  return LookupError;
	}
	LineRecord::FileRecord *frptr = processFile(in, filename, path);
	if (frptr == NULL)
		return LookupError;
	vIncludedFileRec.push_back(*frptr);
	return vIncludedFileRec.size() - 1;
}

int lookupIncludedFile(string s)
{
	//determine how quoted
	bool	bAngleBracket;
	int n = s.find_first_of("<\"", strlen(cpInclude));
	if (n == s.npos) return LookupError; //bad string
	
	//still here?
	bAngleBracket = (s.at(n) == '<');
	int x = s.length() - 2 - n;
	//create filename from quoted string
	string filename = s.substr(n+1, x);
	//cout << "starting " << filename <<endl;

	//have we already seen this one?
	for (unsigned i = 0; i < vIncludedFileRec.size(); i++)
	{
		//if you can find it return the index to the entry
		if (vIncludedFileRec[i].filename == filename) 
		{
			//cout << "found in stash: " << filename << endl;
			return i;
		}
	}

	//still here? the examine each directory in turn;
	if (bAngleBracket == true)
	{
		for (unsigned i = 0; i < vsIncludePath.size(); i++)
		{
			//see if file exists at this location by concatenating path and filename together
			string cont =
				(vsIncludePath[i].c_str()[vsIncludePath[i].length()-1] == '\\') ? "" : "\\";
			string s1 = vsIncludePath[i] + cont + filename;
			//cout << "testing s1: " << s1 << endl;
			if (_access(s1.c_str(), 00) != -1) {
				//cout << "found:" << s1 << endl;
				return lookupHelper(s1, filename, vsIncludePath[i]);
				break;
			}
		}
	} else {
		//#include "thing.h" - lookup in curdir
		string cont;
		if (ps.sCurDir.length() == 0)
			cont = "";
		else {
			cont =
				(ps.sCurDir.c_str()[ps.sCurDir.length()-1] == '\\') ? ps.sCurDir : ps.sCurDir + "\\";
		}
		string s1 = cont + filename;
			
		if (_access(s1.c_str(), 00) != -1) {
			return lookupHelper(s1, filename, ps.sCurDir);
		} else {
			return LookupError;
		}
	}

	//still here? then the file was not found
	//caller deals with this eventuality
	return LookupError;
}

void sub_symbols(string &s)
{
	for (unsigned k = 0; k < vsymSwap.size(); k++) 
	{
		//returns the first occurrence of str within the current string, 
		//starting at index, string::npos if nothing is found,
		//size_type find( const string& str, size_type index );
		size_t i;
		while ((i = s.find (vsymSwap[k].getValue(), 0)) != s.npos)
		{
			//replaces up to num1 characters of the current string (starting at index1) 
			//with up to num2 characters from str beginning at index2
			//string& replace( size_type index1, size_type num1, const string& str, size_type index2, size_type num2 );
			s.replace(i, vsymSwap[k].getValue().length(), vsymSwap[k].getName(), 0, vsymSwap[k].getName().length());
		}
	}
}
void printHelper1(LineRecord::FileRecord fr, vector <string> &vsO)
{
	unsigned u;
	vector <string>::iterator Iter;
	for (unsigned k = 0; k < fr.v.size(); k++)
	{
		if (fr.v[k].kind == _include)
		{
			register int f = fr.v[k].fIndex;
			bool bPushBack = true;
			string s;
			s = (vIncludedFileRec[f].path.length() > 0) ?	vIncludedFileRec[f].path + "\\" : "";
			s += vIncludedFileRec[f].filename;
			sub_symbols(s);
			for (Iter = vsO.begin(); Iter != vsO.end(); Iter++)
			{
				if (s == *Iter)
					return; //nothing to do, been here already
				//still here?
				if (s < *Iter) {
					vsO.insert(Iter, s);
					bPushBack = false;
					break;
				}
			}
			if (bPushBack)
				vsO.push_back(s); //at the end

			printHelper1(vIncludedFileRec[f], vsO);
		}
	}
}
void printHelper2(vector <string> vsOutput, int &lineLength)
{
	for (unsigned u = 0; u < vsOutput.size(); u++)
	{
		string s = vsOutput[u];
		if (lineLength + s.length() > 78) {
			cout << " \\\n\t";
			lineLength = 8;
		}
		cout << s << " ";
		lineLength += s.length() + 1;
	}
}

void printFileList(void)
{
	vector <string> vsOutput;
	for (unsigned i = 0; i < vInputFileRec.size(); i++)
	{
		string temp = vInputFileRec[i].filename;
		sub_symbols(temp);
		int lineLength = temp.length() + 9;
		cout << temp << ":\t";
		printHelper1 (vInputFileRec[i], vsOutput);
		printHelper2(vsOutput, lineLength);
		cout << endl;
	}
}
			

//tbc TODO:
//
/*
	- improper output format currently - should be thing.obj:\tthing.c ...etc
	*/