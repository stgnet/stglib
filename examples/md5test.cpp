#include "/src/stglib/stglib.h"
#include "/src/stglib/stdio.h"
#include "/src/stglib/stfile.h"
#include "/src/stglib/stmd5.h"

int main(int argc,char **argv)
{
	if (!*++argv)
	{
		"use: md5test (file)\n">>StdOutput;
		exit(1);
	}

	StFile file(*argv);

	StMd5 md5;

	file>>md5;

	StdOutDump<<md5;

	return(0);
}
