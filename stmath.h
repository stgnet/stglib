// STGLIB/stmath.h
// Copyright 2002 by StG Net

// math functions

// Make sure we include normal math definitions
#ifndef _INC_MATH
#include <math.h>
#endif



#ifndef STGLIB_STMATH
#define STGLIB_STMATH

#pragma message("using stmath.h")

#include "/src/stglib/stcore.h"

template <class TYPE> class StAverage
{
public:
	TYPE _Total;
	TYPE _Average;
	StSize _Over;

	StAverage()
	{
		_Total=0;
		_Average=0;
		_Over=0;
	}
	StAverage(StSize counts)
	{
		_Total=0;
		_Average=0;
		_Over=counts;
	}

	void operator()(StSize counts)
	{
		_Over=counts;
	}

	TYPE& operator+= (TYPE add)
	{
		_Average=(TYPE)(_Total/_Over);
		_Total-=_Average;
		_Total+=add;
		return(_Average);
	}

	inline operator TYPE(void)
	{
		return(_Average);
	}

};

class StRandomInit
{
public:
	// declared once globally to preload random seed from clock
	StRandomInit()
	{
		srand((unsigned int)time(0));
	}
} St_Random_Initialized;


class StRandom
{
	// to use this class, instantiate it (globally is ok) like this:
	// StRandom rnd;
	// and use it like this:
	// int x=rnd(10);
public:
	inline int operator()(int range)
	{
		if (!range)
			return(rand());
		return(rand()%range);
	}
};

long StFixedSqrt(long num) 
{
    if (!num)
		return(0);
    long n=(num/2)+1;
    long n1=(n+(num/n))/2;
    while (n1<n)
	{
        n=n1;
        n1=(n+(num/n))/2;
    }
    return(n);
}


#endif
