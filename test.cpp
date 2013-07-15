#define STBASE_DEBUG


#include "/src/stglib/stdio.h"
#include "/src/stglib/sttag.h"


int main(void)
{
	StdOutput<<"Testing:\n";

	StdOutput<<StTag("tag")<<"Text";
}
