//#define STBASE_DEBUG
//#define STBASE_DEBUG_FILE "/tmp/debug.log"

#include "/src/stglib/stthread.h"

#include "/src/stglib/stagi.h"
#include "/src/stglib/sthttp.h"
#include "/src/stglib/stfilter.h"

//#include "/src/stglib/stmath.h"


//StString RecordFile;

StFile RecFile;

StSize RecSize=0;	// size is # of samples, not filesize!
StSize RecCurr=0;	// current position, not max file size
StSize RecPos=0;	// position (samples) to jump to
StSize RecActive=0;	// samples written to file when !0
			// 2=seek to recpos

#define WIND_RATE 32
StArray<short> WindData;


class AudioHandler:public StThread
{
	// asterisk pushes 100 ms at a time, 
	// but leave some extra room in case of buffering
	StByte data[800*2+200];
//	StFile ahdlog;

public:
	AudioHandler()
	{
//		ahdlog("/tmp/ahd.log",StFileOpenAppend);
//		ahdlog._FlushAfterWrite=1;
		_ThreadStart();
	}
	void Write(StByte *data,StSize size)
	{
		if (RecActive==2)
		{
			// if asked, seek to new position first
			RecFile._FileSeek(RecPos*2);
			RecSize=RecPos;
			RecActive=1;
		}
		RecFile._Write(data,size);
//if (RecFile._Error)
//RecFile._ShowError(ahdlog);
//else
//ahdlog<<"Wrote "<<size<<" bytes at "<<RecCurr<<"\n";

		// update wind data
		short *sample=(short*)data;
		StSize samples=size/2;
		StSize pos=RecCurr;
		while (samples--)
			WindData[pos++/WIND_RATE]=*sample++;

		// update current position
		RecCurr+=size/2;
		if (RecSize<RecCurr)
			RecSize=RecCurr;
	}
	void _Thread(void)
	{
		while (!_ThreadShutdownRequested)
		{
			int got=read(3,data,sizeof(data));
			if (got<2)
				break;
			if (RecActive)
				Write(data,got);
		}
	}
};

// the agi.Say() function calls this to convert text to wav file
StString SayFile(const char *text)
{
	StUrlEscape urlize;
	StString text2p;
	text>>urlize>>text2p;
	StString url;
	url<<"say.stg.net/"<<text2p;
	StString file;
	StString file_ext;
	file<<"/tmp/say-"<<text2p;
	file_ext<<file<<".wav";

	// does the file already exist?
	StFile test(file_ext);
	if (test._FileOpen())
	{
		StFile wav(file_ext,StFileOpenOverwrite);
		StBuffer data;
		StHttp http(url);
		if (http._Error)
			return("uh-oh1");
		http>>data;
		if (http._Error)
			return("uh-oh1");
		if (!~data)
			return("uh-oh2");
		data>>wav;
		if (wav._Error)
			return("uh-oh2");
	}
	return(file);
}

	StString FileName;
	StString File_Ext;
	StAgi agi;

	StString WindName;
	StString Wind_Ext;

void Wind(StSize target)
{
	// construct a file with bits of audio between RecCurr and target
	StFile wind(Wind_Ext,StFileOpenOverwrite);

agi.log<<"Wind to target "<<target<<" from "<<RecCurr<<"\n";

	long dist=target-RecCurr;
	if (!dist)
	{
		agi.Digit=0;
		return;
	}
	long length=8000+labs(dist)>>5; // 1 sec+1/32 of distance
	if (length>labs(dist))
		length=labs(dist);

	long offset=dist/length;

	while (length--)
	{
		RecCurr+=offset;

		short sample=WindData[RecCurr/WIND_RATE];
		
		wind._Write((StByte*)&sample,2);
	}
	wind._FileClose();

	// then play it, and update RecCurr
	agi.Play(WindName);

	// make sure to delete file when done playing it.
//	!wind;
	RecCurr=target;
}
void Play(void)
{
	// play the audio stream from current position

	agi.Play(FileName,RecCurr);

	long newpos;
	agi("endpos")>>newpos;
	if (newpos)
	{
agi.log<<"Setting position to "<<newpos<<"\n";
		RecCurr=newpos;
		if (RecCurr>RecSize)
			RecCurr=RecSize;
	}
}

