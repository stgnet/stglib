// STGLIB/stsock.h
// Copyright 1999,2002 by StG Net

// base sockets class 
//
// Provides base functionality of sockets interface
// Adapts to Winsock or BSD Sockets
//
// Constructor must be called with TCP or UDP protocol
// specified to set correct socket type
//
// Applications should use StTcp or StUdp rather than 
// StSock directly


#ifndef STGLIB_STSOCKADDR
#define STGLIB_STSOCKADDR

#pragma message("using stsockaddr.h")

#include "/src/stglib/stbox.h"
#include "/src/stglib/stsock.h"

// THIS is GLOBAL on purpose - to avoid having one copy for each socket
char _StSock_HostName_[64];

const char *_GetHostName(void)
{
	gethostname(_StSock_HostName_,sizeof(_StSock_HostName_));
	return(_StSock_HostName_);
}

// cache of reverse name lookups
class StIpName:public StString
{
public:
	unsigned long saddr;
};



#ifdef BOGUS_STGLIB_STTHREAD
// if we have multi-thread enabled, use a separate thread to look up IP addresses
class StIpNameCacheLookup:public StThread
{
public:
	StSignal NewRequest;

	StIpNameCacheLookup()
	{
		_ThreadStart();
	}

	void _Thread()
	{
		while (!_ThreadShutdownRequested)
		{
			StBoxRef<StIpName> scan(_StIpNameCache);
			while (++scan)
			{
				if (!~(*scan))
				{
					// lookup
					struct hostent *entry=gethostbyaddr((char*)&(scan->saddr),4,AF_INET);
					if (entry)
						(*scan)<<entry->h_name;
					else
						(*scan)<<"???";
				}
			}
//			Sleep(1000);
			_ThreadWait(NewRequest);
		}
	}
} _StIpNameCacheLookup;
#endif


// later need to make this drop off older unused entries (create a StCache template)
class StIpNameCache:public StBox<StIpName>
{
public:
	const char *Lookup(unsigned long addr)
	{
		if (addr==0)
			return(0);

		// don't bother trying to look up multicast or broadcast addresses
		StByte network=(StByte)((ntohl(addr))>>24);
		if (network>=224)
			return(0);

		StBoxRef<StIpName> scan(*this);
		while (++scan)
		{
			if (scan->saddr==addr)
				return((const char*)(*scan));
		}
		StIpName *new_entry=new StIpName;
		new_entry->saddr=addr;
#ifdef BOGUS_STGLIB_STTHREAD
		(*this)+=new_entry;
		StIpNameCacheLookup.NewRequest++;
		return("");
#else
//		struct hostent *entry=gethostbyaddr((char*)&addr,4,AF_INET);
struct hostent *entry=NULL;
		if (entry)
			(*new_entry)<<entry->h_name;
		(*this)+=new_entry;
		return((const char*)(*new_entry));
#endif
	}
} _StIpNameCache;




// IP address class
class StIpAddr
{
public:
	struct sockaddr_in _sockaddr;
	char padding[4];
	struct sockaddr_in _netmask;
	StString _readable;

	StIpAddr()
	{
		memset(&_sockaddr,0,sizeof(_sockaddr));
		_sockaddr.sin_family=AF_INET;
		_sockaddr.sin_addr.s_addr=INADDR_ANY;
		_sockaddr.sin_port=0;
		_netmask.sin_addr.s_addr=0;
	}
	// this form used to view an address already in network format
/*	StIpAddr(unsigned long addr,unsigned short port=0)
	{
		memset(&_sockaddr,0,sizeof(_sockaddr));
		_sockaddr.sin_family=AF_INET;
		_sockaddr.sin_addr.s_addr=addr;
		_sockaddr.sin_port=port;
		netmask=0;
	}
*/
	StIpAddr(StIpAddr &addr,unsigned short port)
	{
		memset(&_sockaddr,0,sizeof(_sockaddr));
		_sockaddr=addr._sockaddr;
		_sockaddr.sin_port=htons(port);
		_netmask=addr._netmask;
	}
	StIpAddr(const char *addr,unsigned short port=0)
	{
		memset(&_sockaddr,0,sizeof(_sockaddr));
		_sockaddr.sin_family=AF_INET;
		_sockaddr.sin_addr.s_addr=INADDR_ANY;
		_sockaddr.sin_port=0;
		(*this)(addr,port);
		_netmask.sin_addr.s_addr=0;
	}

