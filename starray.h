// STGLIB/starray.h
// Copyright 1998, 1999, 2002 by StG Net

// auto-resizing array mechanism and usage tracking

// Does not include any MT features - derived or utilizing class must handle that

#ifndef STGLIB_STARRAY
#define STGLIB_STARRAY

#pragma message("using starray.h")

#include "/src/stglib/stcore.h"

// increase in powers of 2 instead of previous method
#define STARRAY_POW2


#ifndef STARRAY_MIN_ALLOC
#define STARRAY_MIN_ALLOC 32
#endif


#ifndef STARRAY_MUL_ALLOC
#define STARRAY_MUL_ALLOC 256
#endif

#ifdef STARRAY_DEBUG

typedef struct
{
	void *_Ptr;
	void *_Inst;
} dbg_typ;

dbg_typ dbg[1024];
int dbg_cnt=0;

char *dbg_stop=0;

void DBG_ADD(void *ptr,void *inst,StSize size)
{
#ifdef STARRAY_DEBUG_TRACE
	printf("STARRAY ADD +%08X inst=%08X (%d)\n",ptr,inst,size);
#endif
	int loop=0;
	while (loop<dbg_cnt)
	{
		if (!dbg[loop]._Ptr)
			break;
		++loop;
	}
	dbg[loop]._Ptr=ptr;
	dbg[loop]._Inst=inst;
	if (dbg_cnt<=loop)
		dbg_cnt=loop+1;
}
void DBG_DEL(void *ptr,void *inst)
{
#ifdef STARRAY_DEBUG_TRACE
	printf("STARRAY DEL -%08X inst %08X\n",ptr,inst);
#endif

	int loop=0;
	while (loop<dbg_cnt)
	{
		if (dbg[loop]._Ptr==ptr)
		{
			if (dbg[loop]._Inst!=inst)
			{
				printf("STARRAY DBG @%08X allocated by %08X illegal del attempt by %08X\n",
					ptr,dbg[loop]._Inst,inst);
				*dbg_stop=0;
			}
			// correct delete - remove it
			dbg[loop]._Ptr=0;
			dbg[loop]._Inst=0;
			return;
		}
		++loop;
	}
	printf("STARRAY DBG del attempt %08X by %08X not found\n",
		ptr,inst);
	*dbg_stop=0;
}
void DBG_CHK(void *ptr,void *inst)
{
	if (!ptr)
		return;

	int loop=0;
	while (loop<dbg_cnt)
	{
		if (dbg[loop]._Ptr==ptr)
		{
			if (dbg[loop]._Inst!=inst)
			{
				printf("STARRAY DBG @%08X allocated by %08X trying to use by %08X\n",
					ptr,dbg[loop]._Inst,inst);
				*dbg_stop=0;
			}
			// correct ptr - return
			return;
		}
		++loop;
	}
	printf("STARRAY DBG check for %08X by %08X not found\n",
		ptr,inst);
	*dbg_stop=0;
}

#else
#define DBG_ADD(p,i,s)
#define DBG_DEL(p,i)
#define DBG_CHK(p,i)
#endif

// global memory allocated
// may roll over, but still valid for checking leaks
StSize _StArray_Bytes;

