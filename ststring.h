// STGLIB/ststring.h (was stbuffer.h)
// Copyright 1996,1998,2000,2002 by StG Net

// simple (basic) buffer (array of unsigned char)

// NOTE: this class insures zero terminator at end of string 
//       (although the byte is NOT counted by _ArrayUsed)


#ifndef STGLIB_STSTRING
#define STGLIB_STSTRING

#pragma message("using ststring.h")

#include "/src/stglib/stcore.h"
//#include "/src/stglib/starray.h"
#include "/src/stglib/stbuffer.h"


// define our own StUpper() to avoid crtl issues with toupper()
#define StToUpper(x) (((x)>='a' && (x)<='z')?(x)-32:(x))

template <class BASE> class StStringFunctions:public BASE
{
public: 
	StStringFunctions()
	{
		STGLIB_CON("StStringFunctions");
	}

	virtual ~StStringFunctions()
	{
		STGLIB_DES("StStringFunctions");
	}

	StStringFunctions& operator= (StStringFunctions& copyfrom)
	{
		BASE::operator=(copyfrom);
		return(*this);
	}

/*	inline operator const char *()
	{
//		return((const char*)&((*this)[0]));
//		return((const char*)(BASE::(*this)));

//		return((const char*)(&(((BASE*)this)[0])));
		return(BASE::operator const char*());
	}
*/
	bool operator^=(const char *s)
	{
		StByte *ptr=(StByte*)((const char*)(*this));
		StSize size=strlen(s);
		if (size!=~(*this))
			return(false);
//		if (memcmp((char*)ptr,s,size))
		while (size--)
		{
			if (StToUpper(*ptr)!=StToUpper(*s))
				return(false);
			++ptr;
			++s;
		}
		return(true);
	}
	
	bool operator==(const char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=this->_GetPointer();//(StByte*)((const char*)(*this));
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
		StByte *ptr=this->_GetPointer(); //(StByte*)((const char*)(*this));
		StSize size=strlen(s);
		if (size!=~(*this))
			return(true);
		if (memcmp((char*)ptr,s,size))
			return(true);
		return(false);
	}
	bool operator<(const char *s)
	{
		StByte *p1=(StByte*)((const char *)(*this));
		StByte *p2=(StByte*)s;
		while (1)
		{
			if (*p1<*p2) return(1);
			if (*p1!=*p2) return(0);
			if (!*p1 || !*p2) return(0);
			++p1;
			++p2;
		}
	}
	bool operator>(const char *s)
	{
		StByte *p1=(StByte*)((const char *)(*this));
		StByte *p2=(StByte*)s;
		while (1)
		{
			if (*p1>*p2) return(1);
			if (*p1!=*p2) return(0);
			if (!*p1 || !*p2) return(0);
			++p1;
			++p2;
		}
	}

/*	bool _Contains(char c)
	{
		StByte *ptr=(StByte*)(*this);
		StSize len=~(*this);
		while (len--)
			if (*ptr++==c)
				return(true);
		return(false);
	}
*/

/*
	StStringFunctions& operator>>(unsigned long &value)
	{
		const char *s=this->_GetPointer(); //*this;
		if (s)
			value=atoi(s);
		return(*this);
	}
*/

/*	StStringFunctions& operator<< (const unsigned long value)
	{
		if (value>9)
		{
			(*this)<<(value/10);
		}
		{
			StByte ch='0'+(StByte)(value%10);
			_Write(&ch,1);
		}
		return(*this);
	}
	StStringFunctions& operator<< (const char *cstring)
	{
		const char *pend=cstring;
		while (*pend)
			pend++;
		_Write((StByte*)cstring,pend-cstring);
		return(*this);
	}
*/

/* STGLIB 2004: Moved to StBase

	// provide ability to write standard c strings

	StStringFunctions& operator<< (const unsigned int value)
	{
		if (value>9)
		{
			(*this)<<(value/10);
		}
		_Write((StByte)('0'+value%10));
		return(*this);
	}
	StStringFunctions& operator<< (int value)
	{
		if (value<0)
		{
			_Write((StByte*)"-",1);
			value=-value;
		}

		if (value>9)
		{
			(*this)<<(value/10);
		}
//		_Write('0'+value%10);
		{
			StByte ch='0'+value%10;
			_Write(&ch,1);
		}
		return(*this);
	}
	StStringFunctions& operator<< (long value)
	{
		if (value<0)
		{
			_Write((StByte*)"-",1);
			value=-value;
		}

		if (value>9)
		{
			(*this)<<(value/10);
		}
		StByte ch='0'+value%10;
		_Write(&ch,1);
		return(*this);
	}
*/
/*	StStringFunctions& operator<< (time_t tv)
	{
		struct tm *tm=localtime(&tv);

		if (!tm)
		{
			(*this)<<"(BAD DATE)";
			return(*this);
		}
		(*this)<<tm->tm_year+1900;
		(*this)<<"/";
		(*this)<<tm->tm_mon+1;
		(*this)<<"/";
		(*this)<<tm->tm_mday;
		(*this)<<" ";
		(*this)<<tm->tm_hour;
		(*this)<<":";
		(*this)<<tm->tm_min;
		(*this)<<":";
		(*this)<<tm->tm_sec;

		return(*this);
	}
*/
	virtual void _Reset(void)
	{
		BASE::_Reset();
	}

	inline StStringFunctions& operator! ()
	{
		//_ArrayEmpty();
		_Reset();
		return(*this);
	}

/*
	virtual StSize _Write(StByte *data,StSize size)
	{
		// add data to buffer
		if (!data) 
			return(0);

		if (!size)
		{
			// no length specified, presume zero terminated string
			StByte *scan=data;
			while (*scan++) 
				size++;
		}
		_ArrayAddZT(data,size);
//		(*this)[~(*this)]=0;
//		_ArraySetUsed(size);
		return(size);
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (size>~(*this))
			size=~(*this);

		if (size)
			memcpy(data,_ArrayPtr,size);
		return(size);
	}
*/
	


/* can't use this - it conflicts with char * operator[](StSize index)
	inline operator char *(void)
	{
		return(Ptr);
	}
*/


/*
	// append buffer with integer value converted to ascii
	StBuffer& operator<< (StSize value)
	{
		STGLIB_MTLOCK("StBuffer::operator<<(unsigned)");


		int digits=1;


		switch (NumberType)
		{
		case 1: // HEX
			// max value: FFFFFFFF
			if (value>0xFFFFFF) digits=8;
			else
			if (value>0xFFFF) digits=6;
			else
			if (value>0xFF) digits=4;
			else
			digits=2;

			break;

		default:
		case 0: // DEC
			// max value: 4294967296
			if (value>999999999) digits=10;
			else
			if (value>99999999) digits=9;
			else
			if (value>9999999) digits=8;
			else
			if (value>999999) digits=7;
			else
			if (value>99999) digits=6;
			else
			if (value>9999) digits=5;
			else
			if (value>999) digits=4;
			else
			if (value>99) digits=3;
			else
			if (value>9) digits=2;
			break;
		}

		_BufferResize(_ArrayUsed+digits,1);
		_ArrayUsed+=digits;
		StByte *ptr=&_ArrayPtr[_ArrayUsed];
		*ptr=0;

		int temp;

		switch (NumberType)
		{
		case 1: // HEX
			
			while (digits--)
			{
				temp=value&0xF;
				*--ptr=(StByte)(temp>9?temp+'A'-10:temp+'0');
				value>>=4;
			}
			break;

		default:

			while (digits--)
			{
				*--ptr=(StByte)('0'+value%10);
				value/=10;
			}
			break;
		}
		STGLIB_MTUNLOCK("StBuffer::operator<<(unsigned)");
		_BufferParse();
		return(*this);
	}
*/

	
/* old stuff from original Buffer class:

	void SetSize(int);							// set allocation size (before accessing ptr)
	void SetLength(int len);
	void operator=(int len);					// set length to this (0=empty buffer)
//	void operator=(Buffer & copy);					// make copy of buffer

	friend void operator%=(Buffer&, Buffer&);	// swap contents of buffers (fast)

//	Buffer & operator[](int index);	// get section of buffer

	void operator=(char *copy);					// set buffer to string
	void operator+=(char *add);					// add string to buffer

	
	void operator+=(char add);					// add character to buffer
	void Add(unsigned char add);				// add byte to buffer
	void Add(unsigned char *,int);				// add bytes to buffer
	void operator+=(Buffer *add);				// add buffer to buffer
	void printf(char *format,...);				// print into buffer
	int  Recv(int socket);						// read from stream
	int  Send(int socket);						// write to stream
	int  RecvFrom(int Socket,struct sockaddr *Addr,int *AddrLen);
	int  SendTo(int Socket,struct sockaddr *Addr,int AddrLen);
	void ParseLines(void);
	int  GetLine(int number,char *buf,int length);
	void dump(char *b,int l);

*/


};
/*
class StStringNoBase:public StArray<StByte>
{
public:
	StStringNoBase()
	{
	}
	virtual ~StStringNoBase()
	{
	}
	inline operator const char *()
	{
		return((const char*)&((StArray<StByte>::operator [](0))));
	}
	StStringNoBase& operator= (StStringNoBase& copyfrom)
	{
		StArray<StByte>::operator=(copyfrom);
		return(*this);
	}
};
*/







