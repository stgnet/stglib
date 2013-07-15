// Example of using StBase's debugging output

#define STBASE_DEBUG
// optional:
//#define STBASE_DEBUG_FILE "debug.txt"

#define STARRAY_DEBUG
#define STARRAY_DEBUG_TRACE

#include "/src/stglib/stglib.h"
#include "/src/stglib/stdio.h"

int main(void)
{
	"IO Debug Test\n">>StdOutput;

	StFile temp("temp.txt");

	temp<<"Hello World\n";

	return(0);
}

