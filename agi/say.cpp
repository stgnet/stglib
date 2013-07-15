#include "/src/stglib/stdio.h"
#include "/src/stglib/stfilter.h"

class JustText:public StFilter
{
public:
	StSize _Filter(StByte *data,StSize size)
	{
		StByte *end=data+size;
		if (size)
			++data;	// skip leading /
		while (data<end)
		{
			if (*data>='A' && *data<='Z')
				_Output(data,1);
			else
			if (*data>='a' && *data<='z')
				_Output(data,1);
			else
			if (*data>='0' && *data<='9')
				_Output(data,1);
			else
			if (*data=='.')
				_Output(data,1);
			else
				_Output((StByte*)" ",1);
			++data;
		}
		return(size);
	}
};

class FileName:public StFilterByte
{
public:
	StByte _Filter(StByte b)
	{
		if (b==' ')
			return('+');
		
		return(b);
	}
};
		

int main(void)
{
	StdOutput._FlushAfterWrite=1;
	StdOutput<<"Content-type: text/plain\n\n";
	StdOutput<<"Test:\n";

	StString text;
	StString file;
	StString temp;
	JustText filter;
	FileName space2plus;
	getenv("REQUEST_URI")>>filter>>text;
	file<<"/tmp/say-";
	text>>space2plus>>file;
	file<<".wav";

	"/tmp/tmp-">>temp;
	text>>space2plus>>temp;
	".wav">>temp;

	StString cmd;
	cmd<<"/usr/local/bin/swift -o "<<temp<<" '"<<text<<"'";
	cmd<<" >/dev/null";
StdOutput<<"cmd="<<cmd<<"|\n";
//	system(cmd);


	StdOutput<<"Content-type: audio/x-wav\x0d\x0a\x0d\x0a";
	StFile wav(file);
	StBuffer buf;
	wav>>buf;
	buf>>StdOutput;

	return(0);
}
