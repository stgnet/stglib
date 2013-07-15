// STGLIB/stbuffer.h
// Copyright 1996,1998,2000,2002 by StG Net

// simple (basic) buffer (array of unsigned char)

// NOTE: This class *DOES NOT* implement zero termination
//       although otherwise is similar to StString


#ifndef STGLIB_STBUFFER
#define STGLIB_STBUFFER

#pragma message("using stbuffer.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/starray.h"

class StBuffer:public StBase
{
protected:
	StArray<StByte> _Storage;

public: 
	StBuffer()
	{
		STGLIB_CON("StBuffer");
		STBASE_DEBUG_CON("StBuffer");
	}

	virtual ~StBuffer()
	{
		STGLIB_DES("StBuffer");
		STBASE_DEBUG_DES("StBuffer");
	}

	// allow array to duplicate storage to prevent pointer/alloc corruption!
	StBuffer& operator= (const StBuffer& copyfrom)
	{
		_Storage=copyfrom._Storage;
//		StArray<StByte>::operator=(copyfrom);
		return(*this);
	}
	
	inline StByte& operator()(StSize element)
	{
		return(_Storage[element]);
	}


	inline operator void *(void)
	{
		return(_Storage._ArrayPtr);
	}
	inline operator StByte *(void)
	{
		return(_Storage._ArrayPtr);
	}

	// this is _absolutely required_ for StringFunctions<> to work
	inline virtual StByte *_GetPointer(void)
	{
		if (!_Storage._ArrayPtr)
			return((StByte*)"");
		return(_Storage._ArrayPtr);
	}

	/*	inline StByte* operator&(void)
	{
		return(_Storage._ArrayPtr);
	}
*/
	inline operator const char *(void)
	{
		if (!_Storage._ArrayPtr)
			return("");
		return((const char*)_Storage._ArrayPtr);
	}

	inline void _Changed(void)
	{
		if (_Notify)
			_Notify->_NotifyChanged(this);
	}
	inline virtual void _Reset(void)
	{
		!_Storage;
		_Changed();
	}

	virtual StSize _Write(const StByte *data,StSize size)
	{
		// add data to buffer
		if (!data) 
			return(0);

		_Storage._ArrayAdd(data,size);
		_Changed();
		return(size);
	}

	virtual StSize _Write(const StByte data)
	{
		_Storage._ArrayAdd(data);
		_Changed();
		return(1);
	}

	virtual StSize _Insert(const StByte *data,StSize size)
	{
		_Storage._ArrayInsert(0,data,size);
		return(size);
	}
	virtual StSize _Remove(StByte *data,StSize size)
	{
		StSize Available=_Storage._ArrayUsed;
		if (size>Available)
			size=Available;

		if (size)
		{
			memcpy(data,_Storage._ArrayPtr,size);
			STBASE_DEBUG_OP("StBuffer","READ",data,size);

			_Storage._ArrayDelete(0,size);
		}
		return(size);
	}

	inline void operator+=(StByte byte)
	{
		_Storage._ArrayAdd(byte);
	}


	StSize _Read(StByte *data,StSize size)
	{
		StSize Available=_Storage._ArrayUsed;
		if (size>Available)
			size=Available;

		if (size)
		{
			memcpy(data,_Storage._ArrayPtr,size);
			STBASE_DEBUG_OP("StBuffer","READ",data,size);
		_Changed();
		}
		return(size);
	}

	// defined only for buffer, not base
	// used to do partial read
	StSize _Read(StBase &Source,StSize size=0)
	{
		if (!size)
		{
			size=Source._Available();
			if (!size)
				size=2048;
			_Storage._ArrayResize(_Storage._ArrayUsed+size);
			// update size to max buffer
			size=_Storage._ArraySize-_Storage._ArrayUsed;
		}
		else
			_Storage._ArrayResize(_Storage._ArrayUsed+size);

		StSize got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,size);
		if (Source._Error)
			return(_Err(Source));

