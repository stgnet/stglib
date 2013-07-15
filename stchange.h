// STGLIB/stchange.h
// Copyright 2002 by StG Net

// change notify template


#ifndef STGLIB_STCHANGE
#define STGLIB_STCHANGE

#pragma message("using stchange.h")


template <class TYPE> class StChange
{
public:
	TYPE _Storage;

	StChange()
	{
	}

	~StChange()
	{
	}

	TYPE& operator= (TYPE& copyfrom)
	{
		if (_Storage!=copyfrom)
		{
			_Storage=copyfrom;
		}

		return(*this);
	}


};

#endif
