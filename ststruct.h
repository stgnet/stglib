// STGLIB/ststruct.h
// Copyright 1996,1998,2000,2002 by StG Net

// template to convert a structure into StBase style i/o handling


#ifndef STGLIB_STSTRUCT
#define STGLIB_STSTRUCT

#pragma message("using ststruct.h")

#include "/src/stglib/stcore.h"

template <class TYPE> class StStruct:public StBase
{
private:
	TYPE _Storage;

public: 
	StStruct()
	{
	}

	~StStruct()
	{
	}


	inline TYPE* operator->(void)
	{
		return(&_Storage);
	}

	inline TYPE* operator&(void)
	{
		return(&_Storage);
	}

	virtual StSize _WriteTo(StByte *data,StSize size)
	{
		// add data to buffer
		if (!data) 
			return(0);

		if (size!=sizeof(_Storage))
			return(_Err(StErr_InvalidDataLength,"StStruct","_Write"));

		memcpy(&_Storage,data,size);
		return(size);
	}
	StSize _Read(StByte *data,StSize size)
	{
		if (size>sizeof(_Storage))
			size=sizeof(_Storage);

		if (size)
		{
			memcpy(data,&_Storage,size);
		}
		return(size);
	}

	virtual StSize _Read(StBase &Source)
	{
		StSize Got=Source._Read((StByte*)&_Storage,sizeof(_Storage));
		if (Source._Error)
			return(_Err(Source));
		if (Got!=sizeof(_Storage))
			return(_Err(StErr_InvalidDataLength,"StStruct","_Read"));
		return(Got);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Wrote=Destination._Write((StByte*)&_Storage,sizeof(_Storage));
		if (Destination._Error)
			return(_Err(Destination));
		if (Wrote!=sizeof(_Storage))
			return(_Err(StErr_InvalidDataLength,"StStruct","_Write"));
		return(Wrote);
	}

	virtual StSize _Available(void)
	{
		return(sizeof(_Storage));
	}
};

#endif
