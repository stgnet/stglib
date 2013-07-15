//#define STBASE_DEBUG
//#define STBASE_DEBUG_FILE "/tmp/debug.log"

#include "/src/stglib/stthread.h"

#include "/src/stglib/stagi.h"
#include "/src/stglib/sthttp.h"
#include "/src/stglib/stfilter.h"

#include "/src/stglib/stmath.h"

StAverage<float> average_dbm(3*10); // 3 second average (100ms chunks)


float dBm(short *sample,int count)
{
	float Vp;
	float Square;
	float Total;
	float Average;
	float Vrms;
	float Vmilliwatt;
	float dB;

	Total=0;
	int i=count;
	while (i--)
	{
		Vp=(*sample++)/20678.0;
		Square=Vp*Vp;
		Total+=Square;
	}
	Average=Total/(float)count;
	Vrms=sqrtf(Average);
	Vmilliwatt=sqrtf(0.6);
	dB=20*log10(Vrms/Vmilliwatt);
	return(dB);
}

class AudioHandler:public StThread
{
	// asterisk pushes 100 ms at a time, 
	// but leave some extra room in case of buffering
	StByte data[800*2+200];

public:
	AudioHandler()
	{
		_ThreadStart();
	}
	void Write(StByte *data,StSize size)
	{
		average_dbm+=dBm((short *)data,size/2);
	}
	void _Thread(void)
	{
		while (!_ThreadShutdownRequested)
		{
			int got=read(3,data,sizeof(data));
			if (got<2)
				break;
//			if (RecActive)
				Write(data,got);
		}
	}
};



short test_data[]=
{
	8828,
	20860,
	20860,
	8828,
	-8828,
	-20860,
	-20860,
	-8828,
};

char *cutdot(char *s)
{
	while (*s)
	{
		if (*s=='.')
		{
			*s=0;
			return(++s);
		}
		++s;
	}
	return(0);
}


int main(void)
{
/*
	float db=dBm(test_data,8);

	printf("%f\n",db);
	exit(0);
*/

	StAgi agi;

	agi.Answer();
	agi.Play("silence/1");
	agi.Play("welcome");
	agi.Play("call");
	agi.Play("meter");

	AudioHandler ah;

		agi.Play("silence/5");

	if (agi("agi_enhanced")=="0.0")
	{
		agi.Play("an-error-has-occured");
//		agi.Say("The gateway interface is not in enhanced mode.");
		agi.Hangup();
		exit(0);
	}




//StString value;
//	value=agi.Get("Enter a string please");
//	StString msg;
//	msg<<"You entered "<<value<<".  Thank you!";
//	agi.Say(msg);
//	agi.Hangup();

//	agi.Play("/tmp/record");

//	agi.Tones("350+440/4000");
	
//	RecActive=0;


	while (1)
	{
		char buf[32];
		sprintf(buf,"%f",(float)average_dbm);
		char *d=cutdot(buf);

		if (d)
		{
			d[1]=0;
			agi.SayNumber(buf);
			agi.Play("point");
			agi.SayNumber(d);
		}
		else
			agi.SayNumber(buf);

		agi.Play("letters/d");
		agi.Play("letters/b");
		agi.Play("letters/m");


		agi.Play("silence/5");
	}

	agi.Hangup();
	exit(0);

}
