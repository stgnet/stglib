// stg template library
// tone generator utilitiez
// mostly for telephony uses

#ifndef STGLIB_tone
#define STGLIB_tone

#include "/src/stglib/stcore.h"
#include "/src/stglib/stsine.h"
#include "/src/stglib/stparse.h"

// helpful notification on win32 compilers
#pragma message("using sttone.h")

class StDualTone:public StBase
{
	StSine sine1;
	StSine sine2;
	StSize samples_left;
public:
	StDualTone(int rate,int tone1,int db1,int tone2,int db2,long ms)
	{
		sine1(rate,tone1,db1);
		sine2(rate,tone2,db2);
		samples_left=ms*(rate/1000);
	}
	virtual char _IsStream(void)
	{
		return(1);
	}
	StSize _Read(StByte *data,StSize size)
	{
		if (size&1) --size;

		StSize samples=size/2;
		if (samples>samples_left)
			samples=samples_left;
		samples_left-=samples;
		size=samples*2;

		unsigned short *samp=(unsigned short *)data;

		while (samples--)
		{
			long value=sine1();
			value+=sine2();
			if (value<-32767) value=-32767;
			if (value>32767) value=32767;

			*samp=(unsigned short)value;
			++samp;
		}
		return(size);
	}
};

// play a tone list in format:
// tone{@db}{+tone{@db}}/duration{,repeat}

class StTones:public StBase
{
	StString ToPlay;
	StSize Rate;
public:
	StTones(StSize rate,const char *tones)
	{
		ToPlay<<tones;
		Rate=rate;
	}
	StSize _WriteTo(StBase &Destination)
	{
		while (~ToPlay)
		{
			StParseString tone1;
			StParseAfter<','> rest(tone1);
			StParseAfter<'/'> ms(tone1);
			StParseAfter<'+'> tone2(tone1);
			StParseAfter<'@'> db1(tone1);
			StParseAfter<'@'> db2(tone2);
	
			ToPlay>>tone1;

			int T1,D1,T2,D2,MS;
			tone1>>T1;
			tone2>>T2;
			db1>>D1;
			db2>>D2;
			ms>>MS;
			if (!~db1) D1=-10;
			if (!~db2) D2=-10;

			StDualTone gen(Rate,T1,D1,T2,D2,MS);

			gen>>Destination;

			!ToPlay<<rest;
		}
		return(1);

	}

};

#endif
