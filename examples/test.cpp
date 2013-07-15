// Example of using StBase's debugging output

//#define STBASE_DEBUG
// optional:
//#define STBASE_DEBUG_FILE "debug.txt"

//#define STARRAY_DEBUG
//#define STARRAY_DEBUG_TRACE

#include "/src/stglib/stglib.h"
#include "/src/stglib/stdio.h"

int main(void)
{
	StFile temp("temp.txt"); //,StFileOpenCreateOrAppend);
	temp<<"Hello World\n";
	if (temp._Error)
		exit(temp._ShowError(StdOutput));

	return(0);
}