// STGLIB 2004: all classes, even string, are now using base
// was: class StString:public StStringFunctions<StStringNoBase>

class StString:public StStringFunctions<StBuffer>
{
public:
	StString()
	{
		STGLIB_CON("StString");
		STBASE_DEBUG_CON("StString");
	}
	StString(const char *initial_string)
	{
		STGLIB_CON("StString");
		STBASE_DEBUG_CON("StString");
		(*this)<<initial_string;
	}
	StString(const StString&  copyfrom)
	{
		STGLIB_CON("StString (COPY)");
		StBuffer::operator=(copyfrom);
	}
	/*
	StString(StString& copyfrom)
	{
		StBuffer::operator=(copyfrom);
	}
	*/
	virtual ~StString()
	{
		STGLIB_DES("StString");
		STBASE_DEBUG_DES("StString");
	}

	StString& operator= (const StString& copyfrom)
	{
		StBuffer::operator=(copyfrom);
		return(*this);
	}

	StString& operator= (const char *s)
	{
		!(*this)<<s;
		return(*this);
	}

/*	operator const char *()
	{
		return((const char *)(*this));
	}
*/

};


// global utilities for converting to strings
inline char StHexDigit(const char x)
{
	return((x)>9?(x)+'A'-10:(x)+'0');
}
inline StByte StHexByte(const char *s)
{
	StByte v=*s&0xF;
	if (*s>'@') v+=9;
	++s;
	v<<=4;
	v+=*s&0xF;
	if (*s>'@') v+=9;
	return(v);
}
inline StSize StHexSize(const char *s)
{
	StSize v=0;

	while (*s>='0' && *s<='f')
	{
		if (*s>'9' && *s<'A')
			break;
		if (*s>'F' && *s<'a')
			break;
		v<<=4;
		v+=(*s)&0xF;
		if (*s>'@') v+=9;

		++s;
	}
	return(v);
}

