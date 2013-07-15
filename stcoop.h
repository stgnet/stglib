// STGLIB/StCoop.h
// Copyright 1999,2002 by StG Net

// co-operative multitasking class (alternative to stthread)

// to use:
// 1) define classes using base StCoop that define _Poll(tick) and do some function
// 2) instatiate class at least once
// 3) while (~StCoops) StCoops._Poll();
// all _Poll functions will be executed round-robin
// no _Poll function may block or all do
// tick is non-zero every time second has elapsed since last polled

#ifndef STGLIB_StCoop
#define STGLIB_StCoop

#pragma message("using StCoop.h")



//********************************************* StCoops class (master control)

#include "/src/stglib/stbox.h"

// class prototype for object going in box
class StCoop;

// box to contain a list of the threads in existance
class StCoopBox:public StBox<StCoop>
{
	StTime _Time;
public:
	void _Poll(void);
	StCoop *_Completed(void);
};

StCoopBox StCoops;

//********************************************* thread class

class StCoop
{
public:
	int _CoopComplete;

	StCoop()
	{
		_CoopComplete=0;
		// add to list
		StCoops+=this;
	}
	virtual ~StCoop()
	{
		// remove from list 
		StCoops-=this;
	}

	// pure virtual declaration for actual handler function
	virtual void _Poll(int Tick)=0;

};

//********************************************* functions depending on StCoop

void StCoopBox::_Poll(void)
{
	StBoxRef<StCoop> scan(StCoops);
	while (++scan)
		if (!scan->_CoopComplete)
			scan->_Poll(_Time._Since());
}

StCoop *StCoopBox::_Completed(void)
{
	StBoxRef<StCoop> scan(StCoops);
	while (++scan)
	{
		StCoop *ptr=scan;
		if (scan->_CoopComplete)
			return(ptr);
	}
	return(NULL);
}


#endif
