#include "/src/stglib/stglib.h"
#include "/src/stglib/stsockif.h"

#include <syslog.h>

#define RETRIES 16


//#define DEBUG(x) {StString temp; temp<<x; syslog(LOG_NOTICE,"%s",(const char*)temp);}
#define DEBUG(x)

#define NOTICE(x) {StString temp; temp<<x; syslog(LOG_NOTICE,"%s",(const char*)temp);}
//#define NOTICE(x)

#define ERROR(x) {StString temp; temp<<x; syslog(LOG_NOTICE,"%s",(const char*)temp);}
//#define ERROR(x)

//////////////////////////////////////////////////////////// TFTP PACKET
#define OP_RRQ 1
#define OP_WRQ 2
#define OP_DATA 3
#define OP_ACK 4
#define OP_ERROR 5

#define EC_UNKNOWN 0
#define EC_FILE_NOT_FOUND 1
#define EC_ACCESS_VIOLATION 2
#define EC_DISK_FULL 3
#define EC_ILLEGAL_OPERATION 4
#define EC_UNKNOWN_TRANSFER 5
#define EC_FILE_EXISTS 6
#define EC_BAD_USER 7

class TFTPpacket:public StBuffer
{
public:
	int OpCode;
	int Block;

	TFTPpacket()
	{
	}

	// preset packet with values
	void Initialize(int opcode)
	{
		!(*this);
		(*this)(0)=(opcode>>8)&0xFF;
		(*this)(1)=(opcode)&0xFF;
	}

	void Initialize(int opcode,int block)
	{
		!(*this);
		(*this)(0)=(opcode>>8)&0xFF;
		(*this)(1)=(opcode)&0xFF;
		(*this)(2)=(block>>8)&0xFF;
		(*this)(3)=(block)&0xFF;
	}

	// make sure StBase knows to wipe the packet
	// buffer every time it's read
	inline virtual char _IsPacket(void)
	{
		return(1);
	}

	// after data has been read into this packet,
	// pull out some useful values
	void _ReadStop(StBase &Destination)
	{
		// data has been udpated

		OpCode=((*this)(0)<<8)|(*this)(1);
		Block=((*this)(2)<<8)|(*this)(3);
	}

	// get the file name from the packet in a safe way
	StString GetFileName(void)
	{
		StString FileName;

		StSize index=2;
		while (index<~(*this) && (*this)(index))
			FileName+=(*this)(index++);

		return(FileName);
	}
	// get the error message from the packet in a safe way
	StString GetErrorMsg(void)
	{
		StString msg;

		StSize index=4;
		while (index<~(*this) && (*this)(index))
			msg+=(*this)(index++);

		return(msg);
	}
};

//////////////////////////////////////////////////////////// LIST OF FILES
// list of files to scan for match to TFTP request

#define TYPE_NORMAL 0
#define TYPE_PHP 1

// this is NOT threadsafe!
StString MAC;

// file entry with full path and name pattern matching
class File
{
public:
	StString Path;	// full path to file
	StString Match;	// pattern to match
	int Type;	// type of file (pass verbatim or process)

	// match * as wildcard character
	bool operator==(const char *s)
	{
		!MAC;

		if (!s || !*s)
			return(false);

		const char *p=(const char*)Match;

		while (*s && *p)
		{
			if (*p==*s)
			{
				++p;
				++s;
				continue;
			}
			if (*p=='#' && p[1])
			{
				++p;
				++s;
				while (*s && *s!=*p)
					++s;
				continue;
			}
			if (*p=='*')
			{
				MAC+=*s;
				++p;
				++s;
				continue;
			}
			return(false);
		}
		if (*s!=*p)
			return(false);
		return(true);
	}

	bool operator>(File &other)
	{
		return(~Match<~other.Match);
	}
 
};

// list of files available
StBox<File> Files;

