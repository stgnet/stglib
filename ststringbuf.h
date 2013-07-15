// STGLIB/ststringbuf.h
// wrap ststring with stbase

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststring.h"
#include "/src/stglib/stbuffer.h"

#ifndef STGLIB_STSTRINGBUF
#define STGLIB_STSTRINGBUF

#pragma message("using ststringbuf.h")

#error OBSOLETE

class StStringBufSection;

class StStringBuf:public StStringFunctions<StBuffer>
{
	friend class StStringBufSection;
//protected:
//	StString _Storage;

public: 
	StStringBuf()
	{
	}

	virtual ~StStringBuf()
	{
	}

	// allow array to duplicate storage to prevent pointer/alloc corruption!
	StStringBuf& operator= (StStringBuf& copyfrom)
	{
//		(*this)=copyfrom._Storage;
		StBuffer::operator=(copyfrom);
		return(*this);
	}

/*	virtual StSize _Read(StBase &Source)
	{
		// read data from source class
		StSize Space=_Storage._ArraySize-_Storage._ArrayUsed;
		StSize Avail=Source._Available();

		if (!Avail)
		{
			// if source doesn't report available size,
			// presume some logical default
			Avail=1024;
		}

		if (Space<Avail)
		{
			// resize array to take in more data
			_Storage._ArrayResize(_Storage._ArrayUsed+Avail);
			Space=_Storage._ArraySize-_Storage._ArrayUsed;
		}

		StSize Got=Source._Read(_Storage._ArrayPtr+_Storage._ArrayUsed,Space);
		if (Source._Error) return(_Err(Source));
		_Storage._ArrayUsed+=Got;
		return(Got);
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Available=_Storage._ArrayUsed;

		if (!Available)
			return(0);

		StSize Wrote=Destination._Write(_Storage._ArrayPtr,Available);
		if (Destination._Error) return(_Err(Destination));
		return(Wrote);
	}

	
	
	
	virtual StSize _Available(void)
	{
		return(~_Storage);
	}

*/

};


// FIX THIS

class StStringBufSection:public StBase
{
protected:
//	StArray<StByte> *_pStorage;
	StStringBuf *_pSB;
	StSize _Offset;
	StSize _Length;

public:
	StStringBufSection()
	{
		_pSB=0;
		_Offset=0;
		_Length=0;
	}

	StStringBufSection(StStringBuf &sb)
	{
//		_pStorage=&(sb._Storage);
		_pSB=&sb;
		_Offset=0;
		_Length=0;
	}

	virtual ~StStringBufSection()
	{
	}

	inline void _SectionMatch(StStringBufSection &sbs)
	{
		_Offset=sbs._Offset;
		_Length=sbs._Length;
	}


	// allow array to duplicate storage to prevent pointer/alloc corruption!
/*	StStringBuf& operator= (StStringBuf& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}
*/


	inline StStringBufSection& operator()(StStringBuf &sb)
	{
//		_pStorage=&(sb._Storage);
		_pSB=&sb;
		return(*this);
	}

	inline StStringBufSection& operator()(StStringBufSection &sbs)
	{
//		_pStorage=sbs._pStorage;
		_pSB=sbs._pSB;
		_Offset=sbs._Offset;
		_Length=sbs._Length; //~sbs;
		return(*this);
	}

	inline StStringBufSection& operator()(StSize offset,StSize length)
	{
		_Offset=offset;
		_Length=length;
		return(*this);
	}
	inline StStringBufSection& operator()(StByte *ptr,StSize length)
	{
//		_Offset=ptr-((StByte*)(*_pStorage));
		_Offset=ptr-(StByte*)(*_pSB);
		_Length=length;
		return(*this);
	}

	inline operator StByte *(void)
	{
		return(((StByte*)(*_pSB))+_Offset);
	}
/*	inline operator void *(void)
	{
		return(((StByte*)*_pStorage)+_Offset);
	}
	inline operator char *(void)
	{
		return((char*)((StByte*)*_pStorage)+_Offset);
	}
*/
  
//	inline StByte* operator&(void)
//	{
//		return(((StByte*)*_pStorage)+_Offset);
//	}


	// return ptr to start of storage as offset reference
//	inline StByte _Reference(void)
//	{
//		return((StByte)*_pStorage);
//	}

	inline virtual void _Reset(void)
	{
		_Offset=0;
		_Length=~(*_pSB);
	}

	inline void operator++(void)
	{
		++_Offset;
	}
	inline void operator--(void)
	{
		--_Offset;
	}

	inline virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_ReadOnly,"StStringBufSection","_Write"));
	}

	inline virtual StSize _Read(StByte *data,StSize size)
	{
		if (size>_Length)
			size=_Length;

		if (size)
		{
			memcpy(data,((StByte*)(*_pSB))+_Offset,size);
		}
		return(size);
	}

	virtual StSize _Read(StBase &Source)
	{
		return(_Err(StErr_ReadOnly,"StStringBufSection","_Read"));
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		if (!_Length)
			return(0);

		StSize Wrote=Destination._Write(((StByte*)(*_pSB))+_Offset,_Length);
		if (Destination._Error) return(_Err(Destination));
		return(Wrote);
	}
	
	
	virtual StSize _Available(void)
	{
		return(_Length);
	}

	bool operator==(char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=(StByte*)(*this);
		StSize size=strlen(s);
		if (size!=~(*this))
			return(false);
		if (memcmp((char*)ptr,s,size))
			return(false);
		return(true);
	}
	bool operator==(const char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=(StByte*)(*this);
		StSize size=strlen(s);
		if (size!=~(*this))
			return(false);
		if (memcmp((char*)ptr,s,size))
			return(false);
		return(true);
	}
	bool operator!=(const char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=(StByte*)(*this);
		StSize size=strlen(s);
		if (size!=~(*this))
			return(true);
		if (memcmp((char*)ptr,s,size))
			return(true);
		return(false);
	}
};

#endif
