// STGLIB/sudp.h
// Copyright 1999 by StG Net

// udp class

#ifndef STGLIB_STUDP
#define STGLIB_STUDP

#pragma message("using studp.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stsock.h"


// this probably needs to be put in it's own stpacket.h
/* OKAY, THIS DOESN"T WORK - packet doesn't know how to get address from SockUdp!
class StUdpPacket:public StBase
{
public:
	StIpAddr Address;

	StBase *Data;

	StUdpPacket(StBase &data_buffer)
	{
		Data=&data_buffer;
	}

	virtual StSize _Read(StBase &Source)
	{
		StSize got=Data->_Read(Source);
		Address=
		return();
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		return(Data->_WriteTo(Destination));
	}

};

*/


class StSockUdp:public StSock
{
public:
	StIpAddr _PacketAddress;
	int _BroadcastEnable;

	// normal usage
	StSockUdp():StSock(IPPROTO_UDP)
	{
		_BroadcastEnable=0;
	}

	StSockUdp(StIpAddr &addr):StSock(IPPROTO_UDP)
	{
		_BroadcastEnable=0;
		// set address and ask for connection
		_SockBind(addr);
	}
	virtual StSize operator()(StIpAddr &addr)
	{
//		_IpAddr(addr);
		return(_SockBind(addr));
	}

	StSize _EnableBroadcast(void)
	{
		_BroadcastEnable=1;
		// enable broadcasts on the socket
		if (setsockopt(_socket_handle,SOL_SOCKET,SO_BROADCAST,(char*)&_BroadcastEnable,sizeof(_BroadcastEnable))==SOCKET_ERROR)
			return(_SockError("Broadcast-enable"));
		return(0);
	}


	StSize _Write(const StByte *data,StSize size)
	{
		StIpAddr TransmitAddress(_PacketAddress);

		// is it a boadcast?
		if (_PacketAddress._sockaddr.sin_addr.s_addr==INADDR_ANY)
		{
			if (!_BroadcastEnable)
				if (_EnableBroadcast())
					return(_Error);

			// if this port was bound to an IP, need to use broadcast address format
			if (_IpAddr!=0)
				// mangle TransmitAddress to _IpAddr
				TransmitAddress=_IpAddr|~_IpAddr._GetNetmask();
			else
				TransmitAddress(INADDR_BROADCAST);

			TransmitAddress._SetPort(_PacketAddress._GetPort());
		}

		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(_socket_handle,&fd);
		switch (select(_socket_handle+1,0,&fd,0,&_socket_timeout))
		{
		case SOCKET_ERROR:
			return(_SockError("Write-select"));
		case 0:
			return(_Err(StErr_Timeout,"_Write","timeout"));
		}

//StdOutput<<"Sending "<<size<<" bytes to "<<TransmitAddress<<"\n";
		StSize put=sendto(_socket_handle,(char*)data,size,0,(const struct sockaddr*)&(TransmitAddress._sockaddr),sizeof(_PacketAddress._sockaddr));

		if (put==SOCKET_ERROR)
			return(_SockError("send"));

		return(put);
	}

	StSize _Read(StByte *data,StSize size)
	{
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(_socket_handle,&fd);

		switch (select(_socket_handle+1,&fd,0,0,&_socket_timeout))
		{
		case SOCKET_ERROR:
			return(_SockError("Read-select"));
		case 0:
			return(_Err(StErr_Timeout,"_Read","timeout"));
		}

		socklen_t fromlen=sizeof(_PacketAddress._sockaddr);
		StSize got=recvfrom(_socket_handle,(char*)data,size,0,(struct sockaddr*)&(_PacketAddress._sockaddr),&fromlen);
		if (got==SOCKET_ERROR)
			return(_SockError("recv"));

		return(got);
	}
};

#endif
