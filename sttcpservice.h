// STGLIB/stcp.h
// Copyright 1999,2002 by StG Net

// this class wraps the stcp class to add a few extra calls for writing servers

#ifndef STGLIB_STTCPSERVICE
#define STGLIB_STTCPSERVICE

#pragma message("using sttcpservice.h")

#include "/stglib/sttcp.h"

class StTcpService:public StTcp
{
public:
	StTcpService():StTcp()
	{
	}

	StTcpService(char *addr,int port=0):StTcp(addr,port)
	{
	}

	// assuming socket already bound
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
	inline StSize_SockListen(char *addr,int port)
	{
		if (_SetAddress(addr,port))
			return(StErr_RETURN);
		if (_SockBind())
			return(StErr_RETURN);
		return(_SockListen());
	}

	// inheriting class *must* declare how to create a new service
	virtual StTcpService* _ServiceCreate(int socket,struct sockaddr_in *address)=0;
	// _ServiceCreate must also copy socket and address into StSock!

	StSock *_SockAccept(void)
	{
		struct sockaddr_in new_address;
		int addrlen=sizeof(new_address);

		int new_socket=accept(_socket_handle,(struct sockaddr*)&new_address,&addrlen);
		if (new_socket==SOCKET_ERROR)
		{
			_SockError("accept");
			return(NULL);
		}
		return(_ServiceCreate(new_socket,&new_address));
	}
};

#endif
