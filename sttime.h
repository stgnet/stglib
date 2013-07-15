// STGLIB/sttime.h
// Copyright 2002 by StG Net

// math functions


#include "/src/stglib/stcore.h"

#ifndef STGLIB_STTIME
#define STGLIB_STTIME

#pragma message("using sttime.h")

// Make sure we include normal time definitions
#ifndef _INC_TIME
#include <time.h>
#endif

class StTimer
{
	time_t start_time;
	StSize wait_seconds;

public:
	StTimer(void)
	{
		// timer not set, just available
		start_time=0;
		wait_seconds=0;
	}
	StTimer(StSize seconds)
	{
		start_time=time(0);
		wait_seconds=seconds;
	}
	void operator()(StSize seconds)
	{
		start_time=time(0);
		wait_seconds=seconds;
	}

	StSize SecondsLeft(void)
	{
		time_t now=time(0);
		StSize elapsed=(StSize)(now-start_time);
		if (elapsed<wait_seconds)
			return(elapsed-wait_seconds);
		return(0);
	}
	void operator!(void)
	{
		start_time=time(0);
	}

	inline StSize operator~(void)
	{
		return(SecondsLeft());
	}

};

		//UNIX maintains date and time as seconds past Jan 1, 1970.  Convert to Excel use with
		//= DATE(1970, 1, 1) + (A1/24/60/60) 
		//=(C1/86400)+DATE(1970,1,1) 
		//
		//Excel stores a date as the number of days since Dec 31, 1899. So you need
		//to convert the Unix time from seconds to days, then add the date value for
		//the Unix base date. Assuming the Unix date is in A1, this should work:
		//  =(A1/86400+DATE(1970,1,1) 

		// Reverse:
		// subtract 25569.00
		// multiply by 86400


class StDate
{
	time_t time;
	StString buf;

public:
	StDate(time_t value)
	{
		time=value;
	}

	StDate(double excel_format)
	{
		time=(long)((excel_format-25569.0)*86400.0);
	}

	StString &YYYYMMDD(void)
	{
		struct tm *tm=gmtime(&time);

		!buf;

		buf._Dec(1900+tm->tm_year,4);
		buf<<"-";
		buf._Dec(tm->tm_mon+1,2);
		buf<<"-";
		buf._Dec(tm->tm_mday,2);

		return(buf);
	}

};

class StTime
{
	time_t _Time;
	StString buf;

public:
	StTime()
	{
		_Time=time(0);
	}

	StSize operator~(void)
	{
		time_t _Now=time(0);
		return((StSize)(_Now-_Time));
	}
	StSize _Since(void)
	{
		time_t _Now=time(0);
		time_t _Last=_Time;
		_Time=_Now;
		return((StSize)(_Now-_Last));
	}

	StTime& operator!()
	{
		_Time=time(0);
		return(*this);
	}
};
#endif
