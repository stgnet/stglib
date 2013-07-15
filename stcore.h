// STGLIB/stcore.h
// Copyright 1996 by StG Net
#ifndef STGLIB_STGLIB
#define STGLIB_STGLIB
#pragma message("using stcore.h")

// load type definitions first
#include "/src/stglib/stglibpt.h"

// load platform-specific definitions
#include "/src/stglib/stplatform.h"

// define stglib specific types
typedef size_t StSize;

// return 'size' value for failed io calls
#define StErr_RETURN ((StSize)(-1))

// time values
typedef time_t StDateTime;



/*
void *operator new(size_t size)
{
	void *ptr=malloc(size);
	printf("===> NEW %x\n",ptr);
	return(ptr);
}
void operator delete(void *ptr)
{
	printf("===> del %x\n",ptr);
	free(ptr);
}
*/

#ifdef STGLIB_DEBUG
#define STGLIB_CON(name) {printf("CONSTR: %s %x\n",name,this);}
#define STGLIB_DES(name) {printf("DESTRY: %s %x\n",name,this);}
#else
#define STGLIB_CON(x)
#define STGLIB_DES(x)
#endif

// as the last thing, include stbase
// this will yank in a lot of the default stuff
// so that user mostly just include stglib and whatever
// upper level chunks of code are wanted

#include "/src/stglib/stbase.h"

#endif

