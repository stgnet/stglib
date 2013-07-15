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

// WIN32: must add "ws2_32.lib" to list of library modules

#ifndef STGLIB_STSOCK
#define STGLIB_STSOCK

#pragma message("using stsock.h")
#ifdef STGLIB_STP_WIN32
#pragma comment(linker, "/defaultlib:ws2_32.lib")
#endif


#include "/src/stglib/stcore.h"
#include "/src/stglib/stbase.h"
#include "/src/stglib/sttime.h"

// get the necessary headers dependant on OS type

#ifdef STGLIB_STP_WIN32
//=========================== WIN32 "Winsock" API


//#ifndef SOCKET_ERROR
#include <winsock2.h>
#include <ws2tcpip.h>
//#endif


#include "/src/stglib/stsockwse.h"

// global flag so wsa init is done once
int _StGLib_StSock_Initialized=0;

typedef int sockaddr_size_type;

// must include stwin32 here
//#include "/stglib/stwin32.h"


#else
//=========================== UNIX "BSD Sockets" API

#include <sys/socket.h>
#include <sys/select.h>
extern "C"
{
#include <netinet/in.h>
#include <arpa/inet.h>
}
#include <net/if.h>

#include <netdb.h>
//#include <ioctl.h>
#include <unistd.h>
//#include <errno.h>
#define SOCKET_ERROR (-1)

// map common "winsock" type calls to unix
// equivalent to make coding much simpler
#define ioctlsocket ioctl
#define closesocket close
#define WSAGetLastError() errno
#define WSAEWOULDBLOCK EWOULDBLOCK
#define WSAEINPROGRESS EINPROGRESS
#define WSAECONNREFUSED ECONNREFUSED
#define WSAECONNRESET ECONNRESET

typedef unsigned sockaddr_size_type;


#endif

// include sockaddr here, since it needs prior defines and the following needs it
#include "/src/stglib/stsockaddr.h"


// for DGRAM/0 to get if configuration
#define IPPROTO_CONF (-1)

// base class for handling sockets (tcp, udp, raw)
class StSock:public StBaseServ
{
	friend class StSockTcp;
protected:
	unsigned int _protocol;
	unsigned int _socket_handle;
	struct timeval _socket_timeout;
	StIpAddr _IpAddr;
	
public:
	StSock(int Protocol)
	{
		_socket_handle=SOCKET_ERROR;
		_protocol=Protocol;

		// all read/write calls unblock every second
		_socket_timeout.tv_sec=1;
		_socket_timeout.tv_usec=0;

#ifdef WIN32
		// Winsock must be initialized before use
		if (!_StGLib_StSock_Initialized)
		{
			WORD wVersionRequested=0x0101;
			WSADATA wsaData;
	
			if (WSAStartup(wVersionRequested,&wsaData))
			{
				_Err(StErr_Unknown,"WinSock failed to initialize","WSAStartup");
				return;
			}
			_StGLib_StSock_Initialized++;
		}
#endif

		_OpenSocket();
	}

	virtual ~StSock()
	{
		_SockShutdown();
	}

	void operator()(const char *addr,StByt2 port=0)
	{
		_IpAddr(addr,port);		
	}
	virtual StSize operator()(StIpAddr &addr)
	{
		return(_IpAddr(addr));
	}
	StIpAddr &GetIpAddress(void)
	{
		return(_IpAddr);
	}

	void _SetTimeoutMSec(StSize msec)
	{
		_socket_timeout.tv_sec=0;
		_socket_timeout.tv_usec=msec*1000;
	}
	void _SetTimeoutSec(StSize sec)
	{
		_socket_timeout.tv_sec=sec;
		_socket_timeout.tv_usec=0;
	}
		
	char _IsStream(void)
	{
		if (_protocol==IPPROTO_TCP)
			return(1);
		return(0);
	}
	char _IsPacket(void)
	{
		if (_protocol==IPPROTO_TCP)
			return(0);
		return(1);
	}

private:
	// allocate socket of the needed type
	StSize _OpenSocket(void)
	{
		if (_socket_handle!=SOCKET_ERROR)
			return(StErr_NONE);

		switch (_protocol)
		{
		case IPPROTO_TCP:
			_socket_handle=socket(AF_INET,SOCK_STREAM,_protocol);
			break;

		case IPPROTO_UDP:
			_socket_handle=socket(AF_INET,SOCK_DGRAM,_protocol);
			break;

		case IPPROTO_IP:
			// for linux, IPPROTO_IP results in error 93 protocol not supported
//			_socket_handle=socket(AF_INET,SOCK_RAW,_protocol);
//			if (_socket_handle==SOCKET_ERROR)

			_socket_handle=socket(PF_PACKET,SOCK_RAW,htons(0x0003)); //htons(ETH_P_ALL));

			break;

		case IPPROTO_CONF:
			_socket_handle=socket(AF_INET,SOCK_DGRAM,0); // IPPROTO_IP
			break;
			
		default:
			return(_Err(StErr_NotImplemented,"StSock: protocol not supported"));
		}

		if (_socket_handle==SOCKET_ERROR)
			return(_SockError("socket"));

		return(StErr_NONE);
	}

