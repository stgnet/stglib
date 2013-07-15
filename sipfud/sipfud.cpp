#include "/src/stglib/stdio.h"
#include "/src/stglib/stsockraw.h"

// SIP FailUre Detector


/* SIP info from http://www.ietf.org/rfc/rfc3261.txt

8.1.1 Generating the Request

   A valid SIP request formulated by a UAC MUST, at a minimum, contain
   the following header fields: To, From, CSeq, Call-ID, Max-Forwards,
   and Via; all of these header fields are mandatory in all SIP
   requests. 



      Subject:            lunch
      Subject      :      lunch
      Subject            :lunch
      Subject: lunch

   Thus, the above are all valid and equivalent, but the last is the
   preferred form.

   Header fields can be extended over multiple lines by preceding each
   extra line with at least one SP or horizontal tab (HT).  The line
   break and the whitespace at the beginning of the next line are
   treated as a single SP character.  Thus, the following are
   equivalent:

      Subject: I know you're there, pick up the phone and talk to me!
      Subject: I know you're there,
               pick up the phone
               and talk to me!



   The following groups of header field rows are valid and equivalent:

      Route: <sip:alice@atlanta.com>
      Subject: Lunch
      Route: <sip:bob@biloxi.com>
      Route: <sip:carol@chicago.com>

      Route: <sip:alice@atlanta.com>, <sip:bob@biloxi.com>
      Route: <sip:carol@chicago.com>
      Subject: Lunch

      Subject: Lunch
      Route: <sip:alice@atlanta.com>, <sip:bob@biloxi.com>,
             <sip:carol@chicago.com>


      Contact: <sip:alice@atlanta.com>;expires=3600

   is equivalent to

      CONTACT: <sip:alice@atlanta.com>;ExPiReS=3600

   and

      Content-Disposition: session;handling=optional

   is equivalent to

      content-disposition: Session;HANDLING=OPTIONAL



         INVITE                   [RFC3261]
         ACK                      [RFC3261]
         BYE                      [RFC3261]
         CANCEL                   [RFC3261]
         REGISTER                 [RFC3261]
         OPTIONS                  [RFC3261]
         INFO                     [RFC2976]



message-header  =  (Accept
                /  Accept-Encoding
                /  Accept-Language
                /  Alert-Info
                /  Allow
                /  Authentication-Info
                /  Authorization
                /  Call-ID
                /  Call-Info
                /  Contact
                /  Content-Disposition
                /  Content-Encoding
                /  Content-Language
                /  Content-Length
                /  Content-Type
                /  CSeq
                /  Date
                /  Error-Info
                /  Expires
                /  From
                /  In-Reply-To
                /  Max-Forwards
                /  MIME-Version
                /  Min-Expires
                /  Organization
                /  Priority
                /  Proxy-Authenticate
                /  Proxy-Authorization
                /  Proxy-Require
                /  Record-Route
                /  Reply-To
                /  Require
                /  Retry-After
                /  Route
                /  Server
                /  Subject
                /  Supported
                /  Timestamp
                /  To
                /  Unsupported
                /  User-Agent
                /  Via
                /  Warning
                /  WWW-Authenticate
                /  extension-header) CRLF
*/



// rather than parse sip header into boxes of parsed string sections,
// perform a quick and dirty single buffer parse with dropped pointers
// and overwriting with zero terminations

class sipdecode
{
public:
	// first line
	const char *Request;
	const char *Method;
	const char *RequestURI;
	const char *StatusCode;
	const char *ReasonPhrase;

	const char *INVITE;
	const char *ACK;
	const char *BYE;
	const char *CANCEL;
	const char *REGISTER;
	const char *OPTIONS;
	const char *INFO;
	const char *PING;

	const char *Error;

	// headers
	const char *To;
	const char *From;
	const char *CSeq;
	const char *CallID;
	const char *MaxForwards;
	const char *Via;
	const char *UserAgent;
	const char *Allow;
	const char *ContentLength;
	const char *Supported;
	const char *AcceptLanguage;
	const char *Event;
	const char *Authorization;
	const char *Expires;
	const char *Date;
	const char *Server;
	const char *Reason;
	const char *PAssertedIdentity;
	const char *Timestamp;
	const char *Diversion;
	const char *Privacy;
	const char *Route;

	const char *EndOfPtrs;

	void Reset(void)
	{
		char **ptr=(char **)&Request;
		char **end=(char **)&EndOfPtrs;
		while (ptr<end)
			*ptr++=NULL;
	}
		
	inline int is_space(char c)
	{
		if (c==13 || c==10) return(0);
		if (c<=32) return(1);
		return(0);
	}
	inline int is_crlf(char c)
	{
		if (c==13 || c==10) return(1);
		return(0);
	}

