// S1GLIB/sthttp.h
// Copyright 1999, 2002, 2007 by StG Net

// HTTP (Hyper Text Transfer Protocol) handling


#ifndef STGLIB_STHTTP
#define STGLIB_STHTTP

#pragma message("using sthttp.h")

#include "/src/stglib/stparse.h"
#include "/src/stglib/stfilter.h"
#include "/src/stglib/sttime.h"
#include "/src/stglib/stpair.h"
#include "/src/stglib/sturl.h"
#include "/src/stglib/stsocktcp.h"



/* replaced with StPair
class StHttpParam:public StBuffer
{
public:
	StString _Param;
	StString _Value;

	StHttpParam(StBase &p,StBase &v)
	{
		_Param<<p;
		_Value<<v;
	}
	StHttpParam(const char *p)
	{
		_Param<<p;
	}
};
*/

class StHttpRequestHeader:public StPair
{
public:
	StString _Method;
	StString _Path;
	StString _Version;
	StString _Host;

	StBuffer _Storage;

	StHttpRequestHeader()
	{
	}
	virtual ~StHttpRequestHeader()
	{
	}

	// cause the data to be read from me, not the source
	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	void _HeaderBuild(const char *method,const char *urlstr)
	{
		StUrlSplit url(urlstr);

		!_Method<<method;
		!_Path<<url._Path;
		!_Version<<"HTTP/1.1";
		!_Host<<url._Host;

		(*this)("Host")=_Host;
		if (!_Defined("User-Agent"))
			(*this)("User-Agent")="StGLib";

		!_Storage<<_Method<<" "<<_Path<<" "<<_Version<<StCrLf;


		StBoxRef<StPairEntry> list(*this);
		while (++list)
			_Storage<<list->Name<<": "<<list->Value<<StCrLf;

		_Storage<<StCrLf;
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		return(_Transfer(&_Storage,&Destination));
	}
};

#define StHttpContentLengthUnknown ((StSize)-1)
class StHttpResponseHeader:public StPair
{
public:
	StBuffer _Storage;
//	StBuffer _Header;

	StString _Version;
	StString _Code;
	StString _Message;
	StSize _ContentLength;
	int _Chunked;
	int _ConnectionClose;

	StHttpResponseHeader()
	{
		_ContentLength=StHttpContentLengthUnknown; // unknown
		_Chunked=0;
		_ConnectionClose=0;
	}
	virtual ~StHttpResponseHeader()
	{
	}

	// cause the data to be read from me, not the source
	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	virtual StSize _ReadFrom(StBase &Source)
	{
		int hdrlen;

		while ((hdrlen=_Storage._Contains("\x0D\x0A\x0D\x0A",4))<0)
		{
			if (!_Storage._Read(Source))
				break;
			if (Source._Error)
				return(_Err(Source));
		}
/*
			// read from sock into header
			StSize got=Source._Read(_Storage._ArrayAddPtr(2048),2048);
//StdOutput<<"sockgot="<<got<<"\n";
			if (!got)
				break;
			if (Source._Error)
				return(_Err(Source));
			_Storage._ArrayAddUsed(got);
*/

		if (hdrlen<0)
			return(_Err(StErr_Unknown,"Http::ReadFrom","Header not found"));

		hdrlen+=4;
//StdOutput<<"hdrlen="<<hdrlen<<"\n";
		// transfer just the header for parsing
//		_Header._Write(_Storage,hdrlen);

		// remove it from storage
//		_Storage._ArrayDelete(0,hdrlen);

		// unread the rest of data back to source
		StSize left=~_Storage-hdrlen;
		if (left)
			Source._UnRead((StByte*)_Storage+hdrlen,left);

		// remove data from header storage
		_Storage._ArrayDelete(hdrlen,left);

		// parse the header
		StFilterTextLine line(_Storage);
		
		StParseString message;
		StParseBefore<' '> version(message);
		StParseBefore<' '> code(message);

		line>>message;
		!_Version<<version;
		!_Code<<code;
		!_Message<<message;

		while (1)
		{
			StParseString left;
			StParseAfterTwo<':',' '> right(left);

			line>>left;
			if (!~left)
				break;

			!((*this)(left))<<right;
		}
		if (_Code!="200")
		{
			StError ec=(StError)(atoi(_Code));
			return(_Err(ec,"StHttpResponseHeader",_Message));
		}
		if (_Defined("Content-Length"))
		{
			_ContentLength=atoi((*this)("Content-Length"));
		}
		if (_Defined("Transfer-Encoding"))
		{
			if ((*this)("Transfer-Encoding")=="chunked")
				_Chunked=1;
		}
		if (_Defined("Connection"))
			_ConnectionClose=1;

		return(1);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		return(_Transfer(&_Storage,&Destination));
	}
};

