// STGLIB/stpipe.h (was sbuffer)
// Copyright 1996,1998,2000,2002 by StG Net

// simple (basic) buffer (array of unsigned char)

// NOTE: This class *DOES NOT* implement zero termination
//       although otherwise is similar to StString


#ifndef STGLIB_STPIPE
#define STGLIB_STPIPE

#pragma message("using stpipe.h")

#include "/src/stglib/stcore.h"

// pipe class depends on StArray defined
#include "/src/stglib/starray.h"


class StPipe:public StBase
{
private:
	StArray<StByte> _Storage;
	StSize _Gone;

public: 
	StPipe(StSize pipe_size=4096)
	{
		_Gone=0;
		// preset array size
		_Storage._ArrayResize(pipe_size,1);
	}

	virtual ~StPipe()
	{
	}

private:
	void _Compact(void)
	{
		// compact the storage
		if (!_Gone)
			return;

		if (_Storage._ArrayUsed)
			memcpy(_Storage._ArrayPtr,_Storage._ArrayPtr+_Gone,
				_Storage._ArrayUsed-_Gone);

		_Storage._ArrayUsed-=_Gone;
		_Gone=0;
	}

public:

	virtual StSize _Write(const StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StPipe","WRITE",data,size);
		// add data to buffer
		if (!data) 
			return(0);

		if (_Storage._ArrayUsed==_Storage._ArraySize)
		{
			// storage full
			if (!_Gone)
				return(_Err(StErr_Overflow,"StPipe","Write"));

			// shove it down to get more in
			_Compact();
		}
		_Storage._ArrayAdd(data,size);
		return(size);
	}
	StSize _Read(StByte *data,StSize size)
	{
		StSize Available=_Storage._ArrayUsed-_Gone;
		if (size>Available)
			size=Available;

		if (size)
		{
			memcpy(data,_Storage._ArrayPtr+_Gone,size);
			_Gone+=size;
			Available-=size;

			// compact if less than half buffer left
			if (Available<_Storage._ArraySize/2)
				_Compact();
		}
		STBASE_DEBUG_OP("StPipe","READ",data,size);
		return(size);
	}

	virtual StSize _ReadFrom(StBase &Source)
	{
		// read data from source class
		if (_Storage._ArrayUsed==_Storage._ArraySize)
		{
			// storage full
			if (!_Gone)
				return(_Err(StErr_Overflow,"StPipe","Write"));

			// shove it down to get more in
			_Compact();
		}
		StSize Unused=_Storage._ArraySize-_Storage._ArrayUsed;
		StSize Got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,Unused);
		STBASE_DEBUG_OP("StPipe","READ",_Storage._ArrayPtr+_Storage._ArrayUsed,Got);
		if (Source._Error)
			return(_Err(Source));
		_Storage._ArrayUsed+=Got;
		return(Got);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Available=_Storage._ArrayUsed-_Gone;

		if (!Available)
			return(0);

		StSize Wrote=Destination._Write(_Storage._ArrayPtr+_Gone,Available);
		if (Destination._Error)
			return(_Err(Destination));
		_Gone+=Wrote;
		Available-=Wrote;
		if (Available<_Storage._ArraySize/2)
			_Compact();

		return(Wrote);
	}

	void _Delete(StSize size)
	{
		if (size>=_Storage._ArrayUsed-_Gone)
		{
			!_Storage;
			_Gone=0;
			return;
		}

		_Gone+=size;
		if (_Gone>_Storage._ArraySize/2)
			_Compact();
	}

	operator StByte *()
	{
		return(_Storage._ArrayPtr+_Gone);
	}

	virtual StSize _Available(void)
	{
		return(_Storage._ArrayUsed-_Gone);
	}
};

#endif