		_Storage._ArrayUsed+=got;

		return(got);
	}

	virtual StSize _ReadFrom(StBase &Source)
	{
		// StG 2004-08-20 modified to keep reading when available size unknown
		// StG 2004-09-09 modified to check data format flag for how to read

		// read data from source class
		StSize Avail=Source._Available();
//		StByte KeepReading=0;
		StSize TotalRead=0;

		if (!Avail)
		{
			// if source doesn't report available size,
			// presume some logical default
			Avail=2048;
//			++KeepReading;
		}

		// if destination data format is packet, clear contents before getting new data
//		if (_DataFormat==StBaseDF_Packet)
		if (_IsPacket())
			!(*this);

		while (1)
		{
			StSize Space=_Storage._ArraySize-_Storage._ArrayUsed;
			if (Space<=Avail)
			{
				// resize array to take in more data
				_Storage._ArrayResize(_Storage._ArrayUsed+Avail+1);
				Space=_Storage._ArraySize-_Storage._ArrayUsed;
			}

			StSize Got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,Space);
			
			if (Source._Error) return(_Err(Source));
			_Storage._ArrayUsed+=Got;
			TotalRead+=Got;

//			if (!KeepReading || !Got)
			if (!Got)
				break;
		_Changed();
			if (!Source._IsStream()) //_DataFormat!=StBaseDF_Stream)
				break;
		}
		return(TotalRead);
	}

	// utility function to allow reading only so many bytes (handy for structures)
	virtual StSize _ReadBytes(StBase &Source,StSize wanted)
	{
		// insure enough space
		StSize Space=_Storage._ArraySize-_Storage._ArrayUsed;
		if (Space<wanted)
			_Storage._ArrayResize(_Storage._ArrayUsed+wanted);

		StSize Got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,wanted);
		if (Source._Error) return(_Err(Source));
		if (Got!=wanted) return(_Err(StErr_InvalidDataLength,"StBuffer::_ReadBytes","too few bytes"));
		_Storage._ArrayUsed+=Got;
		if (Got)
		_Changed();

		return(wanted);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Available=_Storage._ArrayUsed;

		if (!Available)
			return(0);

		STBASE_DEBUG_OP("StBuffer","WRITE",_Storage._ArrayPtr,Available);
		StSize Wrote=Destination._Write(_Storage._ArrayPtr,Available);
		if (Destination._Error) return(_Err(Destination));
		// %%% this should check what was wrote and write more if not all was taken
		return(Wrote);
	}

	virtual StSize _Available(void)
	{
		return(_Storage._ArrayUsed);
	}
/*
	virtual StByte* _OpenDMA(void)
	{
		return(_Storage._ArrayPtr);
	}
*/
	StSize _ArraySize(void)
	{
		return(_Storage._ArraySize);
	}

	// map array use call to storage
	void _ArraySetUsedSize(StSize size)
	{
		_Storage._ArraySetUsed(size);
		_Changed();
	}
	void _ArrayAllocate(StSize need)
	{
		_Storage._ArrayResize(need);
	}

	void _ArrayDelete(StSize element,StSize count)
	{
		_Storage._ArrayDelete(element,count);
		_Changed();
	}

	StByte *_ArrayAddPtr(StSize count)
	{
		_Storage._ArrayResize(_Storage._ArrayUsed+count);
		
		return(_Storage._ArrayPtr+_Storage._ArrayUsed);
	}
	void _ArrayAddUsed(StSize count)
	{
		_Storage._ArrayUsed+=count;
		if (_Storage._ArrayUsed>_Storage._ArraySize)
			_Storage._ArrayUsed=_Storage._ArraySize;
	}

	bool operator==(const char *s)
	{
		StSize size=s?(StSize)strlen(s):0;
		if (size!=_Storage._ArrayUsed)
			return(false);
		if (memcmp(_Storage._ArrayPtr,s,size))
			return(false);
		return(true);
	}

	// normally, this should return StSize,
	// but to accomodate zero origin, non-match
	// must return a negative value
	int _Contains(const char *match,StSize len=0)
	{
		if (!_Storage._ArrayPtr)
			return(-1);

		if (!len)
			len=strlen(match);

		StByte *scan=_Storage._ArrayPtr;
		StByte *end=scan+_Storage._ArrayUsed-len;
		while (scan<=end)
		{
			if (*scan==*match && !memcmp(scan,match,len))
				return(scan-_Storage._ArrayPtr);

			++scan;
		}
		return(-1);
	}

		

};

