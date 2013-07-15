// STGLIB/stcore.h
// Copyright 1996 by StG Net

// Generic Unix includes - NOT REALLY A PLATFORM
// included from stp_posix.h, stp_cygwin.h, ...?

#ifndef STGLIB_STP_GUNIX
#define STGLIB_STP_GUNIX
#pragma message("using stp_gunix.h")


// include certain "c standard" headers
// that are needed - done here to prevent
// multiple includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sched.h>

#include <stdarg.h>
#include <signal.h>

#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>

//#include <io.h>


#define StFileDelete remove
#define StFileRename rename
#define StFileIsTerm(x) isatty(fileno(x))


// for staudio.h (WAVE_FORMAT_*)
#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_MULAW 7

typedef struct {
    UInt16 wFormatTag;
    UInt16 nChannels;
    UInt32 nSamplesPerSec;
    UInt32 nAvgBytesPerSec;
    UInt16 nBlockAlign;
    UInt16 wBitsPerSample;
    UInt16 cbSize;
} WAVEFORMATEX;


// emulate WIN32 sleep call for linux
#ifdef bogus

#define stricmp strcasecmp

void Sleep(unsigned long ms)
{
	if (ms>=1000)
	{
		sleep(ms/1000);
		ms%=1000;
	}
	if (ms)
	{
		usleep(ms*1000);
	}
}
#endif


#endif
