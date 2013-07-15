#include "/src/stglib/stglibmt.h"

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
	TFTPpacket(int opcode)
	{
		(*this)(0)=(opcode>>8)&0xFF;
		(*this)(1)=(opcode)&0xFF;
	}

	TFTPpacket(int opcode,int block)
	{
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
		if (!s || !*s)
			return(false);

		const char *p=(const char*)Match;

//StdOutput<<p<<"="<<s<<"?\n";
		while (*s && *p)
		{
			if (*p==*s)
			{
				++p;
				++s;
				continue;
			}
			if (*p=='*')
			{
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
};

// list of files available
StBox<File> Files;

// utility to convert pattern
StString MacToWild(const char *s)
{
	StString temp;
	while (*s)
	{
		if (
			s[0]=='_' && s[1]=='M' && s[2]=='A' && s[3]=='C' &&
			s[4]=='A' && s[5]=='D' && s[6]=='D' && s[7]=='R' &&
			s[8]=='E' && s[9]=='S' && s[10]=='S' && s[11]=='_')
		{
			s+=12;
			temp<<"************";
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
//			StdOutput<<"ALREADY LOADED: "<<NewFile<<"\n";
			continue;
		}

		// add it to the list
		File &add=+Files;

		add.Path<<fullpath;
		add.Match<<NewFile;
		add.Type=TYPE_NORMAL;
//StdOutput<<"Ext="<<Extension<<"|\n";

		if (Extension=="php")
			add.Type=TYPE_PHP;

//		StdOutput<<"Loaded: "<<add.Match;
//		if (add.Type==TYPE_PHP)
//			StdOutput<<" (PHP)\n";
//		else
//			StdOutput<<"\n";
	}

}

//////////////////////////////////////////////////////////// TFTP SESSION
// handle transfer of file data

class tftp_session:public StThread
{
	StString FileName;
public:
	StSockUdp socket;

	tftp_session(StIpAddr client,StString filename)
	{
		// 3 sec timeout * 10 retries = 30 sec session timeout
		socket._SetTimeoutSec(3);
		socket._PacketAddress=client;
		FileName=filename;

		// if there is a ./ at the start of the filename, get rid of it
		if (FileName(0)=='.' && FileName(1)=='/')
		{
			StByte buf[2];
			FileName._Remove(buf,2);
		}
		_ThreadStart();
	}
	virtual ~tftp_session()
	{
//		StdOutput<<"Thread ending\n";
	}

	void _Thread(void)
	{
		File *entry=Files((const char *)FileName);
		if (!entry)
		{
			StdOutput<<"##########> "<<FileName<<": NOT FOUND\n";

			TFTPpacket data(OP_ERROR,EC_FILE_NOT_FOUND);

			data<<"File does not exist";
			data+=0;

			socket<<data;
			if (socket._Error)
				StdOutput<<"1write socket failed: "<<socket._GetErrorString()<<"\n";
			return;
		}

		StString path;
		path<<entry->Path;

		StFileOpenMethod method=StFileOpenReadOnlyExisting;
		if (entry->Type==TYPE_PHP)
		{

			!path<<"php "<<entry->Path<<" "<<FileName;
StdOutput<<"RUNNING PHP: "<<path<<"\n";
			method=StFileOpenAppOutput;
		}

		StFile file(path,method);

		if (file._FileOpen())
		{
			StdOutput<<FileName<<": Error opening file:"<<file._GetErrorString()<<"\n";

			TFTPpacket data(OP_ERROR,EC_FILE_NOT_FOUND);

			data<<"File open error";
			data+=0;

			socket<<data;
			if (socket._Error)
				StdOutput<<"2write socket failed: "<<socket._GetErrorString()<<"\n";
			return;
		}

		StdOutput<<"Sending path "<<entry->Path<<"\n";

		unsigned short block=0;

		// transmit file as series of 512 byte blocks
		while (1)
		{
			++block;
			TFTPpacket data(OP_DATA,block);
			StSize block_size;

			// this is ugly, but gets the job done
			{
				StByte buf[512];
				block_size=file._Read(buf,512);
				data._Write(buf,block_size);
//if (entry->Type==TYPE_PHP)
//	StdOutput._Write(buf,block_size);

			}

//StdOutput<<"Block "<<block<<" bytes="<<block_size<<"\n";

			int retries=10;
			while (--retries)
			{
				socket<<data;
				if (socket._Error)
				{
					StdOutput<<"3write socket failed: "<<socket._GetErrorString()<<"\n";
					return;
				}

			wait_for_reply:
				socket._SetTimeoutSec(3);
				TFTPpacket reply;
				reply<<socket;
				if (socket._Error)
				{
					if (socket._Error!=StErr_Timeout)
						StdOutput<<"read socket failed: "<<socket._GetErrorString()<<"\n";
//StdOutput<<socket._PacketAddress<<" Block="<<block<<" Retries="<<retries<<"\n";
					continue;
				}

				if (reply.OpCode==OP_ERROR)
				{
					StdOutput<<socket._PacketAddress<<" ERROR "<<reply.GetErrorMsg()<<"\n";
					goto wait_for_reply;
				}

				if (reply.OpCode==OP_ACK)
				{
					if (reply.Block==block)
					{
//						StdOutput<<"Received ACK\n";
						break;
					}
					StdOutput<<"Received ACK for "<<reply.Block<<" but sent "<<block<<"\n";
					// IMPORTANT: to avoid SAS, do NOT retransmit data packet!
					goto wait_for_reply;
				}

				// somethng else was returned?
				StdOutput<<"SESSION received OPCODE "<<reply.OpCode<<"\n";
				return;
			}
			if (!retries)
				return;
		
			// was that the last block?
			if (block_size<512)
				break;
		}
		StdOutput<<"Session Complete\n";
	}
};


//////////////////////////////////////////////////////////// TFTP SERVER
// primary thread answers TFTP requests

class tftp_server:public StThread
{
	StThreadBox<tftp_session> sessions;

public:
	tftp_server()
	{
		_ThreadStart();
	}
	virtual ~tftp_server()
	{
	}

	void _Thread(void)
	{
		StIpAddr tftp_port(0,69);
		StSockUdp socket(tftp_port);
		TFTPpacket packet;

		while (!_ThreadShutdownRequested)
		{
			tftp_session *completed=sessions._ThreadCompleted();
			if (completed)
			{
				StdOutput<<"Deleting thread for "<<completed->socket._PacketAddress<<"\n";
				sessions-=completed;
				delete completed;
				continue;
			}

			socket._SetTimeoutSec(3);
			packet<<socket;

			if (socket._Error)
			{
				if (socket._Error!=StErr_Timeout)
					StdOutput<<"read failed: "<<socket._GetErrorString()<<"\n";
//				sleep(1);
				continue;
			}

			switch (packet.OpCode)
			{
			case OP_RRQ:
				StdOutput<<socket._PacketAddress<<" RRQ "<<packet.GetFileName()<<"\n";
				// %%%% check existing threads for this address first!
				sessions+=new tftp_session(socket._PacketAddress,packet.GetFileName());
				break;

			case OP_WRQ:
				StdOutput<<socket._PacketAddress<<" WRQ "<<packet.GetFileName()<<"\n";
				{
					TFTPpacket answer(OP_ERROR,EC_DISK_FULL);
					answer<<"Write not supported";
					answer+=0;
					socket<<answer;
				}
				break;

			case OP_ERROR:
				StdOutput<<socket._PacketAddress<<" ERROR "<<packet.GetErrorMsg()<<"\n";
				break;

			default:
				StdOutput<<socket._PacketAddress<<" INVALID OPCODE "<<packet.OpCode<<"\n";
				break;
			}
		}
	}
};

//////////////////////////////////////////////////////////// MAIN
int main(void)
{
	LoadDirectory();

	new tftp_server();

	while (~StThreads)
	{
		if (~StThreads>1)
			StdOutput<<"Threads: "<<~StThreads<<"\n";
		sleep(10);
		LoadDirectory();
	}

	return(0);
}

