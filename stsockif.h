// STGLIB/stsockif.h
// Copyright 1999 by StG Net

// sock based inspection of available interfaces

// On instantiation, StSockIfs becomes a box containing a list of StSockIf
// instances, each describing an available interface

/*	EXAMPLE CODE:
	StSockIfs nics;
	if (nics._Error)
	{
		StdOutput<<"Error obtaining interface list: "<<nics._GetErrorString()<<"\n";
		exit(1);
	}

	StBoxRef<StSockIf> nic(nics);
	while (++nic)
	{
		StdOutput<<nic->Name<<"\n";
	}
*/


// original implementation based on winsock
// new implementation using extra capabilities from iphlpapi.lib

#ifndef STGLIB_STSOCKIF
#define STGLIB_STSOCKIF

#pragma message("using stsockif.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stsock.h"
#include "/src/stglib/stbox.h"


//win32:
//#ifndef SIO_GET_INTERFACE_LIST
//#include <ws2tcpip.h>
//#endif


class StSockConf:public StSock
{
public:
	StSockConf():StSock(IPPROTO_CONF)
	{
	}
};


class StSockIf
{
public:
	StIpAddr IpAddress;
	StIpAddr Netmask;
	StIpAddr Broadcast;
	StByt4 Flags;
	StString Name;
};


class StSockIfs:public StBox<StSockIf>
{
public:
	StSockIfs()
	{
		StSockConf sock;
		StBuffer buffer;
		buffer._ArrayAllocate(sizeof(struct ifreq)*16);

		struct ifconf ifc;
		ifc.ifc_len=buffer._ArraySize();
		ifc.ifc_buf=(char*)(StByte*)buffer;

		StSize got=sock._SockIoctl(SIOCGIFCONF,(StByte*)&ifc);
		if (sock._Error)
		{
			_Err(sock);
			return;
		}

		int count=ifc.ifc_len/sizeof(struct ifreq);
		int index=0;
		while (index<count)
		{
			struct ifreq *pi=&ifc.ifc_req[index];
			StSockIf &nif=+(*this);
			nif.Name<<pi->ifr_name;
			nif.IpAddress(*((struct sockaddr_in*)&(pi->ifr_addr)));
			++pi;
			++index;
		}
	}
};



#ifdef REST_OF_THIS_IS_BOGUS_WIN32

//#include <iphlpapi.h>
// MSVC6 doesn't have IPTYPES.H (used by IPHLPAPI.H) so yank FIXED_INFO struct manually 

// Definitions and structures used by getnetworkparams and getadaptersinfo apis

#define MAX_ADAPTER_DESCRIPTION_LENGTH  128 // arb.
#define MAX_ADAPTER_NAME_LENGTH         256 // arb.
#define MAX_ADAPTER_ADDRESS_LENGTH      8   // arb.
#define DEFAULT_MINIMUM_ENTITIES        32  // arb.
#define MAX_HOSTNAME_LEN                128 // arb.
#define MAX_DOMAIN_NAME_LEN             128 // arb.
#define MAX_SCOPE_ID_LEN                256 // arb.

//
// IP_ADDRESS_STRING - store an IP address as a dotted decimal string
//

typedef struct {
    char String[4 * 4];
} IP_ADDRESS_STRING, *PIP_ADDRESS_STRING, IP_MASK_STRING, *PIP_MASK_STRING;

//
// IP_ADDR_STRING - store an IP address with its corresponding subnet mask,
// both as dotted decimal strings
//

typedef struct _IP_ADDR_STRING {
    struct _IP_ADDR_STRING* Next;
    IP_ADDRESS_STRING IpAddress;
    IP_MASK_STRING IpMask;
    DWORD Context;
} IP_ADDR_STRING, *PIP_ADDR_STRING;

//
// FIXED_INFO - the set of IP-related information which does not depend on DHCP
//

typedef struct {
    char HostName[MAX_HOSTNAME_LEN + 4] ;
    char DomainName[MAX_DOMAIN_NAME_LEN + 4];
    PIP_ADDR_STRING CurrentDnsServer;
    IP_ADDR_STRING DnsServerList;
    UINT NodeType;
    char ScopeId[MAX_SCOPE_ID_LEN + 4];
    UINT EnableRouting;
    UINT EnableProxy;
    UINT EnableDns;
} FIXED_INFO, *PFIXED_INFO;