class StHttpUnChunk:public StBase
{
	StBase *Source;
public:
	StHttpUnChunk(StBase &source)
	{
		Source=&source;
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		StBuffer Chunk;
		StBuffer _Storage;
		StSize Total;

STBASE_DEBUG_MSG("begin unchunk","");
StString temp;

		while (1)
		{
			!_Storage;
//			StSize got=Source->_Read(_Storage._ArrayAddPtr(16),15);
			StSize got=_Storage._Read(*Source);
			if (Source->_Error)
				return(_Err(*Source));

			int hdrlen=_Storage._Contains("\x0D\x0A",2);
			if (hdrlen<1)
				return(_Err(StErr_Unknown,"StHttpUnChunk","Chunk header failure"));

			hdrlen+=2;

			StSize left=~_Storage-hdrlen;
			if (left)
				Source->_UnRead((StByte*)_Storage+hdrlen,left);

			StSize chunksize=StHexSize(_Storage);
!temp<<chunksize;
STBASE_DEBUG_MSG("chunk is size=",temp);
			if (!chunksize)
				break;

			!Chunk;
			while (~Chunk<chunksize)
			{
				StSize need=chunksize-~Chunk;
!temp<<need;
STBASE_DEBUG_MSG("chunk need=",temp);

//				got=Source->_Read(Chunk._ArrayAddPtr(need),need);
				got=Chunk._Read(*Source,need);
!temp<<got;
STBASE_DEBUG_MSG("chunk got=",temp);
				if (Source->_Error)
					return(_Err(*Source));
!temp<<~Chunk;
STBASE_DEBUG_MSG("chunk is now=",temp);


			}
			Total+=Destination._Write(Chunk,~Chunk);
!temp<<Total;
STBASE_DEBUG_MSG("total written=",temp);

				StString Trail;
				got=Trail._Read(*Source,2);
				if (got==1)
					got+=Trail._Read(*Source,1);
				if (Source->_Error)
					return(_Err(*Source));
				if (got!=2)
					return(_Err(StErr_Unknown,"StHttpUnChunk","Chunk tail read failure"));
				if (Trail!=StCrLf)
					return(_Err(StErr_Unknown,"StHttpUnChunk","Chunk tail content invalid"));

		}
		return(Total);
	}


};

class StHttp:public StBase
{
public:
	StHttpRequestHeader _Request;
	StHttpResponseHeader _Response;
	StSockTcp sock;

	StHttp()
	{
	}
	StHttp(const char *url)
	{
		Get(url);
	}
	virtual ~StHttp()
	{
	}