template <class T> class StArray
{
	// list of other classes that are
	// allowed to play with us directly
	friend class StBuffer;
	friend class StPipe;
	friend class StFifoBuffer;

protected:
	T *_ArrayPtr;		// ptr to actual storage
	StSize _ArraySize;	// elements allocated
	StSize _ArrayUsed;	// elements used (ever accessed)

public:
	// insure internals are preset to zero
	StArray()
	{
		STGLIB_CON("StArray");
		_ArrayPtr=0;
		_ArraySize=0;
		_ArrayUsed=0;
	}

	// don't leak allocation
	virtual ~StArray()
	{
		STGLIB_DES("StArray");
		if (_ArrayPtr)
		{
			DBG_DEL(_ArrayPtr,this);
//printf("Deleting %lX -> %lX\n",&_ArrayPtr,_ArrayPtr);

			delete[] _ArrayPtr;
			_ArrayPtr=0; // do this to make sure we don't try to re-use it later
			_StArray_Bytes-=_ArraySize*sizeof(T);
		}
	}

	// don't allow direct copy of class!!!
	StArray<T>& operator=(const StArray<T>& copyfrom)
	{
		if (this==&copyfrom)
			return(*this);

		DBG_CHK(_ArrayPtr,this);

		_ArrayResize(copyfrom._ArrayUsed,1);
		memcpy(_ArrayPtr,copyfrom._ArrayPtr,copyfrom._ArrayUsed*sizeof(T));
		_ArrayUsed=copyfrom._ArrayUsed;
		return(*this);
	}
	// NOTE!!! All derived classes MUST include a forward operator= function

/*	StArray(StArray<T>& copyfrom)
	{
		_ArrayResize(copyfrom._ArrayUsed,1);
		memcpy(_ArrayPtr,copyfrom._ArrayPtr,copyfrom._ArrayUsed*sizeof(T));
//		return(*this);
	}
*/


	// return size of array _IN USE_
	inline StSize operator~(void)
	{
		// return used elements (base class returns size of allocation)
		return(_ArrayUsed);
	}

	// %%% MUL_ALLOC should instead be scaled with needed

	// resize array to include at least X elements
	// this will deallocate array if X=0
	// if DontShrink is non-zero, save time by not downsizing
	void _ArrayResize(StSize Needed,int DontShrink=0)
	{
		DBG_CHK(_ArrayPtr,this);
		if (!Needed)
		{
			// when zero array size asked for,
			// always dump all memory
			if (_ArrayPtr)
			{
				DBG_DEL(_ArrayPtr,this);
				delete[] _ArrayPtr;
				_StArray_Bytes-=_ArraySize*sizeof(T);
			}
			_ArrayPtr=0;
			_ArraySize=0;
			return;
		}
		if (DontShrink)
		{
			if (Needed<=_ArraySize)
				return;
		}
		// HACK: increase needed by one so that there is always a zero terminator in list
		Needed++;

		
#ifdef STARRAY_POW2
		{
			// increase Needed to the next power of two
			StSize p=32;
			while (Needed>p)
				p<<=1;
			Needed=p;
		}
#else
		
		if (Needed<STARRAY_MIN_ALLOC)
			Needed=STARRAY_MIN_ALLOC;
		else
		{
			StSize r=Needed%STARRAY_MUL_ALLOC;
//			if (r)
			// this makes sure last entry always empty
			Needed+=STARRAY_MUL_ALLOC-r; // bump up to multiple
		}

#endif

//printf("Changing array from %d to %d\n",_ArraySize,Needed);


		T *NewPtr=new T[Needed];
		memset(NewPtr,0,sizeof(T)*Needed);
		if (_ArrayPtr)
		{
			DBG_DEL(_ArrayPtr,this);
			memcpy(NewPtr,_ArrayPtr,sizeof(T)*(_ArraySize<Needed?_ArraySize:Needed));
//printf("Deleting %lX -> %lX\n",&_ArrayPtr,_ArrayPtr);
			delete[] _ArrayPtr;
		}
		_ArrayPtr=NewPtr;
		DBG_ADD(_ArrayPtr,this,Needed);
//printf("Setting %lX -> %lX\n",&_ArrayPtr,_ArrayPtr);
		_StArray_Bytes-=_ArraySize*sizeof(T);
		_ArraySize=Needed;
		_StArray_Bytes+=_ArraySize*sizeof(T);

		// don't forget to downsize ArrayUsed too
		if (_ArrayUsed>_ArraySize)
			_ArrayUsed=_ArraySize;
	}
	// resize array and mark as used (useful for mapped structures)
	void _ArraySetUsed(StSize Needed)
	{
		_ArrayResize(Needed,1);
//		if (_ArrayUsed<Needed)
			_ArrayUsed=Needed;
	}

	// quickly zero out entire array but don't delete allocation
	inline void _ArrayEmpty(StSize size=0)
	{
		if (!size) size=_ArraySize;
		DBG_CHK(_ArrayPtr,this);
		memset(_ArrayPtr,0,sizeof(T)*size);
		_ArrayUsed=0;
	}
	inline void operator!(void)
	{
		_ArrayEmpty();
	}

	// return ref to element wanted
	inline T & operator[](StSize element)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed<=element)
			_ArrayUsed=element+1;
		if (_ArraySize<=element)
			_ArrayResize(element+1);
		return(_ArrayPtr[element]);
	}

	void _Reset(void)
	{
		_ArrayEmpty();
	}

	/* THIS IS COMMENTED OUT TO AVOID CONFUSION 
	// WITH STBASE STANDARD METHOD OF _Write()
	void _Write(const T *data,StSize count=1)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed+count>_ArraySize)
			_ArrayResize(_ArrayUsed+count,1);
		memcpy(_ArrayPtr+_ArrayUsed,data,sizeof(T)*count);
		_ArrayUsed+=count;
	}
	void _Write(const T data)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed+1>_ArraySize)
			_ArrayResize(_ArrayUsed+1,1);

//		memcpy(_ArrayPtr+_ArrayUsed,&data,sizeof(T));
		_ArrayPtr[_ArrayUsed]=data;

		_ArrayUsed+=1;
	}
	*/
	inline void _ArrayAdd(const T *data,StSize count=1)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed+count>=_ArraySize)
			_ArrayResize(_ArrayUsed+count+1,1);
		if (!count)
			return;
		if (count==1)
		{
			_ArrayPtr[_ArrayUsed]=*data;
		}
		else
		{
			memcpy(_ArrayPtr+_ArrayUsed,data,sizeof(T)*count);
		}
		_ArrayUsed+=count;
	}