	StSize operator()(StIpAddr &addr)
	{
		_sockaddr=addr._sockaddr;
		_netmask=addr._netmask;
		return(StErr_NONE);
	}

	// allow setting an alternate Family and Protocol (not IP)
	// this is mostly just used for raw sockets
	void _Set(unsigned family, unsigned protocol)
	{
		memset(&_sockaddr,0,sizeof(_sockaddr)+4);
		_sockaddr.sin_family=family;
		_sockaddr.sin_port=htons(protocol);
	}


	void operator()(unsigned long network_addr)
	{
		_sockaddr.sin_addr.s_addr=network_addr;
	}

	unsigned short _GetPort(void)
	{
		return(ntohs(_sockaddr.sin_port));
	}
	void _SetPort(unsigned short port)
	{
		_sockaddr.sin_port=htons(port);
	}
	StSize operator()(StIpAddr &addr,unsigned short port)
	{
		_sockaddr=addr._sockaddr;
		_sockaddr.sin_port=htons(port);
		return(StErr_NONE);
	}
	StSize operator()(sockaddr_in sain)
	{
		_sockaddr=sain;
	}

	// this call accepts NETWORK order!
	StSize _SetNetworkAddr(StByt4 addr,StByt2 port=0)
	{
		_sockaddr.sin_family=AF_INET;
		_sockaddr.sin_addr.s_addr=addr;
		_sockaddr.sin_port=port;
		return(StErr_NONE);
	}

	bool operator==(StByt4 match)
	{
		return(_sockaddr.sin_addr.s_addr==match);
	}
	bool operator!=(StByt4 match)
	{
		return(_sockaddr.sin_addr.s_addr!=match);
	}

	bool operator==(StIpAddr &compare_to)
	{
		if (_sockaddr.sin_addr.s_addr==compare_to._sockaddr.sin_addr.s_addr &&
		    _sockaddr.sin_port       ==compare_to._sockaddr.sin_port)
			return(true);
		return(false);
	}
	bool operator!=(StIpAddr &compare_to)
	{
		if (_sockaddr.sin_addr.s_addr!=compare_to._sockaddr.sin_addr.s_addr &&
		    _sockaddr.sin_port       !=compare_to._sockaddr.sin_port)
			return(true);
		return(false);
	}
	bool operator<(StIpAddr &match)
	{
		unsigned long left=ntohl(_sockaddr.sin_addr.s_addr);
		unsigned long right=ntohl(match._sockaddr.sin_addr.s_addr);
		if (left<right)
			return(true);
		return(false);
	}
	bool operator>(StIpAddr &match)
	{
		unsigned long left=ntohl(_sockaddr.sin_addr.s_addr);
		unsigned long right=ntohl(match._sockaddr.sin_addr.s_addr);
		if (left>right)
			return(true);
		return(false);
	}
	StIpAddr& operator++(void)
	{
		unsigned long temp=ntohl(_sockaddr.sin_addr.s_addr);
		++temp;
		_sockaddr.sin_addr.s_addr=htonl(temp);
		return(*this);
	}

	StIpAddr operator|(const StIpAddr &right)
	{
		StIpAddr temp=*this;
		temp._sockaddr.sin_addr.s_addr|=right._sockaddr.sin_addr.s_addr;
		return(temp);
	}
	StIpAddr operator&(const StIpAddr &right)
	{
		StIpAddr temp=*this;
		temp._sockaddr.sin_addr.s_addr&=right._sockaddr.sin_addr.s_addr;
		return(temp);
	}
	StIpAddr operator~()
	{
		StIpAddr temp=*this;
		temp._sockaddr.sin_addr.s_addr=~temp._sockaddr.sin_addr.s_addr;
		return(temp);		
	}
/*
	StSize operator()(SOCKADDR_IN*addr)
	{
		_sockaddr=*addr;
		return(StErr_NONE);
	}
*/
	void operator()(const char *addr,StByt2 port=0)
	{
		if (port)
			_sockaddr.sin_port=htons(port);

		if (!addr)
			return; //(StErr_NONE);

		// copy address string to local storage and parse
		StString host;
		host<<addr;

		// does the address have a ':' in it?
		char *p=(char*)(const char*)host;
		while (*p)
		{
			if (*p==':')
				break;
			p++;
		}
		if (*p)
		{
			// yes, it does - break it apart to get port
			*p++=0;
			_sockaddr.sin_port=htons(atoi(p));
		}

		_sockaddr.sin_addr.s_addr=inet_addr(host);
		if (_sockaddr.sin_addr.s_addr && _sockaddr.sin_addr.s_addr!=-1)
			return; //(StErr_NONE);

		struct hostent *entry=gethostbyname(host);
		if (!entry)
			return;
/*		{
			_Err((StError)WSAGetLastError());
			StString &msg=*(StString*)_ErrorString;
			!msg<<"StIpAddr: "<<"gethostbyname"<<": #"<<_Error<<" "<<_SockErrorDescription(_Error,_ErrorDescription());
			return(StErr_RETURN);
		}
*/
		_sockaddr.sin_addr.s_addr=*(unsigned long *)entry->h_addr_list[0];
		return; //(StErr_NONE);
	}
/*	char *_HostName(void)
	{
		// to save time, if the netmask is known, check for broadcast address before trying to reverse
		if (!~(_sockaddr.sin_addr.s_addr|netmask))
			return("BROADCAST");

		struct hostent *entry=gethostbyaddr((char*)&_sockaddr.sin_addr,sizeof(_sockaddr.sin_addr),AF_INET);
		if (!entry)
			return(inet_ntoa(_sockaddr.sin_addr));

		return(entry->h_name);
	}
*/
	StString _IpAddr(void)
	{
		StString ip;

		ip<<inet_ntoa(_sockaddr.sin_addr);
		return(ip);
	}

