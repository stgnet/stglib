// stg template library
// url handling

#ifndef STGLIB_url
#define STGLIB_url

#include "/src/stglib/stcore.h"
#include "/src/stglib/stfilter.h"
#include "/src/stglib/stparse.h"

// helpful notification on win32 compilers
#pragma message("using sturl.h")

class StUrlSplit:public StString
{
public:
	StString _Protocol;
	StString _Host;
	StString _Path;

	StUrlSplit(const char *url=NULL)
	{
		_AddNotify(this);

		(*this)<<url;
	}
	virtual ~StUrlSplit()
	{
	}

	virtual void _NotifyChanged(StBase *changed)
	{
		// value has changed - reparse
		StParseString url;

		StParseBeforeTwo<'/','/'> prot_colon(url);
		StParseBefore<':'> prot(prot_colon);
		StParseBefore<'/'> host(url);

		url<<(*this);

		if (~prot)
			!_Protocol<<prot;
		else
			!_Protocol<<"http";

		!_Host<<host;
//		if (~url)
//			!_Path<<url;
//		else
//			!_Path<<"/";
		!_Path<<"/"<<url;
	}

};

class StUrlEscape:public StFilter
{
public:
	StUrlEscape()
	{
	}
	virtual ~StUrlEscape()
	{
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		StByte *pend=data+size;
		while (data<pend)
		{
			if ( 
				(*data>='A' && *data<='Z') ||
				(*data>='a' && *data<='z') ||
				(*data>='0' && *data<='9') ||
				*data=='.' ||
				*data=='-' ||
				*data=='_' ||
				*data=='~'
				)
			{
				_Output(data,1);
			}
			else if (*data==' ')
				_Output('+');
			else
			{
				StByte buf[4];
				buf[0]='%';
				buf[1]=StHexDigit(*data>>4);
				buf[2]=StHexDigit((*data)&0x0F);
				_Output(buf,3);
			}
			++data;
		}
		return(size);
	}
};

class StUrlUnEscape:public StFilter
{
public:
	StUrlUnEscape()
	{
	}
	virtual ~StUrlUnEscape()
	{
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		StByte *pend=data+size;
		while (data<pend)
		{
			if (*data=='+')
				_Output(' ');
			else if (*data=='%')
			{
				++data;
				StByte c=StHexByte((char*)data);
				_Output(c);
				++data;
			}
			else
				_Output(data,1);
			++data;
		}
		return(size);
	}
};

#endif