// utility to convert pattern
StString MacToWild(const char *s)
{
	StString temp;
	while (*s)
	{
		// match _MACADDRESS_ to any 12 characters
		if (
			s[0]=='_' && s[1]=='M' && s[2]=='A' && s[3]=='C' &&
			s[4]=='A' && s[5]=='D' && s[6]=='D' && s[7]=='R' &&
			s[8]=='E' && s[9]=='S' && s[10]=='S' && s[11]=='_')
		{
			s+=12;
			temp<<"************";
			continue;
		}
		// match _ADDR_ to any 6 characters
		if (
			s[0]=='_' && s[1]=='A' && s[2]=='D' &&
			s[3]=='D' && s[4]=='R' && s[5]=='_')
		{
			s+=6;
			temp<<"******";
			continue;
		}
		// match _X_ to any number of characters
		if (s[0]=='_' && s[1]=='X' && s[2]=='_')
		{
			s+=3;
			temp<<"#";
			continue;
		}
		temp+=*s;
		++s;
	}
	return(temp);
}

// scan directory recursively into files list
void LoadDirectory(const char *path=0)
{
	if (!path)
		path="/axeconfig";

	// get contents of the directory at path
	StDirectory<StDirectoryEntry> list(path);

	// scan the directory for ignore flag
	StBoxRef<StDirectoryEntry> file(list);
	while (++file)
	{
		if (file->_Name=="ignore.axe")
		{
			NOTICE("Ignoring directory "<<path);
			return;
		}
	}

	// scan the list and process files
	StBoxRef<StDirectoryEntry> scan(list);
	while (++scan)
	{
		// construct a full path to the file
		StDirectoryPath fullpath(path,scan->_Name);

		// recurse through directories
		if (scan->_IsDirectory)
		{
			LoadDirectory(fullpath);
			continue;
		}

		// convert patterns in the file name into the file list
		StString NewFile(MacToWild(scan->_Name));

		StParseString JustFile;
		StParseTermLast<'.'> Extension(JustFile);
		JustFile<<NewFile;

		if (Extension=="php")
			!NewFile<<JustFile; // remove .php from file

		// look it up in table first
		if (Files((const char*)NewFile))
		{
			NOTICE("Detected duplicate file "<<NewFile);
			continue;
		}

		// add it to the list
		File &add=+Files;

		add.Path<<fullpath;
		add.Match<<NewFile;
		add.Type=TYPE_NORMAL;

		if (Extension=="php")
			add.Type=TYPE_PHP;
	}

}

// Assuming that the exact file path wasn't found
// and that the requesting device may have erronously specified a path,
// return a version stripped of any path component
StString FixName(const char *s)
{
	// scan string and drop pointer after last slash
	const char *p=s;
	while (*s)
		if (*s=='/')
			p=++s;
		else
			++s;

	StString rv(p);
	return(rv);
}

//////////////////////////////////////////////////////////// TFTP SESSION
// handle transfer of file data

StBox<class tftp_session> sessions;

class tftp_session:public StCoop
{
	int retries;
	StSize block_size;
	StFile file;
	TFTPpacket data;
public:
	unsigned short block;
	StString FileName;
	StSockUdp *psock;
	StIpAddr client;

	void TransmitBlock(unsigned short wanted)
	{
		if (wanted!=block)
		{
			if (wanted!=block+1)
			{
				ERROR("Detected invalid sequence in TransmitBlock at "<<block);
				throw("invalid block sequence");
			}

			block=wanted;

			data.Initialize(OP_DATA,block);

			// this is ugly, but gets the job done
			{
				StByte buf[512];
				block_size=file._Read(buf,512);
				data._Write(buf,block_size);
			}
		}

		DEBUG("Sending block "<<block<<" size="<<block_size<<" to "<<psock->_PacketAddress);

		(*psock)<<data;
		if (psock->_Error)
		{
			ERROR("write socket failed: "<<psock->_GetErrorString());
			return;
		}
	}

