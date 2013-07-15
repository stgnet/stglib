// stg template library
// cgi handling utilities 

#ifndef STGLIB_cgi
#define STGLIB_cgi

#include "/src/stglib/stcore.h"
#include "/src/stglib/stpair.h"
#include "/src/stglib/stparse.h"
#include "/src/stglib/sturl.h"

// helpful notification on win32 compilers
#pragma message("using stcgi.h")

// BASE 64 Decode (for HTTP_AUTHORIZATION)
static const unsigned char StDecodeB64_Map[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
  7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
 19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
 37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };


class StDecodeB64:public StBase
{
	StBuffer _Input;
public:
	// store the tag and opts when constructed
	StDecodeB64()
	{
	}
	virtual ~StDecodeB64()
	{
	}

	// tell StBase::_Transport that this is a pipe
	// so that the data flows from one class to next
	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	// accept content to put between <tag> and </tag>
	virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Input._Write(data,size));
	}

	// write output stream with tags following XML rules
	virtual StSize _WriteTo(StBase &Destination)
	{
		StBuffer Output;

		unsigned long t,x,y,z;
		unsigned char c;
		int g;
		unsigned long len=~_Input;

		g=3;
		for (x = y = z = t = 0; x < len; x++) 
		{
			c = StDecodeB64_Map[_Input(x)&0xFF];

			if (c == 255)
				continue;
			if (c == 254)
			{
				c = 0;
				g--;
			}
			t = (t<<6)|c;
			if (++y == 4)
			{
				Output(z++) = (unsigned char)((t>>16)&255);
				if (g > 1)
					Output(z++) = (unsigned char)((t>>8)&255);
				if (g > 2)
					Output(z++) = (unsigned char)(t&255);
				y = t = 0;
			}
		}
		if (y!=0)
			return(0); // invalid

		Destination<<Output;
		!_Input;
		return(1);
	}
};


// StCgi - instantiate once, probably best globally, gives array of values

class StCgi:public StPair
{
public:
	StString Request;
	StString AuthUser;
	StString AuthPass;
	StString Raw;
	StFile log;
	StCgi()
	{
		log("/tmp/cgi.log",StFileOpenAppend);
		log._FlushAfterWrite=1;

		StdOutput._FlushAfterWrite=1;
		char *request=getenv("REQUEST_URI");
		if (request)
		{
			if (*request=='/' && request[1])
			{
				(request+1)>>Request;
			}
		}
		StString Input;
		char *method=getenv("REQUEST_METHOD");
		if (!method)
		{
			// this is not being run as a CGI!?
			return;
		}
		if (!strcmp(method,"POST"))
		{
			StdInput>>Input;
			Input>>Raw;
		}
		else if (!strcmp(method,"GET"))
		{
			getenv("QUERY_STRING")>>Input;
		}

		// parse input string for pairs separated by ampersand
		StUrlUnEscape unescape; //filter
		while (~Input)
		{
			StParseString next;
			StParseBefore<'&'> value(next);
			StParseBefore<'='> name(value);
			Input>>next;
			if (~name || ~value)
			{
				StString unName;
				StString unValue;
				name>>unescape>>unName;
				value>>unescape>>unValue;
				(*this)(unName)=unValue;
			}
			!Input<<next;
		}

		char *auth=getenv("HTTP_AUTHORIZATION");
		if (auth)
		{
			StDecodeB64 b64;
			StParseString encoded;
			StParseBefore<' '> type(encoded);
			auth>>encoded;
			if (type=="Basic")
			{
				StParseString user;
				StParseAfter<':'> pass(user);
				encoded>>b64>>user;
				user>>AuthUser;
				pass>>AuthPass;
			}
		}
	}

	virtual ~StCgi()
	{
	}
	void RequireAuth(const char *message)
	{
		StdOutput<<"Status: 401 Authorization Required\n";
		StdOutput<<"WWW-Authenticate: Basic realm=\""<<message<<"\"\n";
	}

};

class StCgiQuery:public StString
{
	StPair *pairs;
public:
	StCgiQuery(StPair &values)
	{
		pairs=&values;
	}
	~StCgiQuery()
	{
	}
	
	virtual void _WriteStart(StBase &Dest)
	{
		StUrlEscape escape;
		// populate string JIT
		!(*this);
		StBoxRef<StPairEntry> scan(*pairs);
		while (++scan)
		{
			if (~(*this))
				"&">>(*this);
			scan->Name>>escape>>(*this);
			"=">>(*this);
			scan->Value>>escape>>(*this);
		}
	}
};

#endif