int main(void)
{

	agi.Answer();
	agi.Play("welcome");

	if (agi("agi_enhanced")=="0.0")
	{
		agi.Play("an-error-has-occured");
		agi.Say("The gateway interface is not in enhanced mode.");
		agi.Hangup();
		exit(0);
	}
	AudioHandler ah;

	FileName<<"/tmp/"<<"rec-"<<agi("agi_uniqueid");
	File_Ext<<FileName<<".sln";
	WindName<<"/tmp/"<<"wnd-"<<agi("agi_uniqueid");
	Wind_Ext<<WindName<<".sln";

	RecFile(File_Ext,StFileOpenOverwrite);
	if (RecFile._Error)
	{
		agi.Play("an-error-has-occured");
		agi.Say("Cant write audio file.");
		agi.Hangup();
		exit(0);
	}

//	RecActive=1;

//	agi.Say("This is a test");

//StString value;
//	value=agi.Get("Enter a string please");
//	StString msg;
//	msg<<"You entered "<<value<<".  Thank you!";
//	agi.Say(msg);
//	agi.Hangup();

//	agi.Play("/tmp/record");

//	agi.Tones("350+440/4000");
	
//	RecActive=0;



	// keys
	// 1 = play  - rewind 1 sec, play
	// 2 = record - prompt to press 2 again to ovewrite, or 6 to append
	// 3 = rewind - rewind 3 sec, play
	// 4 = pause (tick tick)
	// 5 = save file 
	// 6 = end of file, then pause
	// 7 = forward 3 sec, play
	// 8 = beginning of file
	// 9 = abort
	// * = list options

	agi.Say("Press 2 to begin recording, or star for a list of options.");

	StString temp;
	StSize target;
	while (1)
	{
		RecActive=0;
		switch (agi.Digit)
		{
		case 0: 
			agi.Tones("440+523/16,0/2000");
			break;

		case '1': // rewind 1 sec, play
			target=RecCurr;
			if (target<8000)
				target=0;
			else
				target-=8000;
			Wind(target);
			if (!agi.Digit)
				Play();
			break;

		case '2': // record
			if (RecCurr<RecSize)
			{
				agi.Say("Press 6 to go to end before recording.");
				break;
			}
			agi.Tones("880/300");
			RecActive=1;
			
			while (1)
			{
				agi.Tones("0/3000");
				if (agi.Digit)
					break;
			}
			break;

		case '3': // rewind 3 sec, play
			target=RecCurr;
			if (target<24000)
				target=0;
			else
				target-=24000;
			Wind(target);
			if (!agi.Digit)
				Play();
			break;

		case '4': // pause
			agi.Say("Paused.");
			break;

		case '5': // save
			agi.Say("Saving.");
			agi.Hangup();
			exit(0);

		case '6': // eof
			target=RecSize;
			Wind(target);
			break;

		case '7': // fwd 3, play
			target=RecCurr+24000;
			if (target>RecSize)
				target=RecSize;
			Wind(target);
			if (!agi.Digit)
				Play();
			break;

		case '8': // bof, play
			target=0;
			Wind(target);
			if (!agi.Digit)
				Play();
			break;

		case '9': // abort
			agi.Say("Recording aborted.");
			agi.Say("Goodbye.");
			agi.Hangup();
			exit(0);
			
		case '#':
			agi.Play(FileName);
			break;

		case '*':
		case '0':
			temp<<"You pressed "<<agi.Pressed;
			agi.Say(temp);
			break;
		default:
			temp<<"Unknown code "<<agi("result");
			agi.Say(temp);
			break;
		}
	}

	return(0);
}
