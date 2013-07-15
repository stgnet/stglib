// STGLIB/ststringbuf.h
// wrap ststring with stbase


#ifndef STGLIB_STDUMP
#define STGLIB_STDUMP

#pragma message("using stdump.h")

#include "/src/stglib/stcore.h"

class StDump:public StBase
{
protected:
	StBase *_pBase;

public:
	StDump()
	{
		_pBase=0;
	}

	StDump(StBase &base)
	{
		STBASE_DEBUG_CON("StDump");
		_pBase=&base;
	}

	virtual ~StDump()
	{
		STBASE_DEBUG_DES("StDump");
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		Destination;
		// derived class should declare how to write to another class, if possible
		return(_Err(StErr_NotImplemented,"_Write"));
	}

	inline virtual char hex_digit(char x)
	{
		return((x)>9?(x)+'A'-10:(x)+'0');
	}

	inline virtual StSize _Read(StByte *data,StSize size)
	{
		data;
		size;
		return(_Err(StErr_NotImplemented,"StDump","_Read"));
	}
	inline virtual StSize _Write(const StByte *data,StSize size)
	{
		StSize orig_size=size;
		StSize loop,line;
		unsigned char databyte;
		static unsigned char buf[80];
		unsigned char *pbuf;
		StSize value;

		line=0;
		while (size)
		{
			pbuf=buf;
			loop=4;
			value=line;
			pbuf+=4;
			while (loop--)
			{
				*--pbuf=hex_digit((char)(value&15));
				value>>=4;
			}
			pbuf+=4;
			*pbuf++=':';
			*pbuf++=' ';

			loop=0;
			while (loop<16)
			{
				if (loop==size) break;

				databyte=*(data+loop);
				value=databyte>>4;
				*pbuf++=hex_digit((char)(value&15));
				*pbuf++=hex_digit(databyte&15);
				*pbuf++=' ';
				loop++;
			}
			while (loop<16)
			{
				*pbuf++=' ';
				*pbuf++=' ';
				*pbuf++=' ';
				loop++;
			}
			*pbuf++=' ';
			loop=0;
			while (loop<16)
			{
				if (loop==size) break;

				databyte=*(data+loop);
				databyte&= 127;
				if (databyte<32 || databyte>126) databyte='.';
				*pbuf++=databyte;
				loop++;
			}
			if (size>16) size-=16; else size=0;
			data+=16;

			*pbuf++='\n';
			*pbuf=0;
			line+=16;

			(*_pBase)<<(const char*)buf;
		}

		// must return the size originally passed, otherwise
		// some functions may consider it an error
		return(orig_size);
	}
};

#endif