	tftp_session(StIpAddr replyto,StString filename,StIpAddr local,StSockUdp *ps)
	{
		psock=ps;
		client=replyto;

		sessions+=this;

		DEBUG("SESSION: "<<filename);

		// bind to the local ip address (but not the port!)
//		StIpAddr local_ip(local,0);
//		socket._SockBind(local_ip);

		block=0;
		psock->_PacketAddress=client;
		FileName=filename;

		// if there is a ./ at the start of the filename, get rid of it
		if (FileName(0)=='.' && FileName(1)=='/')
		{
			StByte buf[2];
			FileName._Remove(buf,2);
		}
		if (FileName(0)=='/')
		{
			StByte buf[1];
			FileName._Remove(buf,1);
		}

		File *entry=Files((const char *)FileName);
		if (!entry)
			entry=Files(FixName(FileName));
		if (!entry)
		{
			NOTICE("########### FILE NOT FOUND "<<FileName);

			TFTPpacket data;
			
			data.Initialize(OP_ERROR,EC_FILE_NOT_FOUND);

			data<<"File does not exist";
			data+=0;

			(*psock)<<data;
			if (psock->_Error)
				ERROR("1write socket failed: "<<psock->_GetErrorString());

			++_CoopComplete;
			return;
		}

		StString path;
		path<<entry->Path;

		StFileOpenMethod method=StFileOpenReadOnlyExisting;
		if (entry->Type==TYPE_PHP)
		{
			if (!~MAC)
				MAC<<"_MACADDRESS_";

			!path<<"php "<<entry->Path<<" "<<FileName<<" "<<MAC<<" "<<local._IpAddr()<<" "<<psock->_PacketAddress._IpAddr();

			NOTICE("===> "<<path);

			method=StFileOpenAppOutput;
		}

		if (file._FileOpen(path,method))
		{
			ERROR("##### ERROR open file "<<path<<": "<<file._GetErrorString());

			TFTPpacket data;
			data.Initialize(OP_ERROR,EC_FILE_NOT_FOUND);

			data<<"File open error";
			data+=0;

			(*psock)<<data;
			if (psock->_Error)
				ERROR("2write socket failed: "<<psock->_GetErrorString());

			++_CoopComplete;
			return;
		}

		DEBUG("Sending path "<<entry->Path);

		retries=RETRIES;
		TransmitBlock(1);
	}
	virtual ~tftp_session()
	{
		// remove from box first
		NOTICE("Deleting session for "<<client);
		sessions-=this;
//		DEBUG("Session deleted");
	}