	void Get(const char *url)
	{
		_Request._HeaderBuild("GET",url);

		sock._SockConnect(_Request._Host,80);
		if (sock._Error)
		{
			_Err(sock);
			return;
		}

		// send request
		_Request>>sock;
		if (sock._Error)
		{
			_Err(sock);
			return;
		}

		// parse response to get header
		sock>>_Response;
		if (sock._Error)
		{
			_Err(sock);
			return;
		}
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
StString temp;
		StSize wrote=0;
		if (_Response._Chunked)
		{
			StHttpUnChunk upchuck(sock);
			wrote=upchuck._WriteTo(Destination);
			if (upchuck._Error)
				return(_Err(upchuck));
			return(wrote);
		}

temp<<_Response._ContentLength;
STBASE_DEBUG_MSG("not chunked size=",temp);

		if (!_Response._ContentLength)
		{
			wrote=sock._WriteTo(Destination);
			if (sock._Error)
				return(_Err(sock));
			return(wrote);
		}

		StBuffer buffer;
		StSize length=0;
			while (length<_Response._ContentLength)
			{
				StSize need=_Response._ContentLength-length;
!temp<<need;
STBASE_DEBUG_MSG("data need=",temp);

				!buffer;
//				got=Source->_Read(buffer._ArrayAddPtr(need),need);
				StSize got=buffer._Read(sock,need);
!temp<<got;
STBASE_DEBUG_MSG("data got=",temp);
				if (sock._Error)
					return(_Err(sock));
				length+=got;
				wrote+=Destination._Write(buffer,~buffer);
!temp<<length;
STBASE_DEBUG_MSG("length is now=",temp);


			}

		return(wrote);
	}

};



/*

	void ParseHeader(void)
	{
		_BoxEmpty();

		// scan storage for values
		StParseString FirstLine;
		StParseBefore<' '> Method(FirstLine);		// usually GET or POST
		StParseBefore<' '> Path(FirstLine);

		// read the http header one line at a time in text mode
		StFilterTextLine textline(_Storage);

		StString line;

		// parse the first line
		line<<textline;
		FirstLine<<line;

		// store values
		_Method<<Method;
		_Path<<Path;
		_Version<<FirstLine; // version is everything left on first line

		// parse the parameters
		while (!line._Error)
		{
			!line<<textline;
			if (!~line)
				break;

			StParseString value;
			StParseAfter<':'> param(value);
			StParseAfter<' '> nothing(value);

			// parse
			value<<line;

			if (!~value && !~param)
				break;

			// and throw in the box
//			(*this)+=new StPairEntry(param,value);
			!((*this)(param))<<value;

			if (param=="Content-Length")
			{
				StString temp;
				temp<<value;
//				_Content-Length=atoi(temp);
			}
		}
	}

	int _HeaderComplete(void)
	{
		// this attempts a "fast" pointer based scan for the header termination sequence
		const char *start=_Storage;
		const char *end=start+~_Storage;
		const char *scan=start;

		// back "end" up 3 chars to keep from going past allocation
		end-=3;
		if (end<=scan)
			return(0);

		while (scan<end)
		{
			if (scan[0]==0x0D && scan[1]==0x0A && scan[2]==0x0D && scan[3]==0x0A)
			{
				// found end marker
				_HeaderSize=scan-start;
				_HeaderSize+=4;
				return(1);
			}
			++scan;
		}
		return(0);
	}

	virtual StSize _Read(StBase &Source)
	{
		// after being written to, we will pass the data only without header
//		_NoHeaderOutput=1;

		// save source data path for later reading
		_Source=&Source;

		_HeaderSize=0;

		StTime expired;

		// loop while reading header until it's all in
		while (~expired<10)
		{
			StSize got=_Storage._Read(Source);
			if (_Storage._Error)
				return(_Err(_Storage));

			if (_HeaderComplete())
				break;

			// don't have the whole header - kill some time and retry
			if (got)
				!expired;
			else
				Sleep(1);
		}

		// see if there is data past header already read
		// and store it for passing to output
		StSize data_size=~_Storage-_HeaderSize;
		if (data_size)
		{
			_Data=new StBuffer;
			const char *start=_Storage;
			start+=_HeaderSize;

			(*_Data)._Write((StByte*)start,data_size);
		}

		ParseHeader();
		return(_HeaderSize);
	}


class HttpRequest:public HttpHeader
{
public:
	StString _Method;
	StString _Path;
	StString _Version;

	StHttp(const char *host,const char *method,const char *path)
	{
		STBASE_DEBUG_CON("StHttp");

		_Method<<method;
		_Path<<path;
		_Version<<"HTTP/1.1";

		(*this)("Host")=host;
	}
};

class HttpResponse:public HttpHeader
{
public:
	StString _Version;
	StString _Code;
	StString _Status;

};

*/


#endif