StString StHex(unsigned long value,unsigned char digits=2)
{
	static char buf[9];

	char *p=buf+digits;
	*p=0;
	while (digits--)
	{
		--p;
		unsigned char b=(unsigned char)(value&0x0F);
		*p=StHexDigit(b);
		value>>=4;
	}
	return(p);
}

char *StDecRecur(char *p,unsigned long value,int digits)
{
	if (digits>1)
		p=StDecRecur(p,value/10,digits-1);

	*p='0'+(StByte)(value%10);
	++p;
	*p=0;
	return(p);
}

StString StDec(unsigned long value,int digits)
{
	static char buf[12];

	StDecRecur(buf,value,digits);
	return(buf);

}

/*
StString StReadable(StBase &in)
{
	StString out;
	StBuffer buf;
	buf<<in;
	StSize index=0;
	while (index<~buf)
	{
		StByte b=buf[index];
		if (b<0x20 || b>=0x7F)
			out<<"["<<StHex(b)<<"]";
		else
			out._Write(&b,1);
		++index;
	}
	return(out);
}
*/

StString StReadable(StByte *buf,StSize len)
{
	StString out;
	StSize index=0;
	while (index<len)
	{
		StByte b=buf[index];
		if (b<0x20 || b>=0x7F)
			out<<"["<<StHex(b)<<"]";
		else
			out._Write(&b,1);
		++index;
	}
	return(out);
}