	void _Poll(int Tick)
	{
		if (!Tick)
			return;

		if (!retries)
		{
			NOTICE(client<<" SESSION TIMED OUT ON RETRIES "<<FileName);
			++_CoopComplete;
			return;
		}

		--retries;
		if ((retries&3)==0)
		{
			// send retry only every 4 seconds
			NOTICE(client<<" SESSION RESENDING BLOCK "<<block<<" ON RETRY "<<retries<<" OF "<<FileName);
			psock->_PacketAddress=client;
			TransmitBlock(block);
		}
	}
	void Cancel(void)
	{
			NOTICE(client<<" SESSION CANCELLED BY ERROR "<<FileName);
			++_CoopComplete;
			return;
	}
	void Ack(TFTPpacket &reply)
	{
		if (reply.Block!=block)
		{
			if (reply.Block+1!=block)
				NOTICE(client<<" Received ACK for "<<reply.Block<<" but sent "<<block);

			// IMPORTANT: to avoid SAS, do NOT retransmit data packet!
			return;
		}
		DEBUG("Received ACK for "<<reply.Block);
		if (block_size<512)
		{
			// that was the last block
			NOTICE(client<<" SESSION COMPLETED ON ACK OF LAST BLOCK "<<FileName);
			++_CoopComplete;
			return;
		}
		retries=RETRIES;
		psock->_PacketAddress=client;
		TransmitBlock(block+1);
	}
	void BOGUS_Poll(int Tick)
	{
//		psock->_SetTimeoutSec(0);
		TFTPpacket reply;
		reply<<(*psock);
		if (psock->_Error)
		{
			if (psock->_Error!=StErr_Timeout)
			{
				ERROR("read socket failed: "<<psock->_GetErrorString());
				++_CoopComplete;
				return;
			}

			if (!Tick)	
				return;

			if (!retries)
			{
				DEBUG("SESSION TIMED OUT ON RETRIES");
				++_CoopComplete;
				return;
			}

			--retries;
			DEBUG("SESSION RESENDING BLOCK ON TIMEOUT");
			TransmitBlock(block);
			return;
		}

		if (reply.OpCode==OP_ERROR)
		{
			ERROR("#### ERROR REPORTED #"<<reply.Block<<" = "<<reply.GetErrorMsg());

			++_CoopComplete;
			return;
		}

		if (reply.OpCode==OP_ACK)
		{
			DEBUG("Received ACK for "<<reply.Block);
			if (reply.Block!=block)
			{
				DEBUG("Received ACK for "<<reply.Block<<" but sent "<<block);

				// IMPORTANT: to avoid SAS, do NOT retransmit data packet!
				return;
			}
			DEBUG("Received ACK for "<<reply.Block);
			if (block_size<512)
			{
				// that was the last block
				DEBUG("SESSION COMPLETED ON ACK OF LAST BLOCK");
				++_CoopComplete;
				return;
			}
			retries=RETRIES;
			TransmitBlock(block+1);
			return;
		}

		// somethng else was returned?
		DEBUG("#### SESSION received unexpected OPCODE "<<reply.OpCode);
	}
};


//////////////////////////////////////////////////////////// TFTP SERVER
// primary thread answers TFTP requests

class tftp_server:public StCoop
{
	StIpAddr tftp_port;
	StSockUdp socket;
	TFTPpacket packet;

public:
	tftp_server(StIpAddr &answer)
	{
		tftp_port(answer,69);
		socket(tftp_port);

		NOTICE("Listening to "<<tftp_port);
	}
	virtual ~tftp_server()
	{
	}

	void _Poll(int Tick)
	{
/*
		if (Tick)
		{
			// dump the session list
			StBoxRef<class tftp_session> scan(sessions);
			while (++scan)
				NOTICE("**** "<<scan->client<<" = "<<scan->FileName<<" "<<scan->block);
		}

*/
		StCoop *completed=StCoops._Completed();
		if (completed)
		{
			delete completed;
		}

		socket._SetTimeoutMSec(10);
		packet<<socket;

		if (socket._Error)
		{
			if (socket._Error!=StErr_Timeout)
				ERROR("read failed: "<<socket._GetErrorString());
			return;
		}

		switch (packet.OpCode)
		{
		case OP_RRQ:
			NOTICE(socket._PacketAddress<<" RRQ "<<packet.GetFileName());
			{
				int found=0;
				StBoxRef<class tftp_session> scan(sessions);
				while (++scan)
				{
					if (scan->client==socket._PacketAddress)
					{
						NOTICE("#### "<<socket._PacketAddress<<" SKIPPING RRQ "<<packet.GetFileName()<<" FROM IP "<<scan->client<<" SERVING "<<scan->FileName);
						++found;
//						NOTICE("#### "<<socket._PacketAddress<<" ABANDONING SESSION "<<scan->FileName);
//						delete scan;
					}
				}
				if (!found)
					new tftp_session(socket._PacketAddress,packet.GetFileName(),tftp_port,&socket);
			}
			break;

		case OP_WRQ:
			NOTICE(socket._PacketAddress<<" WRQ "<<packet.GetFileName());
			{
				TFTPpacket answer;
				answer.Initialize(OP_ERROR,EC_DISK_FULL);
				answer<<"Write not supported";
				answer+=0;
				socket<<answer;
			}
			break;

		case OP_ACK:
			// scan the list of current sessions, pass ack to corerct one
			{
				int acked=0;
				StBoxRef<class tftp_session> scan(sessions);
				while (++scan)
				{
					if (scan->client==socket._PacketAddress)
					{
						scan->Ack(packet);
						acked=1;
					}
				}
				if (!acked)
					ERROR(socket._PacketAddress<<" ACK UNEXPECTED "<<packet.Block);
			}
			break;

		case OP_ERROR:
			NOTICE(socket._PacketAddress<<" REPORTED ERROR: "<<packet.GetErrorMsg());
			{
				StBoxRef<class tftp_session> scan(sessions);
				while (++scan)
				{
					if (scan->client==socket._PacketAddress)
					{
						scan->Cancel();
					}
				}
			}
			break;

		default:
			NOTICE(socket._PacketAddress<<" INVALID OPCODE "<<packet.OpCode);
			break;
		}
	}
};

