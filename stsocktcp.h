// STGLIB/stsocktcp.h
// Copyright 1999 by StG Net

// tcp class

#ifndef STGLIB_STSOCKTCP
#define STGLIB_STSOCKTCP

#pragma message("using stsocktcp.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stsock.h"


class StSockTcp:public StSock
{
public:
	// normal usage
	StSockTcp():StSock(IPPROTO_TCP)
	{
		STBASE_DEBUG_CON("StSockTcp");
		// presumption is that caller will set address and ask for connection
	}

	StSockTcp(const char *addr,int port=0):StSock(IPPROTO_TCP)
	{
		STBASE_DEBUG_CON("StSockTcp");
		STBASE_DEBUG_CON(addr);

		// set address and ask for connection
		_SockConnect(addr,port);
	}

/*	// SERVICE usage
	StSockTcp(StSockTcp *acceptor):StSock(IPPROTO_TCP)
	{
		// this constructor is called when connection is opened
		// the acceptor class from which this was created is presumed unimportant,
		// but might be useful
	}
*/
	StSockTcp *_SockAccept(void)
	{
		struct sockaddr_in new_addr;
		sockaddr_size_type addrlen=sizeof(struct sockaddr);

		if (_SockSelectRead()) return(NULL);

		int new_sock=accept(_socket_handle,(struct sockaddr*)&new_addr,&addrlen);
		if (new_sock==SOCKET_ERROR)
		{
			_SockError("accept");
			return(NULL);
		}

		// return ptr to new socket class
		StSockTcp *NewSock=new StSockTcp;
		NewSock->_protocol=_protocol;
		NewSock->_socket_handle=new_sock;
		NewSock->_IpAddr._sockaddr=new_addr;
		return(NewSock);
	}

	virtual ~StSockTcp()
	{
	}
};

#endif