static const char *StCrLf="\x0D\x0A";

// FIX THIS

class StStringSection:public StBase
{
protected:
//	StArray<StByte> *_pStorage;
	StString *_pSB;
	StSize _Offset;
	StSize _Length;

public:
	StStringSection()
	{
		STGLIB_CON("StStringSection");
		_pSB=0;
		_Offset=0;
		_Length=0;
	}

	StStringSection(StString &sb)
	{
		STGLIB_CON("StStringSection");
//		_pStorage=&(sb._Storage);
		_pSB=&sb;
		_Offset=0;
		_Length=0;
	}

	virtual ~StStringSection()
	{
		STGLIB_DES("StStringSection");
	}

	StString GetString()
	{
		StString temp;
		temp<<(*this);
		return(temp);
	}


/*	operator const char *()
	{
		StString temp;
		temp<<(*this);
		return(temp);
	}
*/

/*	operator const char *()
	{
		char *temp=new char[~(*this)+1];
		return(temp);
	}
*/

	inline void _SectionMatch(StStringSection &sbs)
	{
		_Offset=sbs._Offset;
		_Length=sbs._Length;
	}


	// allow array to duplicate storage to prevent pointer/alloc corruption!
/*	StString& operator= (StString& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}
*/


	inline StStringSection& operator()(StString &sb)
	{
//		_pStorage=&(sb._Storage);
		_pSB=&sb;
		return(*this);
	}

	inline StStringSection& operator()(StStringSection &sbs)
	{
//		_pStorage=sbs._pStorage;
		_pSB=sbs._pSB;
		_Offset=sbs._Offset;
		_Length=sbs._Length; //~sbs;
		return(*this);
	}

	inline StStringSection& operator()(StSize offset,StSize length)
	{
		_Offset=offset;
		_Length=length;
		return(*this);
	}
	inline StStringSection& operator()(StByte *ptr,StSize length)
	{
//		_Offset=ptr-((StByte*)(*_pStorage));
		_Offset=(StSize)(ptr-(StByte*)(*_pSB));
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
		return(_Err(StErr_ReadOnly,"StStringSection","_Write"));
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
		return(_Err(StErr_ReadOnly,"StStringSection","_Read"));
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
		StSize size=(StSize)strlen(s);
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
		StSize size=(StSize)strlen(s);
		if (size!=~(*this))
			return(false);
		if (memcmp((char*)ptr,s,size))
			return(false);
		return(true);
	}
	bool operator!=(char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=(StByte*)(*this);
		StSize size=(StSize)strlen(s);
		if (size!=~(*this))
			return(true);
		if (memcmp((char*)ptr,s,size))
			return(true);
		return(false);
	}
	bool operator!=(const char *s)
	{
		// this should be re-written to remove the
		// clib calls to improve speed
		StByte *ptr=(StByte*)(*this);
		StSize size=(StSize)strlen(s);
		if (size!=~(*this))
			return(true);
		if (memcmp((char*)ptr,s,size))
			return(true);
		return(false);
	}
};


#endif
