// STGLIB/stthread.h
// Copyright 1999,2002 by StG Net

// os-independant (win32 or posix) thread implementation

#ifdef STGLIB_STGLIB
#error Please include /stglib/stthread.h before /stglib/stcore.h
#endif

#ifndef STGLIB_STTHREAD
#define STGLIB_STTHREAD

#pragma message("using stthread.h")

#include "/src/stglib/stcore.h"

// this code depends on the target api - if it wasn't included, try doing so
#ifdef WIN32
//############################################# WIN32 VERSION

#elif linux

//############################################# LINUX VERSION
#ifndef _REENTRANT
#error Please add -D_REENTRANT to makefile!
#endif



#define STTHREAD_STACKSIZE 8192

#define STTHREAD_STORAGE char *_ThreadStack; int _ThreadPid 
#define STTHREAD_CSTR _ThreadStack=(char*)malloc(STTHREAD_STACKSIZE); _ThreadPid=getpid()
#define STTHREAD_DSTR free(_ThreadStack)

#define STTHREAD_EVENT_SET kill(_ThreadPid,SIGUSR1)
#define STTHREAD_EVENT_WAIT pause()

#define STTHREAD_BEGIN clone(_ThreadZero,_ThreadStack+STTHREAD_STACKSIZE,CLONE_VM|CLONE_FILES,this)

#else
//############################################# POSIX VERSION

#error POSIX version not implemented

#ifndef _REENTRANT
#error Please add -D_REENTRANT to makefile!
#endif

#ifndef PTHREAD_ONCE_INIT
#include <pthread.h>
#endif

#endif // if WIN32

//********************************************* bring in mutex calls
//#include "/stglib/stmutex.h"


//********************************************* mutex class

#ifdef WIN32
//############################################# WIN32 VERSION



class StMutex
{
	HANDLE hMutex;
	unsigned locked;

public:
	StMutex()
	{
		locked=0;
		hMutex=CreateMutex(NULL,FALSE,NULL);
		if (!hMutex)
			throw; // a fit
	}
	~StMutex()
	{
		if (locked)
			ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	inline void _MutexLock(void)
	{
//printf("MutexWait: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		switch (WaitForSingleObject(hMutex,INFINITE))
		{
		case WAIT_OBJECT_0:
			break;

		case WAIT_FAILED:
		case WAIT_ABANDONED:
		case WAIT_TIMEOUT:
		default:
			throw; // a fit
			break;
		}
//printf("MutexLock: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		++locked;

		// if the same thread ++'s a mutex, locked will go to >1
	}

	inline void _MutexUnlock(void)
	{
//printf("MutexUnLk: %08X by %08X was locked=%d\n",this,GetCurrentThreadId(),locked);
		if (!locked)
			throw;
		--locked;
		if (!ReleaseMutex(hMutex))
			throw; // a fit
	}
	inline void operator++(void)
	{
		_MutexLock();
	}
	inline void operator--(void)
	{
		_MutexUnlock();
	}
	inline operator unsigned()
	{
		return(locked);
	}
};


#elif linux
//############################################# LINUX VERSION

#else

//############################################# POSIX VERSION

#error POSIX UNFINISHED

class StMutex
{
	pthread_mutex_t posix_mutex;

public:
	StMutex()
	{
		pthread_mutex_init(&posix_mutex,NULL);
	}
	~StMutex()
	{
		pthread_mutex_destroy(&posix_mutex);
	}

	inline void _MutexLock(void)
	{
		pthread_mutex_lock(&posix_mutex);
	}

	inline void _MutexUnlock(void)
	{
		pthread_mutex_unlock(&posix_mutex);
	}

	inline void operator++(void)
	{
		_MutexLock();
	}
	inline void operator--(void)
	{
		_MutexUnlock();
	}
};
#endif


//********************************************* signal class
class StSignal
{
	StSize Value; // can be used as count of bytes or items
public:
	StSignal()
	{
		Value=0;
	}
	~StSignal()
	{
	}

	// this function calls StThreads class (defined later)
	void SendSignalToThreads(StSignal *signal);

