
class LineRecord
{
public:
	KIND kind;
	LineRecord(string);
	virtual void process();
private:
	string s;
};

class FileRecord : public LineRecord
{
	vector <LineRecord> v;
public:
	void process();
	FileRecord(string fileName);
};

class Conditional : public LineRecord
{
	CONDITIONAL cType;
public:
	void process();
	Conditional(string cString);
};

class Definition : public LineRecord
{
	bool	bDefining; //false if #undef directive
public:
	void process();
	Definition (string dString);
};