class StBufSect:public StBase
{
protected:
	StBuffer *_Buffer;
	StSize _Offset;
	StSize _Length;

public: 
	StBufSect(StBuffer &ref)
	{
		STGLIB_CON("StBufSect");
		_Buffer=&ref;
		_Offset=0;
		_Length=0;
	}

	virtual ~StBufSect()
	{
		STGLIB_DES("StBufSect");
	}

	virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_ReadOnly,"StFileMap","_Write"));
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (_Offset+size>_Length)
		{
			size=_Length; //-_Offset;
		}

		if (size)
		{
			memcpy(data,(StByte*)(*_Buffer)+_Offset,size);
//			_Offset+=size;
		}
		return(size);
	}
	virtual StSize _Available(void)
	{
		return(_Length);
	}
	virtual void _Reset(void)
	{
		_Offset=0;
		_Length=0;
	}

	void operator()(int off,int len)
	{
		_Offset=off;
		_Length=len;
	}
};


// implement a fifo buffer by re-writing the read/write functions
class StFifoBuffer:public StBuffer
{
public:
	StFifoBuffer()
	{
		STGLIB_CON("StFifoBuffer");
		STBASE_DEBUG_CON("StFifoBuffer");
	}

	virtual ~StFifoBuffer()
	{
		STGLIB_DES("StFifoBuffer");
		STBASE_DEBUG_DES("StFifoBuffer");
	}

	// read data from this buffer, then delete
	virtual StSize _Read(StByte *data,StSize size)
	{
		StSize Available=~_Storage;
		if (!Available)
			return(0);

		if (size>Available)
			size=Available;

		memcpy(data,&(_Storage[0]),size);

		_Storage._ArrayDelete(0,size);
		return(size);
	}

	char _IsStream(void)
	{
		// make sure that the entire fifo is read - not just the first block
		return(1);
	}

	// write data into this buffer
	inline virtual StSize _Write(const StByte *data,StSize size)
	{
		// add data to buffer
		if (!data) 
			return(0);

		_Storage._ArrayAdd(data,size);
		return(size);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Available=~_Storage;

		if (!Available)
			return(0);

		StSize Wrote=Destination._Write(&(_Storage[0]),Available);
		if (Destination._Error) return(_Err(Destination));

		if (Wrote==Available)
		{
			_Storage._ArrayEmpty();
			return(Wrote);
		}

		if (Wrote>Available)
		{
			return(_Err(StErr_DataSizeFailure,"FifoBuffer:Write","Write to destination returned more bytes than was available"));
		}
/*
		// presume wrote<available, thus need to copy data
		StSize chunk=Available-Wrote;
		memcpy(_Storage._ArrayPtr,_Storage._ArrayPtr+Wrote,chunk);
		_Storage._ArrayUsed=chunk;
*/
		_Storage._ArrayDelete(0,Wrote);
		return(Wrote);
	}

	virtual StSize _Read(StBase &Source)
	{
		// read another base into this memory
		StSize Available=~Source;
		_Storage._ArrayResize(_Storage._ArrayUsed+Available);
		StSize Got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,Available);
		STBASE_DEBUG_OP("StFifoBuffer","READ",_Storage._ArrayPtr+_Storage._ArrayUsed,Got);
		if (Source._Error)
			return(_Err(Source));
		_Storage._ArrayUsed+=Got;
		return(Got);
	}
};

#endif
