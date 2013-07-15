// STGLIB/stcore.h
// Copyright 1996 by StG Net
#ifndef STGLIB_STP_TURBOC
#define STGLIB_STP_TURBOC
#pragma message("using stp_turboc.h")


typedef int bool;
const int true=1;
const int false=0;


#include <stdio.h>
#include <memory.h>
#include <utime.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//#define _sys_nerr sys_nerr
#define _sys_errlist sys_errlist

#define _isatty isatty
#define _fileno fileno

#endif

