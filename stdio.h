// stg template library
// stdio routines
//
// purpose is to map stg io methods onto std_in, std_out, std_err paths


#ifndef STGLIB_STDIO
#define STGLIB_STDIO

#include "/src/stglib/stcore.h"
#include "/src/stglib/stfile.h"
#include "/src/stglib/stdump.h"

#pragma message("using stdio.h")

// and map global instances of StFile onto standard io paths
StFile StdInput(stdin);
StFile StdOutput(stdout,StFileOpenCreateOrAppend);
StFile StdError(stderr,StFileOpenCreateOrAppend);

StDump StdOutDump(StdOutput);

#endif