	// before this class is written to an output device, update the inherited string with the current ip:port
	operator StString& ()
	{
		!_readable<<inet_ntoa(_sockaddr.sin_addr);
		if (_sockaddr.sin_port)
		{
			_readable<<":"<<ntohs(_sockaddr.sin_port);

			servent *s=getservbyport(_sockaddr.sin_port,NULL);	
			if (s)
			{
				_readable<<" ["<<s->s_name<<"]";
			}
		
		}

		// to save time, if the netmask is known, check for broadcast address before trying to reverse
		if (!~(_sockaddr.sin_addr.s_addr|_netmask.sin_addr.s_addr))
		{
			_readable<<" (*)";
		}
		else
		{
			// reverse lookup the name
//			struct hostent *entry=gethostbyaddr((char*)&_sockaddr.sin_addr,sizeof(_sockaddr.sin_addr),AF_INET);
//			if (entry)
//			{
//				_readable<<" ("<<entry->h_name<<")";
//			}
			const char *name=_StIpNameCache.Lookup(_sockaddr.sin_addr.s_addr);
			if (name&& *name)
				_readable<<" ("<<name<<")";
		}
		return(_readable);
	}
	StByt4 _GetNetworkAddr()
	{
		return(_sockaddr.sin_addr.s_addr);
	}

	StIpAddr _GetNetmask(void)
	{
		StIpAddr temp;
		temp._SetNetworkAddr(_netmask.sin_addr.s_addr);
		return(temp);
	}
	void _SetNetmask(StIpAddr &mask)
	{
		_netmask.sin_addr.s_addr=mask._sockaddr.sin_addr.s_addr;
	}


};

class StMacAddr
{
public:
	StByte MAC[6];

//	StMacAddr():StBaseData(MAC,6)
	StMacAddr()
	{
		memset(MAC,0,6);
	}
	void operator!(void)
	{
		memset(MAC,0,6);
	}
//	StMacAddr(const StMacAddr& copy):StBaseData(MAC,6)
	StMacAddr(const StMacAddr& copy)
	{
		memcpy(MAC,copy.MAC,sizeof(MAC));
	}
//	StMacAddr(StByte *buf,StSize len):StBaseData(MAC,6)
	StMacAddr(StByte *buf,StSize len)
	{
		if (len>6)
			len=6;
		if (len<6)
			memset(MAC,0,6);
		memcpy(MAC,buf,len);
	}

	// handy for comparing to 0x00 or 0xFF
	/*
	bool operator==(StByte match)
	{
		if (MAC[0]==match &&
			MAC[1]==match &&
			MAC[2]==match &&
			MAC[3]==match &&
			MAC[4]==match &&
			MAC[5]==match
		)
			return(true);
		return(false);
	}
	*/
	StString ColonFormat(void)
	{
		StString buf;
		buf<<StHex(MAC[0]);
		buf<<":";
		buf<<StHex(MAC[1]);
		buf<<":";
		buf<<StHex(MAC[2]);
		buf<<":";
		buf<<StHex(MAC[3]);
		buf<<":";
		buf<<StHex(MAC[4]);
		buf<<":";
		buf<<StHex(MAC[5]);
		return(buf);
	}
};

#endif