	// perform address to ip translation
	// accepts the following:
	//
	// ("1.2.3.4",80)
	// ("1.2.3.4:80")
	// ("foobar.com",80)
	// ("foobar.com:80")
	//
	// port specified in addr string will take precedence
	// over port specified in second arg
	//
	// this function is a private utility for other StSock
	// calls and not intended to be called directly by app

/*
protected:
	StSize _SockSetAddress(const char *addr,int port=0)
	{
		if (port)
			_address.sin_port=htons(port);

		if (!addr)
			return(StErr_NONE);

		const char *host=addr;

		_address.sin_addr.s_addr=inet_addr(host);
		if (_address.sin_addr.s_addr && _address.sin_addr.s_addr!=-1)
			return(StErr_NONE);

		struct hostent *entry=gethostbyname(host);
		if (!entry)
			return(_SockError("gethostbyname"));

		_address.sin_addr.s_addr=*(unsigned long *)entry->h_addr_list[0];
		return(StErr_NONE);
	}
*/
public:

	void _SockShutdown(void)
	{
		if (_socket_handle!=SOCKET_ERROR)
		{
			shutdown(_socket_handle,2);
			closesocket(_socket_handle);

			_socket_handle=SOCKET_ERROR;
		}
	}


/*
	StSize _SockIoctl(unsigned long code,void *in,StSize in_size,void *out,StSize out_size)
	{
		if (_OpenSocket())
			return(StErr_RETURN);
		StSize got=0;
		if (ioctlsocket(_socket_handle,code,in,in_size,out,out_size,&got,NULL,NULL)==SOCKET_ERROR)
			return(_SockError("ioctlsocket"));
		return(got);
	}
	StSize _SockSetOption(unsigned long code,unsigned int value)
	{
		return(_SockIoctl(code,&value,sizeof(value),NULL,0));
	}
	StSize _SockGetData(unsigned long code,StByte *data,StSize size)
	{
//		return(_SockIoctl(code,0,0,data,size));
		if ( return(ioctlsocket(_socket_handle,code,data));
	}
*/
	StSize _SockIoctl(unsigned long code,StByte *data)
	{
		return(ioctlsocket(_socket_handle,code,data));
	}

	StSize _SockBind(void)
	{
		if (_OpenSocket())
			return(StErr_RETURN);

		int salen=sizeof(struct sockaddr);
		if (_IpAddr._sockaddr.sin_family==PF_PACKET)
			salen+=4;

		if (bind(_socket_handle,(struct sockaddr*)&_IpAddr._sockaddr,salen)==SOCKET_ERROR)
			return(_SockError("bind"));
		return(StErr_NONE);
	}
	StSize _SockBind(StIpAddr &addr)
	{
		if (_OpenSocket())
			return(StErr_RETURN);
		_IpAddr(addr);
		if (bind(_socket_handle,(struct sockaddr*)&_IpAddr._sockaddr,sizeof(struct sockaddr))==SOCKET_ERROR)
			return(_SockError("bind"));
		return(StErr_NONE);
	}

	StSize _SockConnect(void)
	{
		if (_OpenSocket())
			return(StErr_RETURN);
		if (connect(_socket_handle,(struct sockaddr*)&_IpAddr._sockaddr,sizeof(struct sockaddr))==SOCKET_ERROR)
			return(_SockError("connect"));
		return(StErr_NONE);
	}
	inline StSize _SockConnect(const char *addr, int port)
	{
		_IpAddr(addr,port);
		if (_IpAddr==0)
			return(StErr_RETURN);
		return(_SockConnect());
	}

/*	inline void _DeviceUnblock(void)
	{
		// DeviceUnblock is called by SServer when a server instance
		// is being deconstructed.  The purpose is to somehow force
		// the accept() call to fail out so the thread completes.

		// this works, although it seems a bit dangerous
		_SockShutdown();

		// note that this call destroys the socket, so it should only
		// be used when the device is no longer needed
	}
*/

	// utility to format error reporting string
	StSize  _SockError(const char *func)
	{
		if (!_Error)
		{
			_Err((StError)WSAGetLastError());
			StString &msg=*(StString*)_ErrorString;
#ifdef WIN32
			!msg<<"StSock: "<<func<<": #"<<_Error<<" "<<_SockErrorDescription(_Error,_ErrorDescription());
#else
			!msg<<"StSock: "<<func<<": #"<<_Error<<" "<<_ErrorDescription();
#endif
		}
		return(StErr_RETURN);
	}