DWORD
WINAPI
GetNetworkParams(
    PFIXED_INFO pFixedInfo, PULONG pOutBufLen
    );


//
// ADAPTER_INFO - per-adapter information. All IP addresses are stored as
// strings
//

typedef struct _IP_ADAPTER_INFO {
    struct _IP_ADAPTER_INFO* Next;
    DWORD ComboIndex;
    char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];
    char Description[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
    UINT AddressLength;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
    DWORD Index;
    UINT Type;
    UINT DhcpEnabled;
    PIP_ADDR_STRING CurrentIpAddress;
    IP_ADDR_STRING IpAddressList;
    IP_ADDR_STRING GatewayList;
    IP_ADDR_STRING DhcpServer;
    BOOL HaveWins;
    IP_ADDR_STRING PrimaryWinsServer;
    IP_ADDR_STRING SecondaryWinsServer;
    time_t LeaseObtained;
    time_t LeaseExpires;
} IP_ADAPTER_INFO, *PIP_ADAPTER_INFO;

DWORD
WINAPI
GetAdaptersInfo(
    PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen
    );

typedef ULONG IPAddr;       // An IP address.

DWORD
WINAPI
SendARP(
    IPAddr DestIP,
    IPAddr SrcIP,
    PULONG pMacAddr,
    PULONG  PhyAddrLen
    );


#pragma comment(linker, "/defaultlib:iphlpapi.lib")

// new implementation gives more data on if
class StSockIf
{
public:
	StString Name;
	StIpAddr IpAddress;
	StIpAddr Netmask;
	StIpAddr Broadcast;
	StIpAddr Gateway;
	StIpAddr DHCP_Server;
	StIpAddr DNS_Primary;
	StIpAddr DNS_Secondary;
	StByt4 Flags;

};


class StSockIfs:public StBox<StSockIf>
{
public:
	StSockIfs()
	{
		// instantiating this class causes it to obtain the list of adapters
		StIpAddr DNS1;
		StIpAddr DNS2;

		// first get "fixed info" to get DNS servers
		ULONG filen=sizeof(FIXED_INFO);
		FIXED_INFO *fi=(FIXED_INFO*)malloc(filen);
		DWORD result=GetNetworkParams(fi,&filen);
		if (result==ERROR_BUFFER_OVERFLOW)
		{
			// resize and re-get
			free(fi);
			fi=(FIXED_INFO*)malloc(filen);
			result=GetNetworkParams(fi,&filen);
		}
		if (result==NO_ERROR)
		{
			// store the DNS entries for later use
			DNS1(fi->DnsServerList.IpAddress.String);
			if (fi->DnsServerList.Next)
				DNS2(fi->DnsServerList.Next->IpAddress.String);
		}

		ULONG ailen=3*sizeof(IP_ADAPTER_INFO);
		IP_ADAPTER_INFO *ai=(IP_ADAPTER_INFO*)malloc(ailen);

		result=GetAdaptersInfo(ai,&ailen);
		if (result==ERROR_BUFFER_OVERFLOW)
		{
			// resize and re-get
			free(ai);
			ai=(IP_ADAPTER_INFO*)malloc(ailen);
			result=GetAdaptersInfo(ai,&ailen);
		}
		if (result!=NO_ERROR)
		{
			// bail
			return;
		}

		IP_ADAPTER_INFO *scan=ai;

		while (scan)
		{
			StSockIf *sif=+(*this);
			sif->Name<<scan->Description;



			sif->IpAddress(scan->IpAddressList.IpAddress.String);
			sif->Netmask(scan->IpAddressList.IpMask.String);
			sif->IpAddress._SetNetmask(sif->Netmask);

			sif->Gateway(scan->GatewayList.IpAddress.String);

			if (scan->DhcpEnabled)
				sif->DHCP_Server(scan->DhcpServer.IpAddress.String);

			// and copy in the DNS servers previously found
			sif->DNS_Primary=DNS1;
			sif->DNS_Secondary=DNS2;

			scan = scan->Next;
		}
	}
};

#endif
#endif