	inline void operator=(StSize new_value)
	{
		if (new_value && !Value)
		{
			// value has gone from zero to non-zero - signal!
			Value=new_value;
			SendSignalToThreads(this);
		}
		else
			Value=new_value;
	}
	inline operator StSize()
	{
		return(Value);
	}
	inline StSize operator++(int)
	{
		StSize old_value=Value;
		Value++;
		if (Value && !old_value)
		{
			SendSignalToThreads(this);
		}
		return(old_value);
	}
	inline StSize operator--(int)
	{
		StSize old_value=Value;
		Value--;
		return(old_value);
	}
};



//********************************************* StThreads class (master control)

#include "/src/stglib/stbox.h"

// class prototype for object going in box
class StThread;

// box to contain a list of the threads in existance
StBox<StThread> StThreads;

//********************************************* thread class

class StThread
{
	// list of signals that thread is waiting on
	StArray<StSignal*> _ThreadWaiting;

	STTHREAD_STORAGE;

public:
	StSignal _ThreadShutdownRequested;
	StSignal _ThreadRunning;
	StSignal _ThreadCompleted;

	StThread()
	{
		STTHREAD_CSTR;

		// add to list of threads
		StThreads+=this;
	}
	virtual ~StThread()
	{
		// ask thread to stop please
		_ThreadShutdownRequested++;

		// and wait for it to exit
		_ThreadWait(_ThreadCompleted);

		STTHREAD_DSTR;

		// remove from list of threads
		StThreads-=this;
	}
	inline void _ThreadWait(StSignal *signal,...)
	{
		va_list arglist;
		unsigned index=0;

		!_ThreadWaiting;
		va_start(arglist,signal);
		while (signal)
		{
			_ThreadWaiting<<signal;
			signal=va_arg(arglist,StSignal*);
		}
		va_end(arglist);

		// array of signals to wait on is loaded
		if (!~_ThreadWaiting)
			return;

		while (1)
		{
			index=0;
			while (index<~_ThreadWaiting)
			{
				if (*(_ThreadWaiting[index++]))
				{
					!_ThreadWaiting;
					return;
				}
			}
			STTHREAD_EVENT_WAIT;
		}
	}
	inline void _ThreadWait(StSignal &signal)
	{
		!_ThreadWaiting;

		if (_ThreadShutdownRequested &&
			&signal==&_ThreadCompleted)
		{
			// ignore shutdown signal, since
			// it's already gone off and we're
			// waiting for thread to finish
			;
		}
		else
			_ThreadWaiting<<&_ThreadShutdownRequested;

		_ThreadWaiting<<&signal;


		while (1)
		{
			StSize index=0;
			while (index<~_ThreadWaiting)
			{
				if (*(_ThreadWaiting[index++]))
				{
					!_ThreadWaiting;
					return;
				}
			}
			STTHREAD_EVENT_WAIT;
		}
	}


	inline void _ThreadSignal(StSignal *signal)
	{
		if (!~_ThreadWaiting)
			return;

		unsigned index=0;
		while (index<~_ThreadWaiting)
		{
			if (signal==_ThreadWaiting[index])
			{
				// found a signal that was being waited on

				STTHREAD_EVENT_SET;
				break;
			}
			index++;
		}
	}

	// pure virtual declaration for actual thread "handler" function
	virtual void _Thread(void)=0;

	// startup the thread
#ifdef WIN32
	friend void _ThreadZero(void *that);
#else
	friend int _ThreadZero(void *that);
#endif

	// this function must be called to kick start the thread function
	void _ThreadStart(void);
};


//********************************************* functions depending on StThreads

#ifdef WIN32
void _ThreadZero(void *that)
#else
int _ThreadZero(void *that)
#endif
{
	((StThread*)that)->_ThreadRunning++;
	((StThread*)that)->_Thread();
	((StThread*)that)->_ThreadRunning=0;
	((StThread*)that)->_ThreadCompleted++;

#ifndef WIN32
	return(0);
#endif

}

void StThread::_ThreadStart(void)
{
	if (!_ThreadRunning)
		STTHREAD_BEGIN;
}

inline void StSignal::SendSignalToThreads(StSignal *signal)
{
	StBoxRef<StThread> scan(StThreads);
	while (++scan)
		scan->_ThreadSignal(signal);
}


	

//********************************************* Thread Box


template <class TYPE> class StThreadBox:public StBox<TYPE>
{
public:


	TYPE *_ThreadCompleted(void)
	{
		StBoxRef<TYPE> scan(*this);
		while (++scan)
		{
			TYPE *ptr=scan;
			if (ptr->_ThreadCompleted)
				return(ptr);
		}
		return(NULL);


//		StSize i=0;
///		while (i<~(*this))
//		{
//			TYPE *ptr=&((*this)[i]);
//			if (ptr && ptr->_ThreadCompleted)
//					return(ptr);
//			++i;
//		}
//		return(0);
	}
};

#endif