	// support for device io (see SDevice)

/*	virtual StSize _WriteTo(StBase &Destination)
	{

		StBuffer buf(4096);

		StSize written=0;

		while (1)
		{
			StSize got=_Read(buf,~buf);
			if (_Error)
				return(StErr_RETURN);

			if (!got)
				break;

			written+=Destination._Write(buf,got);
			if (Destination._Error)
				return(_Err(Destination));

			if (_protocol!=IPPROTO_TCP) // only one packet for UDP & RAW
				break;
		}
		return(written);
	}
*/
	virtual StSize _SockSelectRead()
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(_socket_handle,&fd);
		struct timeval tv=_socket_timeout;
		switch (select(_socket_handle+1,&fd,0,0,&tv))
		{
		case SOCKET_ERROR:
			return(_SockError("Read-select"));
		case 0:
			return(_Err(StErr_Timeout,"_Select","timeout"));
		}
		return(0);
	}
	virtual StSize _SockSelectWrite()
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(_socket_handle,&fd);
		struct timeval tv=_socket_timeout;
		switch (select(_socket_handle+1,0,&fd,0,&tv))
		{
		case SOCKET_ERROR:
			return(_SockError("Read-select"));
		case 0:
			return(_Err(StErr_Timeout,"_Select","timeout"));
		}
		return(0);
	}
	virtual StSize _Write(const StByte *data,StSize size)
	{
		if (_SockSelectWrite()) return(StErr_RETURN);

		StSize put=send(_socket_handle,(char*)data,size,0);

		if (put==SOCKET_ERROR)
			return(_SockError("send"));

		return(put);
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (_socket_handle==SOCKET_ERROR)
			return(_SockError("Read invalid handle"));

		if (_PreRead)
			return(_ReRead(data,size));

		while (1)
		{
			StTimer timeout(10);
			fd_set fd;
			FD_ZERO(&fd);
			FD_SET(_socket_handle,&fd);
			struct timeval tv={10,0};//_socket_timeout;
			int sel=select(_socket_handle+1,&fd,NULL,NULL,&tv);
//StdOutput<<"Sock sel="<<sel<<" sec="<<tv.tv_sec<<"\n";
			switch (sel)
			{
			case SOCKET_ERROR:
				return(_SockError("Read-select"));
			case 0:
				if (!~timeout)
					continue;
				return(_Err(StErr_Timeout,"_Read","timeout"));
			}

			StSize got=recv(_socket_handle,(char*)data,size,0);
			if (got==SOCKET_ERROR)
				return(_SockError("recv"));
//StdOutput<<"sock recv "<<got<<" errno="<<errno<<"\n";
			if (!got)
				continue;
			return(got);
		}
	}

	/* BROKEN
	StBuffer _SockGetName(void)
	{
		StBuffer buf;
		
		int size=64;
		buf._ArrayAllocate(size);

		StSize got=getsockname(_socket_handle,(sockaddr *)(void*)buf,&size);
		if (got==SOCKET_ERROR)
		{
			_SockError("getsockname");
			!buf;
			return(buf);
		}
		
		buf._ArraySetUsedSize(size);
		return(buf);
	}
	*/
	StIpAddr _SockGetName(void)
	{
		StIpAddr addr;
		socklen_t len=sizeof(struct sockaddr);
		if (getsockname(_socket_handle,(struct sockaddr*)&addr,&len)==SOCKET_ERROR)
			_SockError("getsockname");
		return(addr);
	}

	StIpAddr _SockGetPeer(void)
	{
		StIpAddr addr;
		socklen_t len=sizeof(struct sockaddr);
		if (getpeername(_socket_handle,(struct sockaddr*)&addr,&len)==SOCKET_ERROR)
			_SockError("getpeername");

		return(addr);
	}

	// service functions
	
	// bind socket, address presumed to be already set
	StSize _SockListen(void)
	{
		// put socket into listen mode
		if (_OpenSocket())
			return(StErr_RETURN);
		if (listen(_socket_handle,5)==SOCKET_ERROR)
			return(_SockError("listen"));
		return(StErr_NONE);
	}

	// need to bind socket 
	inline StSize _SockListen(const char *addr,int port)
	{
		_IpAddr(addr,port);
//		if (_IpAddr==0)
//			return(StErr_RETURN);
		if (_SockBind())
			return(StErr_RETURN);
		return(_SockListen());
	}
	inline StSize _SockListen(int port)
	{
		_IpAddr(0,port);
		if (_IpAddr==0)
			return(StErr_RETURN);
		if (_SockBind())
			return(StErr_RETURN);
		return(_SockListen());
	}

	StBaseServ *_ServiceAccept(void)
	{
		// temporary storage for new connnection address
		struct sockaddr_in new_addr;
		sockaddr_size_type addrlen=sizeof(struct sockaddr);

		// wait here on accept call for incoming connection
		int new_socket_handle=accept(_socket_handle,(struct sockaddr*)&new_addr,&addrlen);

		if (new_socket_handle==SOCKET_ERROR)
		{
			_SockError("accept");
			return(NULL);
		}

		// create a new service (socket class) to handle this connection
		StSock *NewSock=(StSock*)_ServiceCreate();

		// initialize the new class

		NewSock->_protocol=_protocol;
		NewSock->_socket_handle=new_socket_handle;

		NewSock->_IpAddr._sockaddr=new_addr;
		
		return(NewSock);
	}
};

#endif
