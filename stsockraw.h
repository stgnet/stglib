// STGLIB/sraw.h
// Copyright 1999 by StG Net

// tcp class

#ifndef STGLIB_STSOCKRAW
#define STGLIB_STSOCKRAW

#pragma message("using stsockraw.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stsock.h"

#ifdef WIN32
// why isn't this defined in winsock2.h???
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
#endif


#ifndef PROTOENT
#define PROTOENT struct protoent
#endif

typedef struct
{
	StBig2 SourcePort;
	StBig2 DestinationPort;
	StBig4 SequenceNumber;
	StBig4 Acknowledgement;
	StBig1 Reserved:4, DataOffset:4;
	StBig1 Flags;
	StBig2 Window;
	StBig2 Checksum;
	StBig2 Urgent;

} StTcpHeader;

//typedef struct
class StUdpHeader
{
public:
	StBig2 SourcePort;
	StBig2 DestinationPort;
	StBig2 Length;
	StBig2 Checksum;

	StBaseStatic Data(void)
	{
		StBaseStatic payload;
		payload((StByte*)this+sizeof(StUdpHeader),Length-sizeof(StUdpHeader));
		return(payload);
	}
};
// StUdpHeader;

typedef struct
{
	StBig1 Type;
	StBig1 Code;
	StBig2 Checksum;
	StBig2 Identifier;
	StBig2 Sequence;

} StIcmpHeader;

/*
typedef union
{
	StTcpHeader Tcp;
	StUdpHeader Udp;
	StIcmpHeader Icmp;

} StIpSubHeader;
*/

class StIpHeader
{
public:
	StBig1 IpHeaderLength:4, IpVersion:4;
	StBig1 TypeOfService;
	StBig2 TotalLength;
	StBig2 Identification;
	StBig2 FragmentOffset;
	StBig1 TimeToLive;
	StBig1 Protocol;
	StBig2 Checksum;
	StBig4 SourceAddr;
	StBig4 DestinationAddr;

/*	StIpSubHeader* operator->()
	{
		return((StIpSubHeader*)((StByte*)this+IpHeaderLength));
	}
*/
	StTcpHeader *Tcp(void)
	{
		if (Protocol!=IPPROTO_TCP) return(0);
		if (IpHeaderLength*4<20) return(0);
		return((StTcpHeader*)((StByte*)this+IpHeaderLength*4));
	}
	StUdpHeader *Udp(void)
	{
		if (Protocol!=IPPROTO_UDP) return(0);
		if (IpHeaderLength*4<20) return(0);
		return((StUdpHeader*)((StByte*)this+IpHeaderLength*4));
	}
	StIcmpHeader *Icmp(void)
	{
		if (Protocol!=IPPROTO_ICMP) return(0);
		if (IpHeaderLength*4<20) return(0);
		return((StIcmpHeader*)((StByte*)this+IpHeaderLength*4));
	}
	StBaseStatic Data(void)
	{
		StBaseStatic payload;
		StByte *ptr=NULL;
		StSize len=ntohs(TotalLength);
		switch(Protocol)
		{
		case IPPROTO_UDP:
			ptr=(StByte*)((StByte*)this+IpHeaderLength*4)+sizeof(StUdpHeader);
			len=ntohs(Udp()->Length)-sizeof(StUdpHeader);
			break;
		}
		payload(ptr,len);
		return(payload);
	}

