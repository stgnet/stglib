// stg template library
// Platform Identification
// Also loads platform-specific files to workaround differances

// NOTE: If this is included (manually) _before_ stcore.h, then it identifies
// the platform using an error output from the compiler

#ifndef STGLIB_STPLATFORM
#define STGLIB_STPLATFORM
#pragma message("using stgplatform.h")


// make sure the compiler has defined some _basic_ requirements
#ifndef __LINE__
#error WARNING: __LINE__ NOT DEFINED BY COMPILER
#endif

#ifndef __DATE__
#error WARNING: __DATE__ NOT DEFINED BY COMPILER
#endif

// perform varoius platform checks, load platform specific files
#ifdef WIN32
#ifndef STGLIB_STGLIB
#error PLATFORM IS WINDOWS (stp_win32.h)
#endif
#include "/src/stglib/stp_win32.h"

#else
#ifdef __TURBOC__
#ifndef STGLIB_STGLIB
#error PLATFORM IS TURBOC (stp_turboc.h)
#endif
#include "/src/stglib/stp_turboc.h"

#else
#ifdef __CYGWIN32__
#ifndef STGLIB_STGLIB
#error PLATFORM IS CYGWIN (stp_cygwin.h)
#endif
#include "/src/stglib/stp_cygwin.h"

// fallthrough: presume a *nix environment of some sort, 
// probably GCC based, at least posix compatible
#else
#ifndef STGLIB_STGLIB
#error PLATFORM IS POSIX (stp_posix.h)
#endif
#include "/src/stglib/stp_posix.h"

// lots of endif's to finish out #if/#else's
#endif
#endif
#endif

// and one for the STGLIB_STPLATFORM
#endif