//////////////////////////////////////////////////////////// FTP
class ftp_session:public StCoop
{
	StSockTcp *psock;
	StFilterTextLine line;
public:
	ftp_session(StSockTcp *ps)
	{
		NOTICE("STARTING FTP Session");
		psock=ps;
		line(*psock);
		line<<"220 (axeconfig)\n";
	}
	virtual ~ftp_session()
	{
		NOTICE("Ending FTP Session");
		delete psock;
	}

	void _Poll(int Tick)
	{
		StParseString Param;
		StParseBefore<' '> Command(Param);

		StString cmd;
//		cmd<<line;
		line>>cmd;
		if (psock->_Error)
		{
			NOTICE("Read error "<<psock->_GetErrorString());
			++_CoopComplete;
			return;
		}
		NOTICE("FTP CMD: "<<cmd);

		Param<<cmd;
		if (Command=="USER")
		{
			"331 Please specify the password.">>cmd;
			return;
		}
		"202 Command not implemented.">>cmd;
	}
};

class ftp_server:public StCoop
{
	StIpAddr ftp_port;
	StSockTcp socket;

public:
	ftp_server(StIpAddr &answer)
	{
		ftp_port(answer,21);
//		socket(ftp_port);
		socket._SockBind(ftp_port);
		socket._SockListen();
		if (socket._Error)
			ERROR("socket open error "<<socket._GetErrorString());

		NOTICE("FTP Listening to "<<ftp_port);
	}
	virtual ~ftp_server()
	{
	}

	void _Poll(int Tick)
	{
		socket._SetTimeoutMSec(10);
		StSockTcp *connection=socket._SockAccept();
		if (connection)
		{
			NOTICE("Creating ftp session for "<<connection->GetIpAddress());
			(*connection)<<"Started\r\n";
			new ftp_session(connection);
		}
	}
};


//////////////////////////////////////////////////////////// MAIN
int main(void)
{
	NOTICE("starting up");

	close(0);
	close(1);
	close(2);

	LoadDirectory();

	Files._BoxSort();

/*
	StBoxRef<File> scan(Files);
	while (++scan)
	{
		StdOutput<<scan->Path<<" = "<<scan->Match<<"\n";
	}
*/

	StSockIfs interfaces;
	if (interfaces._Error)
	{
		StdOutput<<"ERROR Interfaces: "<<interfaces._GetErrorString()<<"\n";
		exit(1);
	}

	if (!~interfaces)
	{
		StIpAddr all;
		new tftp_server(all);
		new ftp_server(all);
	}
	else
	{
		StBoxRef<StSockIf> scan(interfaces);
		while (++scan)
		{
			new tftp_server(scan->IpAddress);
//			new ftp_server(scan->IpAddress);
		}
	}

	while (~StCoops) StCoops._Poll();

	return(0);
}