	void _DumpTo(StBuffer &out,StBuffer &pkt)
	{
		StIpAddr Source;
		StIpAddr Destination;

		out<<"------: IP\n";

		if (TotalLength>~pkt)
		{
			out<<"Invalid length - dumping entire packet\n";
			StDump hexdump(out);
                        StByte *ptr=(StByte*)pkt;
                        StByte *end=(StByte*)pkt+~pkt;
                        hexdump._Write(ptr,end-ptr);
			return;
		}

		Source(SourceAddr);
		Destination(DestinationAddr);

		out<<"Source: "<< Source<<"\n";
		out<<"Dest  : "<< Destination<<"\n";


		switch (Protocol)
		{
		case IPPROTO_ICMP:
			out<<"------: ICMP\n";
			break;

		case IPPROTO_TCP:
			out<<"------: TCP\n";
			break;
		case IPPROTO_UDP:
			out<<"------: UDP\n";
			break;
		default:
			out<<"------: UNKNOWN PROTOCOL "<<Protocol<<"\n";
			break;
		}
		


/*
                switch (Protocol)
                {
                case IPPROTO_ICMP:
                        Source._SetNetworkAddr((*this)->SourceAddr);
                        Dest._SetNetworkAddr((*this)->DestinationAddr);

                        dump_buf<<"ICMP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";

                        dump_buf<<"Type="<<(*this)->Icmp()->Type;
                        if ((*this)->Icmp()->Type<(sizeof(Icmp_Type)/sizeof(char*)))
                                dump_buf<<" "<<Icmp_Type[(*this)->Icmp()->Type];

                        dump_buf<<" Code="<<(*this)->Icmp()->Code;

                        dump_buf<<"\n";

                        {
                                StByte *ptr=(StByte*)((*this)->Icmp())+sizeof(StIcmpHeader);
                                StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
                                hexdump._Write(ptr,end-ptr);
                        }
                        break;

                case IPPROTO_TCP:
                        Source._SetNetworkAddr((*this)->SourceAddr,(*this)->Tcp()->SourcePort);
                        Dest._SetNetworkAddr((*this)->DestinationAddr,(*this)->Tcp()->DestinationPort);

                        dump_buf<<"TCP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";

//                      dump_buf<<"TCP FM:"<<ntohs((*this)->Tcp()->SourcePort)<<" TO:"<<ntohs((*this)->Tcp()->DestinationPort)<<"\n";
                        {
                                StByte *ptr=(StByte*)((*this)->Tcp())+sizeof(StTcpHeader);
                                StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
                                hexdump._Write(ptr,end-ptr);
                        }

//                              dump_buf<<"TotalLength="<<ntohs((*this)->TotalLength)<<"\n";
                        break;

                case IPPROTO_UDP:
                        Source._SetNetworkAddr((*this)->SourceAddr,(*this)->Udp()->SourcePort);
                        Dest._SetNetworkAddr((*this)->DestinationAddr,(*this)->Udp()->DestinationPort);

                        dump_buf<<"UDP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";
//                      dump_buf<<"UDP FM:"<<ntohs((*this)->Udp()->SourcePort)<<" TO:"<<ntohs((*this)->Udp()->DestinationPort)<<"\n";
                        {
                                StByte *ptr=(StByte*)((*this)->Udp())+sizeof(StUdpHeader);
                                StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
                                hexdump._Write(ptr,end-ptr);
                        }
                        break;

                default:
                        Source._SetNetworkAddr((*this)->SourceAddr);
                        Dest._SetNetworkAddr((*this)->DestinationAddr);

                        dump_buf<<" *** PROTOCOL #"<<(*this)->Protocol;

                        {
                                PROTOENT *p=getprotobynumber((*this)->Protocol);
                                if (p)
                                {
                                        dump_buf<<" ("<<p->p_name<<")";
                                }
                        }
                        dump_buf<<"  <FROM:"<<Source<<" >TO:"<<Dest<<"\n";
                        {
                                StByte *ptr=(StByte*)this+(*this)->IpHeaderLength*4;
                                StByte *end=(StByte*)this+ntohs((*this)->TotalLength);
                                hexdump._Write(ptr,end-ptr);
                        }
                        break;
                }
*/


	}




};

class StEthHeader
{
public:
	StMacAddr EthDestinationAddr;
	StMacAddr EthSourceAddr;
	StBig2 EthTypeCode;

	StIpHeader *Ip(void)
	{
		if (EthTypeCode!=0x0800) return(0);
		return((StIpHeader*)((StByte*)this+6+6+2));
	}

	void _DumpTo(StBuffer &out,StBuffer &pkt)
	{
		out<<"------: ETHERNET\n";
		out<<"Source: "<<EthSourceAddr.ColonFormat()<<"\n";
		out<<"Dest  : "<<EthDestinationAddr.ColonFormat()<<"\n";
		out<<"Type  : "<<EthTypeCode<<"\n";

		switch (EthTypeCode)
		{
		case 0x0800:
			Ip()->_DumpTo(out,pkt);
			break;
		case 0x0806:
			out<<"        ARP PACKET\n";
			break;
		default:
			out<<"        UNKNOWN PACKET\n";
			break;
		}
			
	}
};