/* this is no longer needed - assuming one extra byte always now
	// special zero termination version for strings
	void _ArrayAddZT(T *data,StSize count)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed+count+1>_ArraySize)
			_ArrayResize(_ArrayUsed+count+1,1);
		memcpy(_ArrayPtr+_ArrayUsed,data,sizeof(T)*count);
		_ArrayUsed+=count;
		memset(_ArrayPtr+_ArrayUsed,0,sizeof(T)*(_ArraySize-_ArrayUsed));
	}
*/
	void _ArrayAdd(T data)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed+1>_ArraySize)
			_ArrayResize(_ArrayUsed+1,1);
//		memcpy(_ArrayPtr+_ArrayUsed,&data,sizeof(T));
//		_ArrayUsed+=1;
		_ArrayPtr[_ArrayUsed++]=data;
	}

	StArray& operator<< (T data)
	{
		_ArrayAdd(data);
		return(*this);
	}

	// insert elements
	void _ArrayInsert(StSize element,StSize shift)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArraySize<=_ArrayUsed+shift)
			_ArrayResize(_ArrayUsed+shift);
//		if (_ArrayUsed<=element+shift)
//			_ArrayUsed=element+shift;

		StSize move=_ArrayUsed-element;
		T *InsPtr=_ArrayPtr+element;
		memmove(InsPtr+shift,InsPtr,sizeof(T)*move);
		memset(InsPtr,0,sizeof(T)*shift);
		_ArrayUsed+=shift;
	}
	void _ArrayInsert(StSize element,const T* data,StSize shift)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArraySize<=_ArrayUsed+shift)
			_ArrayResize(_ArrayUsed+shift);
//		if (_ArrayUsed<=element+shift)
//			_ArrayUsed=element+shift;

		StSize move=_ArrayUsed-element;
		T *InsPtr=_ArrayPtr+element;
		memmove(InsPtr+shift,InsPtr,sizeof(T)*move);
//		memset(InsPtr,0,sizeof(T)*shift);
		memcpy(InsPtr,data,sizeof(T)*shift);
		_ArrayUsed+=shift;
	}

	void _ArrayDelete(StSize element,StSize shift)
	{
		DBG_CHK(_ArrayPtr,this);
		if (_ArrayUsed<=element)
			_ArrayUsed=element+1;
		if (_ArraySize<=_ArrayUsed)
			_ArrayResize(_ArrayUsed);

		StSize move=_ArrayUsed-element-shift;
		T *DelPtr=_ArrayPtr+element;
		memcpy(DelPtr,DelPtr+shift,sizeof(T)*move);
		memset(DelPtr+move,0,sizeof(T)*shift);
		_ArrayUsed-=shift;
	}


};

// look like an array of objects,
// but hide the actual array of pointers to the objects
// and all the new/delete'ing
template <class T> class StArrayObj
{
	StArray<T*> _Array;

public:
	StArrayObj()
	{
		STGLIB_CON("StArrayObj");
	}
	virtual ~StArrayObj()
	{
		STGLIB_DES("StArrayObj");
		StSize index=0;
		while (index<~_Array)
		{
			T* ptr=_Array[index];
			if (ptr)
				delete ptr;
			++index;
		}
	}

	// don't allow direct copy of class!!!
	StArrayObj<T*>& operator=(StArrayObj<T*>& copyfrom)
	{
		if (this==&copyfrom)
			return(*this);

		_Array=copyfrom;
		return(*this);
	}

	inline T & operator[](StSize index)
	{
		T* ptr=_Array[index];
		if (!ptr)
			_Array[index]=ptr=new T;

		return(*ptr);
	}

	// return size of array _IN USE_
	inline StSize operator~(void)
	{
		// return used elements (base class returns size of allocation)
		return(~_Array);
	}

	void _Randomize(void)
	{
		// randomize the array
		StSize loop=0;
		while (loop<~_Array)
		{
			StSize r=rnd(~_Array-loop);
			T* temp=_Array[loop];
			_Array[loop]=_Array[r];
			_Array[r]=temp;
			++loop;
		}
	}
};


template <class T> class StStack
{
	StArray<T> _Array;
public:
	StStack()
	{
		STGLIB_CON("StStack");
	}
	virtual ~StStack()
	{
		STGLIB_DES("StStack");
	}
	inline T & operator[](StSize index)
	{
		return(_Array[index]);
	}
	inline T& operator++()
	{
		_Array._ArrayInsert(0,1);
		return(_Array[0]);
	}
	inline T operator--(int)
	{
		T copy;
		copy=_Array[0];
		_Array._ArrayDelete(0,1);
		return(copy);
	}
	inline StSize operator~(void)
	{
		return(~_Array);
	}
};

#endif
