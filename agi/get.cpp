#include "/src/stglib/stdio.h"
#include "/src/stglib/sthttp.h"

int main(int argc, char **argv)
{
	if (!*++argv)
	{
		StdOutput<<"use: get (url)\n";
		return(1);
	}

	StHttp http;

	http.Get(*argv);

	if (http._Error)
		exit(http._ShowError(StdOutput));

	StBuffer text;

	// for now, output must be to buffer
	// because sock won't transfer
	http>>text;

	StFile file("output.wav");

//	text>>StdOutput;
	text>>file;

/* how we can see the header info:
	http._Response._Header>>StdOutput;

	StdOutput<<"Code="<<http._Response._Code<<"|\n";
	StdOutput<<"Mesg="<<http._Response._Message<<"|\n";

	StBoxRef<StPairEntry> scan(http._Response);
	while (++scan)
	{
		StdOutput<<scan->Name<<"="<<scan->Value<<"\n";
	}
*/
	return(0);
}