#ifdef BOGUS_ALREADY_DEFINED_JUST_FOR_REFERENCE
enum
{
    IPPROTO_IP = 0,        /* Dummy protocol for TCP.  */
    IPPROTO_HOPOPTS = 0,   /* IPv6 Hop-by-Hop options.  */
    IPPROTO_ICMP = 1,      /* Internet Control Message Protocol.  */
    IPPROTO_IGMP = 2,      /* Internet Group Management Protocol. */
    IPPROTO_IPIP = 4,      /* IPIP tunnels (older KA9Q tunnels use 94).  */
    IPPROTO_TCP = 6,       /* Transmission Control Protocol.  */
    IPPROTO_EGP = 8,       /* Exterior Gateway Protocol.  */
    IPPROTO_PUP = 12,      /* PUP protocol.  */
    IPPROTO_UDP = 17,      /* User Datagram Protocol.  */
    IPPROTO_IDP = 22,      /* XNS IDP protocol.  */
    IPPROTO_TP = 29,       /* SO Transport Protocol Class 4.  */
    IPPROTO_IPV6 = 41,     /* IPv6 header.  */
    IPPROTO_ROUTING = 43,  /* IPv6 routing header.  */
    IPPROTO_FRAGMENT = 44, /* IPv6 fragmentation header.  */
    IPPROTO_RSVP = 46,     /* Reservation Protocol.  */
    IPPROTO_GRE = 47,      /* General Routing Encapsulation.  */
    IPPROTO_ESP = 50,      /* encapsulating security payload.  */
    IPPROTO_AH = 51,       /* authentication header.  */
    IPPROTO_ICMPV6 = 58,   /* ICMPv6.  */
    IPPROTO_NONE = 59,     /* IPv6 no next header.  */
    IPPROTO_DSTOPTS = 60,  /* IPv6 destination options.  */
    IPPROTO_MTP = 92,      /* Multicast Transport Protocol.  */
    IPPROTO_ENCAP = 98,    /* Encapsulation Header.  */
    IPPROTO_PIM = 103,     /* Protocol Independent Multicast.  */
    IPPROTO_COMP = 108,    /* Compression Header Protocol.  */
    IPPROTO_RAW = 255,     /* Raw IP packets.  */
    IPPROTO_MAX
};
#endif

const char *Icmp_Type[]=
{
	"Ping Reply",
	"1",
	"2",
	"Destination Unreachable",
	"Source Quench",
	"Redirect",
	"6",
	"7",
	"Ping Request",
	"9",
	"10",
	"Time Exceeded",
	"Parameter Problem",
	"Timestamp Query",
	"Timestamp Reply",
	"15",
	"16",
	"Address Mask Request",
	"Address Mask Reply",
	0
};

// destination unreachable
const char *Icmp_Code_Type3[]=
{
	"Network Unreachable",
	"Host Unreachable",
	"Protocol Unreachable",
	"Port Unreachable",
	"Fragmentation Needed and DF set",
	"Source Route Failed",
	"Destination network unknown",
	"Destination host unknown",
	"Source host isolated",
	"Communication with detination network administratively prohibited",
	"Comunication with destination host administratively prohibited",
	"Network unreachable for type of service",
	"Host unreachable for type of service",
	0
};

// Redirect
const char *Icmp_Code_Type5[]=
{
	"Network",
	"Host",
	"Type of service and Network",
	"Type of service and Host",
	0
};

// Time Exceeded
const char *Icmp_Code_Type11[]=
{
	"Time To Live exceeded",
	"Fragment Reassembly Time exceeded",
	0
};

// Parameter Problem
const char *Icmp_Code_Type12[]=
{
	"Pointer identifies error",
	"required option missing",
	0
};



class StIpPacket:public StBuffer
{
	StBuffer dump_buf;
public:
	StIpPacket()
	{
		// tell base this is a packet style buffer, not a stream
//		_SetPacketMode();
		// this is now done by defining _IsPacket()
	}

	virtual ~StIpPacket()
	{
	}

	inline virtual char _IsPacket(void)
	{
		return(1);
	}

	// must declare assignment operator to prevent munged ptrs
	// (is this still necessary since _Storage is a member???)
	StIpPacket& operator= (StIpPacket& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}

	StIpHeader* operator ->()
	{
		return((StIpHeader*)((void*)(*(StBuffer*)this)));
	}
/*	StIpSubHeader* operator->*()
	{
		return((StIpSubHeader*)((StByte*)this+(*this)->IpHeaderLength*4));
	}
*/



