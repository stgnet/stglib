// STGLIB/stserver.h
// Copyright 1999,2001 by StG Net

// server base class

#ifndef STGLIB_STSERVER
#define STGLIB_STSERVER

#pragma message("using stserver.h")

#include "/src/stglib/stthread.h"
#include "/src/stglib/stfifo.h"

// This thread/class processes incoming connections
class StServiceThread:public StThread
{
public:
	StFifo<StBaseServ> Services;

	StServiceThread()
	{
		_ThreadStart();
	}
	virtual ~StServiceThread()
	{
	}
	void _Thread(void)
	{
		// process the service from fifo
		while (!_ThreadShutdownRequested)
		{
			_ThreadWait(~Services);
			if (~Services)
			{
				Services->_ServiceProcess();
				delete Services++;
			}
		}
	}
};

// this thread/class contains the service that is accepting connections,
// it also starts new service threads as needed (up to max given)
template <class SERVICE> class StServer:public StThread
{
	StSize _MaxHeadRoom;
public:
	SERVICE _Acceptor;
	StBox<StServiceThread> _Threads;

	StServer(unsigned short port,int max_threads=8)
	{
		_Acceptor._SockListen(0,port);
		_MaxHeadRoom=max_threads;
		_ThreadStart();
	}
	virtual ~StServer()
	{
	}
	virtual void _ServerChooseThread(SERVICE *new_svc)
	{
		StServiceThread *best=0;
		StBoxRef<StServiceThread> scan(_Threads);
		while (++scan)
		{
			if (!~(scan->Services))
			{
				// empty thread - use this
				(scan->Services)+=new_svc;
				return;
			}
			if (!best)
				best=scan;
			else
				if (~(scan->Services) < ~(best->Services))
					best=scan;
		}
		if (~_Threads<_MaxHeadRoom)
		{
			// all threads busy, but not yet maxed out
			StServiceThread *new_thrd=new StServiceThread;
			_Threads+=new_thrd;
			(new_thrd->Services)+=new_svc;
			return;
		}
		// give new service to the least busy thread
		(best->Services)+=new_svc;
	}
	void _Thread(void)
	{
		while (!_ThreadShutdownRequested)
		{
			// this will create a new service when request received
			SERVICE *new_svc=(SERVICE*)_Acceptor._ServiceAccept();
			if (!new_svc)
			{
				StdError<<"ServiceAccept: "<<_Acceptor._GetErrorString()<<"\n";
				continue;
			}

			_ServerChooseThread(new_svc);
		}
	}
};

#endif
