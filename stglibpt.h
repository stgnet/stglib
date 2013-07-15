// STGLIB/stglibpt.h
// Copyright 1996 by StG Net

#ifndef STGLIB_STGLIBPT
#define STGLIB_STGLIBPT
#pragma message("using stglibpt.h")

// This file defines common types
// and anything else needed by stplatform.h
// prior to stcore.h

// unsigned types
typedef unsigned char StByte;
typedef unsigned short St2Byte;
typedef unsigned long St4Byte;

// alternate format
typedef unsigned char StByt1;
typedef unsigned short StByt2;
typedef unsigned long StByt4;

// signed types
typedef char StChar;
typedef short St2Int;
typedef long St4Int;

// alternate format
typedef char StInt1;
typedef short StInt2;
typedef long StInt4;

// more common unix/palm style types
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;


StByt2 _endian_check=0x0001;

inline int _BigEndian(void)
{
	return( ((StByt1*)&_endian_check) [1]);
}

// network (big endian) variable with auto endian conversion
typedef unsigned char StBig1;

class StBig2
{
	union
	{
		StByt1 _value[2];
		StByt2 _value2;
	};
public:
	inline operator unsigned short(void)
	{
		if (_BigEndian()) return(_value2);
		return(_value[0]<<8|_value[1]);
	}
	inline StBig2& operator=(unsigned short x)
	{
		if (_BigEndian())
		{
			_value2=x;
			return(*this);
		}
		_value[0]=(x>>8)&0xFF;
		_value[1]=x&0xFF;
		return(*this);
	}
};

class StBig4
{
	union
	{
		StByt1 _value[4];
		StByt4 _value4;
	};
public:
	inline operator unsigned long(void)
	{
		if (_BigEndian()) return(_value4);
		return(_value[0]<<24|_value[1]<<16|_value[2]<<8|_value[3]);
	}
	inline StBig4& operator=(unsigned long x)
	{
		if (_BigEndian())
		{
			_value4=x;
			return(*this);
		}
		_value[0]=(x>>24)&0xFF;
		_value[1]=(x>>16)&0xFF;
		_value[2]=(x>>8)&0xFF;
		_value[3]=x&0xFF;
		return(*this);
	}
};


#endif

