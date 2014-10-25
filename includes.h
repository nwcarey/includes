char * const cpInclude = "#include";
char * const cpDefine  = "#define";
char * const cpUndef	= "#undef";
char * const cpPragma	= "#pragma";
char * const cpIf		= "#if";
char * const cpElse		= "#else";
char * const cpElif		= "#elif";
char * const cpEndif	= "#endif";

enum KIND {_define, _include, _conditional};
enum CONDITIONAL {_if, _else, _elif, _endif};

extern vector <string> vsIncludePath;		//globally accessible vector of directories in INCLUDE env var or -I arg
extern vector <string> vsInputFiles;	//what was given on the command line

class LineRecord
{
public:
	LineRecord(string);
	struct FileRecord;
	struct Conditional;
	struct Definition;
	KIND kind;
	friend int processFileList(vector <string>);
	friend int lookupIncludedFile(string);
	friend void printFileList(void);
	friend void printHelper1(LineRecord::FileRecord, vector <string> &);
private:
	string s;
	union {
		int fIndex;//lookup for FileRecord of included file
		//FileRecord  *frp;
		Conditional	*cp;
		Definition  *dp;
	};
};

enum FTYPE {_cmdline, _quote, _bracket};

struct LineRecord::FileRecord {
	string filename;	//filename - search by this
	string path;	//where found
	FTYPE	ftype;
	bool	bDefaultPath; //true if found within <> brackets, false if ""
	vector <LineRecord> v;
	FileRecord(string sFile, string sPath, bool bAngle);
	~FileRecord() {};
	friend	string findPath(char *);
};

extern vector <LineRecord::FileRecord> vInputFileRec;
extern vector <LineRecord::FileRecord> vIncludedFileRec;
;
struct LineRecord::Conditional {
	string text;
	CONDITIONAL cType;
	Conditional(string s);
	~Conditional() {};
};
struct LineRecord::Definition {
	string	text;
	bool	bDefining; //false if #undef directive
	Definition(string s);
	~Definition() {};
};

class symbol {
private:
	string name;
	string value;
public:
	bool CompareName (string);
	bool CompareValue (string);
	symbol(string, string);
	~symbol();
	string getName(void);
	string getValue(void);
};

extern vector <symbol> vsymSwap;
extern vector <symbol> vsymDefinitions;
extern symbol *findEntryByName(vector<symbol>, string n);
extern symbol *findEntryByValue(vector<symbol>, string v);

class ParseState {
public:
	string	sCurDir;
	string  sFile;
	bool	parsing;
	bool	bDebug;
};

extern ParseState ps;
const int LookupError = -1;
	

	

