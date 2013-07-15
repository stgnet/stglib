// S1GLIB/stfifo.h
// Copyright 1999, 2002 by StG Net

// template class for "fifo" functionality (first in, first out)

#ifndef STGLIB_STFIFO
#define STGLIB_STFIFO

#pragma message("using stfifo.h")

#include "/src/stglib/stcore.h"

template <class TYPE> class StFifo
{
	StArray<TYPE*> _Table;	// table of ptrs to objects
	StSignal _ItemCount;	// count of items in fifo
	StSize _Index;		// index to first item to come out

	void _Lock(void)
	{
		STGLIB_MTLOCK("StFifo");
	}
	void _UnLock(void)
	{
		STGLIB_MTUNLOCK("StFifo");
	}

public:

	StFifo()
	{
		_ItemCount=0;
		_Index=0;
	}

	~StFifo()
	{
	}

	inline void operator! (void)
	{
		// empty the fifo
		!_Table;
		_ItemCount=0;
		_Index=0;
	}

	inline StSignal& operator~(void)
	{
		// this returns the count of items in fifo
		return(_ItemCount);
	}

	inline void operator+=(TYPE* Add)
	{
		if (_Index/2>_ItemCount)
		{
			// empty gap is larger than remaining items
			// move backwards to remove gap

			_Table._ArrayDelete(0,_Index);
			_Index=0;
		}
		_Table[_Index+_ItemCount]=Add;
		_ItemCount++;
	}

	// return pointer to current item (using fifo->item)
	inline TYPE* operator->(void)
	{
		if (_Index<0 || _Index>=~_Table)
			return((TYPE*)0);
		return(_Table[_Index]);
	}
	inline operator TYPE* (void)
	{
		if (_Index<0 || _Index>=~_Table)
			return((TYPE*)0);
		return(_Table[_Index]);
	}

	// go to next item (remove current item from fifo)
	inline TYPE* operator++(int)
	{
		TYPE* ret_ptr=_Table[_Index];

		_ItemCount--;
		if (!_ItemCount)
		{
			// nothing left
			!_Table;
			_Index=0;
			return(ret_ptr);
		}
		_Index++;
		return(ret_ptr);
	}
};

#endif