	// skip non-space up to the next space, drop zero
	char *space(char *s,char *e)
	{
		while (s<e)
		{
			if (is_crlf(*s)) return(s);
			if (is_space(*s))
			{
				*s++=0;
				while (s<e && is_space(*s)) ++s;
				return(s);
			}
			++s;
		}
		return(s);
	}
	// skip non-crlf up to next crlf
	char *crlf(char *s,char *e)
	{
		while (s<e)
		{
			if (is_crlf(*s))
			{
				char first=*s;
				*s++=0;
				if (is_crlf(*s) && *s!=first) ++s;
				return(s);
			}
			++s;
		}
		return(s);
				
	}
	// skip to colon dropping at previous space, also eat trailing spaces
	char *colon(char *s,char *e)
	{
		char *d=NULL;
		while (s<e)
		{
			if (is_crlf(*s)) return(s);

			if (is_space(*s))
			{
				if (!d) d=s;
				++s;
				continue;
			}
			if (*s==':')
			{
				if (!d) d=s;
				*d=0;
				++s;

				while (s<e && is_space(*s)) ++s;
				return(s);
			}
			++s;
		}
		return(s);
			
	}
	void operator()(const char *sip,int len)
	{
		Reset();

/*
         INVITE                   [RFC3261]
         ACK                      [RFC3261]
         BYE                      [RFC3261]
         CANCEL                   [RFC3261]
         REGISTER                 [RFC3261]
         OPTIONS                  [RFC3261]
         INFO                     [RFC2976]
*/

/*
		unsigned short first2=name[0];
		first2<<=8;
		first2|=name[1];
		first2|=0x2020;
*/
		switch ((sip[0]<<8)|sip[1])
		{
		case 0x494E: 
			if (sip[2]=='V')
				Request=INVITE=sip;
			else
				Request=INFO=sip;
			break;
		case 0x4143: Request=ACK=sip; break;
		case 0x4259: Request=BYE=sip; break;
		case 0x4341: Request=CANCEL=sip; break;
		case 0x5245: Request=REGISTER=sip; break;
		case 0x4F50: Request=OPTIONS=sip; break;
		case 0x5049: Request=PING=sip; break;
		case 0x5349: /* SIP reply */ break;
		default:
			Error="Invalid first line"; return;
		}


		if (sip[0]!='S' || sip[1]!='I' || sip[2]!='P')
			Request=sip;

		Method=sip;

		char *e=(char *)sip+len;
		char *s=(char *)sip;
		// first line

		s=space(s,e);
		if (Request)
			RequestURI=s;
		else
			StatusCode=s;

		s=space(s,e);
		if (!Request)
			ReasonPhrase=s;

		s=crlf(s,e);

		// subsequent lines
		while (s<e)
		{
			if (is_crlf(*s)) break;
			char *name=s;
			s=colon(s,e);
//StdOutput<<"Name="<<name<<"\n";
			char *value=s;
			s=crlf(s,e);
//StdOutput<<"Value="<<value<<"\n";

			unsigned short first2=name[0];
			first2<<=8;
			first2|=name[1];
			first2|=0x2020;
			switch (first2)
			{
			case 0x746F: To=value; break;
			case 0x6672: From=value; break;
			case 0x6373: CSeq=value; break;
			case 0x6361: CallID=value; break;
			case 0x6D61: MaxForwards=value; break;
			case 0x7669: Via=value; break;
			case 0x7573: UserAgent=value; break;
			case 0x616C: Allow=value; break;
			case 0x636F: ContentLength=value; break;
			case 0x7375: Supported=value; break;
			case 0x6163: AcceptLanguage=value; break;
			case 0x6576: Event=value; break;
			case 0x6175: Authorization=value; break;
			case 0x6578: Expires=value; break;
			case 0x6461: Date=value; break;
			case 0x7365: Server=value; break;
			case 0x7265: Reason=value; break;
			case 0x702D: PAssertedIdentity=value; break;
			case 0x7469: Timestamp=value; break;
			case 0x6469: Diversion=value; break;
			case 0x7072: Privacy=value; break;
			case 0x726F: Route=value; break;

			case 0x7777: /* IGNORE WWW-Authenticate */ break;
			case 0x782D: /* IGNORE X- */ break;
			case 0x6D69: /* IGNORE Min-SE */ break;
			case 0x6369: /* IGNORE Cisco-GUID */ break;
			default:
				StdOutput<<"Undecoded header: "<<name<<" : "<<value<<"\n";
				exit(1);
			}

		}
	}
};



int main(void)
{
	StIpAddr port(0,5060);
	StIpAddr mask("255.255.255.0");

	StSockRaw raw;

//	StIpPacket p;
	StEthPacket p;

	if (raw._Error) raw._ShowErrorExit(StdOutput);


	sipdecode sip;

	while (1)
	{
//StdOutput<<".";
		p<<raw;
		if (p._Error) p._ShowErrorExit(StdOutput);


		StIpHeader *ip=p->Ip();
		if (ip)
		{
			StUdpHeader *udp=ip->Udp();

			if (udp)
			{
				if (udp->SourcePort==5060 || udp->DestinationPort==5060)
				{


					StIpAddr Source;
					StIpAddr Destination;

					Source(ip->SourceAddr);
					Destination(ip->DestinationAddr);
					
					StdOutput<<"\n### "<<Source<<":"<<udp->SourcePort<<" ==> "<<Destination<<":"<<udp->DestinationPort<<"\n";


					StBuffer msg;
					udp->Data()>>msg;

					sip((const char*)msg,~msg);
					if (sip.Error)
					{
						StdOutput<<"*** DECODE ERROR!\n";
						StdOutDump<<msg;
					
						continue;
					}

					if (sip.Request)
					{
						StdOutput<<sip.Method<<" "<<sip.RequestURI<<"\n";
						StdOutput<<"      FROM: "<<sip.From<<"\n";
						StdOutput<<"        TO: "<<sip.To<<"\n";
						StdOutput<<"\n";
					}
					else
					{
						StdOutput<<sip.StatusCode<<" = "<<sip.ReasonPhrase<<"\n";
					}
				}

			}

		}
		

/*
//StdOutput<<"!"<<p->Protocol;
		if (p->Protocol!=IPPROTO_UDP) continue;
		int from=p->Udp()->SourcePort;
		int to=p->Udp()->DestinationPort;

StdOutput<<from<<"-"<<to<<"\n";
		if (from!=5060 && to!=5060) continue;

		StdOutput<<"got packet "<<~p<<"bytes\n";

		StdOutput<<p.Dump(&mask)<<"\n";

*/

	}

	return(0);
}