	StBuffer &Dump(StIpAddr *Netmask)
	{
//		StIpAddr Source((*this)->SourceAddr);
//		StIpAddr Dest((*this)->DestinationAddr);

		StIpAddr Source;
		StIpAddr Dest;
		Source._SetNetmask(*Netmask);
		Dest._SetNetmask(*Netmask);

		StDump hexdump(dump_buf);


		!dump_buf<<"---";
//		dump_buf<<"FM:"<<Source<<"\n";
//		dump_buf<<"TO:"<<Dest<<"\n";
//		dump_buf<<"IP hdr size="<<(*this)->IpHeaderLength<<"\n";
//			dump_buf<<"Protocol="<<(*this)->Protocol<<"\n";


		if (ntohs((*this)->TotalLength) > ~(*this))
		{
			dump_buf<<"Invalid length - dumping entire packet\n";
			StByte *ptr=(StByte*)(*this);
			StByte *end=(StByte*)(*this)+~(*this);
			hexdump._Write(ptr,end-ptr);
		}
		else
		switch ((*this)->Protocol)
		{
		case IPPROTO_ICMP:
			Source._SetNetworkAddr((*this)->SourceAddr);
			Dest._SetNetworkAddr((*this)->DestinationAddr);

			dump_buf<<"ICMP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";

			dump_buf<<"Type="<<(*this)->Icmp()->Type;
			if ((*this)->Icmp()->Type<(sizeof(Icmp_Type)/sizeof(char*)))
				dump_buf<<" "<<Icmp_Type[(*this)->Icmp()->Type];
			
			dump_buf<<" Code="<<(*this)->Icmp()->Code;

			dump_buf<<"\n";

			{
				StByte *ptr=(StByte*)((*this)->Icmp())+sizeof(StIcmpHeader);
				StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
				hexdump._Write(ptr,end-ptr);
			}
			break;

		case IPPROTO_TCP:
			Source._SetNetworkAddr((*this)->SourceAddr,(*this)->Tcp()->SourcePort);
			Dest._SetNetworkAddr((*this)->DestinationAddr,(*this)->Tcp()->DestinationPort);

			dump_buf<<"TCP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";

//			dump_buf<<"TCP FM:"<<ntohs((*this)->Tcp()->SourcePort)<<" TO:"<<ntohs((*this)->Tcp()->DestinationPort)<<"\n";
			{
				StByte *ptr=(StByte*)((*this)->Tcp())+sizeof(StTcpHeader);
				StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
				hexdump._Write(ptr,end-ptr);
			}

//				dump_buf<<"TotalLength="<<ntohs((*this)->TotalLength)<<"\n";
			break;

		case IPPROTO_UDP:
			Source._SetNetworkAddr((*this)->SourceAddr,(*this)->Udp()->SourcePort);
			Dest._SetNetworkAddr((*this)->DestinationAddr,(*this)->Udp()->DestinationPort);

			dump_buf<<"UDP <FROM:"<<Source<<" >TO:"<<Dest<<"\n";
//			dump_buf<<"UDP FM:"<<ntohs((*this)->Udp()->SourcePort)<<" TO:"<<ntohs((*this)->Udp()->DestinationPort)<<"\n";
			{
				StByte *ptr=(StByte*)((*this)->Udp())+sizeof(StUdpHeader);
				StByte *end=(StByte*)((*this))+ntohs((*this)->TotalLength);
				hexdump._Write(ptr,end-ptr);
			}
			break;

		default: 
			Source._SetNetworkAddr((*this)->SourceAddr);
			Dest._SetNetworkAddr((*this)->DestinationAddr);

			dump_buf<<" *** PROTOCOL #"<<(*this)->Protocol;
			
			{
				PROTOENT *p=getprotobynumber((*this)->Protocol);
				if (p)
				{
					dump_buf<<" ("<<p->p_name<<")";
				}
			}
			dump_buf<<"  <FROM:"<<Source<<" >TO:"<<Dest<<"\n";
			{
				StByte *ptr=(StByte*)this+(*this)->IpHeaderLength*4;
				StByte *end=(StByte*)this+ntohs((*this)->TotalLength);
				hexdump._Write(ptr,end-ptr);
			}
			break;
		}

		return(dump_buf);
	}

};

class StEthPacket:public StBuffer
{
	StString dump_buf;
public:
	StEthPacket()
	{
		if (sizeof(class StEthHeader)!=6+6+2) _Die("Invalid sizeof(StEthHeader)");
	}
	virtual ~StEthPacket()
	{
	}
	inline virtual char _IsPacket(void)
	{
		return(1);
	}
	StEthPacket& operator= (StEthPacket& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}
	StEthHeader * operator ->()
	{
		return((StEthHeader*)((void*)(*(StBuffer*)this)));
	}
	StBuffer &_Dump(void)
	{
		(*this)->_DumpTo(dump_buf,*this);
		return(dump_buf);
	}
};



class StSockRaw:public StSock
{
public:
	// this form presumes there is only one local address
	StSockRaw():StSock(IPPROTO_IP)
	{
		_IpAddr._Set(PF_PACKET,0x0003); //ETH_P_ALL);
		_SockBind();
//		_SockSetOption(SIO_RCVALL,1); // enable receipt of all packets
	}

	// this form specifies the address
// these really doesn't work - instead just get all ether packets and filter manually
/*
	StSockRaw(StIpAddr &addr):StSock(IPPROTO_IP)
	{
		_SockBind(addr);
//		_SockSetOption(SIO_RCVALL,1); // enable receipt of all packets

	}
	StSockRaw(char *addr,int port=0):StSock(IPPROTO_IP)
	{
		_IpAddr(addr,port);
		_SockBind();
//		_SockSetOption(SIO_RCVALL,1); // enable receipt of all packets

	}
*/
};

#endif
