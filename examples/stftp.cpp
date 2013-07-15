#include "/src/stglib/stthread.h"
#include "/src/stglib/stdio.h"
#include "/src/stglib/stbase.h"
#include "/src/stglib/stsock.h"
#include "/src/stglib/stsockudp.h"
#include "/src/stglib/sttime.h"

class TFTPpacket:public StBaseData
{
public:
	StByte data[256];

	TFTPpacket():StBaseData(data,256)
	{
		memset(data,0,256);
	}
};

class tftpd:public StThread
{
public:
	tftpd()
	{
		_ThreadStart();
	}

	void _Thread(void)
	{
		StIpAddr tftp(0,69);
		StSockUdp tftpd(tftp);

		while (1)
		{
			TFTPpacket tp;

			tp<tftpd;

			if (tftpd._Error)
			{
				StdOutput<<"read failed: "<<tftpd._GetErrorString()<<"\n";
				sleep(1);
				continue;
			}

			StdOutput<<"Got packet\n";
		}
	}
};

int main(void)
{
	new tftpd();

	while (1)
	{
		StdOutput<<"running...\n";
		sleep(10);
	}

	return(0);
}
