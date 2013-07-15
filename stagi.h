// StG Template Library
// Asterisk Gateway Interface
// Note the following presumed conditions:
// 1) On start, StdInput is read for a list of name: value pairs
// 2) StdOutput connected to Asterisk where it receives commands
// 3) StdInput connected to Asterisk where it replies to commands
// 4) /tmp/ directory exists and is writeable for temp files, logs
// 5) If called as EAGI, path 3 is audio in 16 bit linear 8khz

#ifndef STGLIB_STAGI
#define STGLIB_STAGI

#pragma message("using stagi.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stdio.h"
#include "/src/stglib/stpair.h"
#include "/src/stglib/ststring.h"
#include "/src/stglib/stfilter.h"
#include "/src/stglib/stparse.h"
#include "/src/stglib/sturl.h"
#include "/src/stglib/sttone.h"

StString SayFile(const char *text);

//#define ALL_DIGITS " 1234567890#*"

class StAgi:public StPair
{
	StString cmd;
	StFilterTextLine line;
	int Code;
public:
#ifdef STAGI_DEBUG
	StFile log;
#endif
	StString Pressed;
	int Digit;

	StAgi()
	{
		line(StdInput);
		StdOutput._FlushAfterWrite=1;
#ifdef STAGI_DEBUG
		log("/tmp/agi.log",StFileOpenAppend);
		log._FlushAfterWrite=1;
		log<<"Started StAgi()\n";
#endif
	
		Code=0;
		Digit=0;

		// load the pairs from stdinput
		while (1)
		{
			StString input;
			line>>input;
			if (!~input)
				break;
#ifdef STAGI_DEBUG
log<<"<="<<input<<"\n";
#endif

			StParseString left;
			StParseAfterTwo<':',' '> right(left);
			input>>left;
			if (~right)
				(*this)(left)<<right;
		}
	}
	int final_reply(const char *s)
	{
		int value=0;
		int count=3;
		while (count)
		{
			if (*s<'0' || *s>'9')
				return(0);
			value*=10;
			value+=*s-'0';
			--count;
		}
		Code=value;
		if (*s=='-')
			return(0);
		return(1);
	}
	void Command(char *c=0)
	{
		!Pressed;
		Digit=0;
		if (c)
			!cmd<<c;
#ifdef STAGI_DEBUG
log<<"=>"<<cmd<<"\n";
#endif
		// send command
		StdOutput<<cmd<<"\n";

		// parse reply
		StString input;
		line>>input;
#ifdef STAGI_DEBUG
log<<"reply="<<input<<"\n";
#endif
			
// reply format:
// ###-text
// text
// ### text
// (first two lines are optional, 2nd may repeat many times)
// if value is 200, the following text is name=value pairs sep'd by space
// often pairs will be result=# endpos=#
// else # is error code, and first line text is explanation text

		StParseString text;
//		StParseFirst<4> number(text);
		StParseBefore<' '> number(text);

		input>>text;
#ifdef STAGI_DEBUG
log<<"<- numb="<<number<<"| text="<<text<<"|\n";
#endif

		if (number=="200")
		{
			StString temp;
			temp<<text;

			while (1)
			{
				// parse text for name=value pairs
				StParseString pair;
				StParseAfter<' '> next(pair);
				temp>>pair;

#ifdef STAGI_DEBUG
log<<"< add: "<<pair<<"\n";
#endif
				_AddEquals(pair);

				if (!~next)
					break;
				!temp<<next;
			}
		}

		while (~input && !final_reply(input))
		{
			line>>input;
#ifdef STAGI_DEBUG
log<<"<="<<input<<"\n";
#endif
		}
	}
	virtual StSize _Write(const StByte *data,StSize size)
	{
		// reset things
		!cmd;
		_ResetError();

		// store command
		StSize wrote=cmd._Write(data,size);

		Command();
		return(1);
	}
	void SayNumber(const char *s)
	{
		!cmd<<"SAY NUMBER "<<s<< " 1234567890#*";
		Command();
	}
	void Play(const char *file,StSize offset=0)
	{
		!cmd<<"STREAM FILE "<<file<<" 1234567890#*";
		if (offset)
			cmd<<" "<<offset;

		Command();
		const char *result=(*this)("result");
		Digit=atoi(result);
		if (Digit)
			Pressed._Write((StByte)Digit);
	}
	void Say(const char *text)
	{
		// ask external function for a file that
		// has been rendered (by some TTS) to wav
		StString file;
		file<<SayFile(text);
		Play(file);
	}
	StString Get(const char *file,int timeout=4000,int digits=0)
	{
		StString prompt;
		prompt<<file;
		if (prompt._Contains(" ")>0)
			!prompt<<SayFile(file);
		!cmd<<"GET DATA "<<prompt<<" "<<timeout;
		if (digits)
			cmd<<" "<<digits;
		Command();
		return((*this)("result"));
	}
	void Answer(void)
	{
		Command("ANSWER");
	}
	void Hangup(void)
	{
		Command("HANGUP");
	}
	void Tones(const char *tones)
	{
		StUrlEscape urlize;
		StString tones2u;
		tones>>urlize>>tones2u;
		StString file;
		StString file_ext;
		file<<"/tmp/tones-"<<tones2u;
		file_ext<<file<<".sln";

		StFile test(file_ext);
		if (test._FileOpen())
		{
			// doesn't already exist
			StFile sln(file_ext);
			StTones generator(8000,tones);
			StBuffer temp;
			generator>>temp;
			temp>>sln;
		}
		Play(file);
	}
	virtual ~StAgi()
	{
		Hangup();
	}
};

#endif 
