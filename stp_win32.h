// STGLIB/stcore.h
// Copyright 1996 by StG Net
#ifndef STGLIB_STP_WIN32
#define STGLIB_STP_WIN32

/*
 Compiler                           _MSC_VER value
   --------                           --------------
   C Compiler version 6.0                  600
   C/C++ compiler version 7.0              700
   Visual C++, Windows, version 1.0        800
   Visual C++, 32-bit, version 1.0         800
   Visual C++, Windows, version 2.0        900
   Visual C++, 32-bit, version 2.x         900
   Visual C++, 32-bit, version 4.0         1000
   Visual C++, 32-bit, version 5.0         1100
   Visual C++, 32-bit, version 6.0         1200
*/

#if (_MSC_VER<1200)

#error Please updgrade to at least MSVC6 or update stp_win32.h

#elif (_MSC_VER==1200)

#pragma message("using stp_win32.h MSVC6")

// fix for lack of socklen_t
typedef int socklen_t;

#elif (_MSC_VER==1500)

#pragma message ("using stp_win32.h MSVC9")


#else

#error Unsupported version of MSVC in stp_win32.h

#endif


// define NO_DEPRECATE for VC8's CLIB
#define _CRT_SECURE_NO_DEPRECATE


// include certain "c standard" headers
// that are needed - done here to prevent
// multiple includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <errno.h>

#include <stdarg.h>
#include <signal.h>

#include <io.h>


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//#ifndef _INC_WINDOWS
#ifndef STRICT
#define STRICT
#endif
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <mmreg.h>
//#endif




// file io
#define StFileDelete remove
#define StFileRename rename
#define StFileIsTerm(x) _isatty(_fileno(x))

#define F_OK 0
#define R_OK 2
#define W_OK 4
#define access _access


#define popen _popen
#define pclose _pclose
#define unlink _unlink

#define sleep(x) Sleep(1000*x)
#define isatty _isatty






#ifdef STGLIB_STTHREAD
////////////////////////////////////////////////////////////////////////// MULTI THREADED

// use multi-threaded libraries!
#ifndef _MT
#error Please change project to use multithreaded library
#endif


//#define _WIN32_WINNT 0x0400

#ifndef _INC_PROCESS
#include <process.h>
#endif




// basic signal handling - single auto resetting trigger
#define STTHREAD_STORAGE HANDLE _Thread_Event
#define STTHREAD_CSTR _Thread_Event=::CreateEvent(NULL,FALSE,FALSE,NULL)
#define STTHREAD_DSTR ::CloseHandle(_Thread_Event)

#define STTHREAD_EVENT_SET ::SetEvent(_Thread_Event)
#define STTHREAD_EVENT_WAIT ::WaitForSingleObject(_Thread_Event,INFINITE)

#define STTHREAD_BEGIN _beginthread(_ThreadZero,0,this)

/////////////////////////////////////////////////////////////////////////// END OF MT
#endif





// this is a hack to try to prevent issues with 64 bit int warnings
// better method is to turn of 64bit compatibility checking in MSVC
#ifdef BOGUS

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
// VC7 or VC8 or later have 64 bit support
//typedef unsigned __int64 StSize;

//typedef size_t StSize;
typedef unsigned long StSize;
#else

// size of all buffers and indexes
typedef unsigned long StSize;

#endif
#endif // BOGUS

#endif
