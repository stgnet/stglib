// S1GLIB/stbox.h
// Copyright 1999, 2002 by StG Net

// base class for "box" functionality (put items in a box, enumarate)

//template <class T> class StBoxItems;

#ifndef STGLIB_STBOX
#define STGLIB_STBOX

#pragma message("using stbox.h")

template <class TYPE> class StBoxRef;

template <class TYPE> class StBox:public StBase
{
	friend class StBoxRef<TYPE>;


	StArray<TYPE*> _Table;	// table of ptrs to objects
	StSize _ItemCount;	// count of items in box
	int _BoxAutoDelete;

/*
	void _Lock(void)
	{
		STGLIB_MTLOCK("StBox");
	}
	void _UnLock(void)
	{
		STGLIB_MTUNLOCK("StBox");
	}
*/

public:

	inline StBox(int auto_delete=0)
	{
		STBASE_DEBUG_CON("StBox");
		_ItemCount=0;
		_BoxAutoDelete=auto_delete;
	}

	void _BoxEmpty(void)
	{
		if (_BoxAutoDelete)
		{
			StSize scan=0;
			while (scan<~_Table)
			{
				TYPE *Remove=_Table[scan];
				if (Remove)
				{
					_BoxNotifyParentOfChildRemoved(Remove);
					delete Remove;
					_Table[scan]=NULL;
				}
				scan++;
			}

		}
		_ItemCount=0;
	}

	virtual ~StBox()
	{
		_BoxEmpty();
	}

	inline StBox& operator= (StBox& copyfrom)
	{
		StArray<TYPE*>::operator=(copyfrom);
		_ItemCount=copyfrom._ItemCount;
		return(*this);
	}

	// this is virtual so inheriting class can
	// override it and provide additional functionality
	inline virtual void operator! (void)
	{
		// empty the box
		_BoxEmpty();
	}

	inline StSize operator~(void)
	{
		// this returns the count of items in box
		// which can be different from ~_Table
		return(_ItemCount);
	}
	inline StSize _BoxCount(void)
	{
		return(_ItemCount);
	}

	// dummy virtual functions - redefined if to be used by inheriting class
	inline virtual void _BoxNotifyParentOfChild(TYPE *Add)
	{
	}
	inline virtual void _BoxNotifyParentOfChildRemoved(TYPE *Remove)
	{
	}

	inline StSize _BoxAdd(TYPE* Add)
	{
		if (_ItemCount<~_Table)
		{
			// there's an empty spot - find it 
			StSize scan=0;
			while (scan<~_Table)
			{
				if (!_Table[scan])
				{
					_Table[scan]=Add;
					_ItemCount++;
					_BoxNotifyParentOfChild(Add);
					return(scan);
				}
				scan++;
			}
		}
		// extend table
		StSize last=~_Table;
		_Table[last]=Add;
		_ItemCount++;
		_BoxNotifyParentOfChild(Add);
		return(last);
	}

/*
	inline StSize _BoxAdd(TYPE* Add)
	{
		if (_ItemCount<~_Table)
		{
			// there's an empty spot - find it 
			StSize scan=0;
			while (scan<~_Table)
			{
				if (!_Table[scan])
				{
					_Table[scan]=Add;
					_ItemCount++;
					_BoxNotifyParentOfChild(Add);
					return(scan);
				}
				scan++;
			}
		}
		// extend table
		StSize index=~_Table;
		_Table[index]=Add;
		_ItemCount++;
		_BoxNotifyParentOfChild(Add);
		return(index);
	}
*/
	inline void operator+=(TYPE* Add)
	{
		_BoxAdd(Add);
	}

	// auto add constructs operator and flags for auto delete
	// won't work for classes inheriting TYPE though...
	inline TYPE& operator+()
	{
//		if (!~_Table)
//			_BoxAutoDelete=1;
		TYPE* add=new TYPE;
		_BoxAdd(add);
		return(*add);
	}

	inline void operator-=(TYPE* Remove)
	{
		StSize scan=0;
		while (scan<~_Table)
		{
			if (_Table[scan]==Remove)
			{
				_Table[scan]=0;
				_ItemCount--;
//				Remove->_BoxNotifyParent(0);
				_BoxNotifyParentOfChildRemoved(Remove);
				return;
			}
			scan++;
		}
		// shouldn't ever get here...
	}

	// reference directly by index
/*	inline virtual TYPE& operator[](int index)
	{
		if (index<0 || (StSize)index>=~_Table)
			throw "StBox: attempt to access element beyond used portion of array";
		TYPE* ptr=_Table[index];
		if (!ptr)
			throw "StBox: attempt to access element that does not exist";
		return(*_Table[index]);
	}
*/

	// implemented as a bubble sort to keep == items in order
	void _BoxSort(void)
	{
		TYPE *temp;
		StSize one;
		StSize two;

		while (1)
		{
			one=0;

			while (one<~_Table && !_Table[one])
					one++;

			if (one==~_Table)
				break;

sort_next:
			two=one;
			two++;
			while (two<~_Table && !_Table[two])
					two++;

			if (two==~_Table)
				break;

			if (*(_Table[one]) > *(_Table[two]))
			{
//printf("%d ",one);
				temp=_Table[one];
				_Table[one]=_Table[two];
				_Table[two]=temp;
				continue;
			}
/*				if (*(_Table[one]) > *(_Table[two]))
				{
					temp=_Table[one];
					_Table[one]=_Table[two];
					_Table[two]=temp;
					continue;
				}
*/
			one=two;
			goto sort_next;

		}
	}

	void _BoxRandomize(void)
	{
		TYPE *temp;
		StSize one;
		StSize two;

		one=0;
		while (one<~_Table)
		{
			two=one+rand()%(~_Table-one);

			temp=_Table[one];
			_Table[one]=_Table[two];
			_Table[two]=temp;

			++one;
		}
	}

//	virtual StBase& operator<< (StBase &that);
//	virtual StBase& operator>> (StBase &that);


	// find entry where TYPE==s
	TYPE* operator()(const char *s)
	{			
		StSize scan=0;
		while (scan<~_Table)
		{
			TYPE *entry=_Table[scan];

			if (entry)
			if ((*entry)==s)
//			if (entry->operator==(s))
				return(entry);

			scan++;
		}

		return(NULL);
	}

	// find entry where TYPE==s
	TYPE* operator()(const StSize n)
	{			
		StSize scan=0;
		while (scan<~_Table)
		{
			TYPE *entry=_Table[scan];

			if ((*entry)==n)
				return(entry);

			scan++;
		}

		return(NULL);
	}

	// find entry where TYPE==base
	TYPE* operator()(StBase &find)
	{			
		StSize scan=0;
		while (scan<~_Table)
		{
			TYPE *entry=_Table[scan];

			if ((*entry)==find)
				return(entry);

			scan++;
		}

		return(NULL);
	}

};

template <class TYPE> class StBoxRef
{
	StSize _Index;		// index to current item in box
	StBox<TYPE> *_Box;	// reference to box

public:

	StBoxRef(StBox<TYPE>&box)
	{
		_Box=&box;
		_Index=-1;
	}
	virtual ~StBoxRef()
	{
	}

	inline void operator!(void)
	{
		// reset index
		_Index=-1;
	}

	// return pointer to current item (using box->item)
	inline TYPE* operator->(void)
	{
		if (_Index<0 || _Index>=~(_Box->_Table))
			return((TYPE*)0);
		return((_Box->_Table)[_Index]);
	}

	inline operator TYPE& ()
	{
		if (_Index<0 || _Index>=~(_Box->_Table))
			return(*((TYPE*)0));
		return(*((_Box->_Table)[_Index]));
	}

	inline operator TYPE* ()
	{
		if (_Index<0 || _Index>=~(_Box->_Table))
			return( ((TYPE*)0));
		return( ((_Box->_Table)[_Index]));
	}

	// go to next item
	inline TYPE* operator++(void)
	{
		while (1)
		{
			_Index++;
			if (_Index<0 || _Index>=~(_Box->_Table))
			{
				_Index=-1;
				return((TYPE*)0);
			}

			if ((_Box->_Table)[_Index])
				return((_Box->_Table)[_Index]);
		}
	}

	// go to previousitem
	inline TYPE* operator--(void)
	{
		// convert reset index for reverse direction
		if (_Index==-1)
			_Index=~(_Box->_Table);

		while (1)
		{
			_Index++;
			if (_Index<0 || _Index>=!(_Box->_Table))
			{
				_Index=-1;
				return((TYPE*)0);
			}

			if ((_Box->_Table)[_Index])
				return((_Box->_Table)[_Index]);
		}
	}




};

#ifdef bogus

	// follow tree structure, writing each item at each level
	// for StDataFile, but could be used for other purposes
	template <class TYPE> StBase& StBox<TYPE>::operator>> (StBase &parent)
	{
//printf("Write: %s >> %s\n",this->_DataName(),parent._DataName());

		StBase &child=parent._RecurseLevel(*this);

		// look through the items in the box and write them to the datafile
		StBoxRef<TYPE> item(*this);
		while (++item)
			(*item)>>(child);

		// if child & parent are not the same, assume that the child
		// was new'd by _RecurseLevel and thus needs deleting
		if (&child!=&parent)
			delete &child;

		return(*this);
	}

	// follow tree structure, writing each item at each level
	// for StDataFile, but could be used for other purposes
	template <class TYPE> StBase& StBox<TYPE>::operator<< (StBase &parent)
	{
		StBase &child=parent._RecurseLevel(*this);

		// look through the items in the box and write them to the datafile
		StBoxRef<TYPE> item(*this);
		while (++item)
			(*item)<<(child);

		return(*this);
	}

#endif


#include "/src/stglib/stpostfx.h"

#endif


/* USE THIS NEW ALGORITHM INSTEAD

 * @(#)OETransSortAlgorithm.java	95/11/22 Andrew Kitchen
 *

 * An Odd-Even Transposition sort demonstration algorithm
 *
 * @author Andrew Kitchen
 * @version 	22 Nov 1995


class OETransSortAlgorithm extends SortAlgorithm {
    void sort(int a[]) throws Exception {
	pause(0,a.length-1);
	for (int i = 0; i < a.length/2; i++ ) {
		if (stopRequested) {
		    return;
		}
		for (int j = 0; j+1 < a.length; j += 2) 
		    if (a[j] > a[j+1]) {
		        int T = a[j];
		        a[j] = a[j+1];
		        a[j+1] = T;
		    }
		pause(); pause();
		for (int j = 1; j+1 < a.length; j += 2) 
		    if (a[j] > a[j+1]) {
		        int T = a[j];
		        a[j] = a[j+1];
		        a[j+1] = T;
		    }
		pause(); pause();
	}	
	pause(-1,-1);
    }
}

*/

/* NEW SORT ALGORITHM - USE TO REPLACE BUBBLE SORT

	NEVER MIND - THIS ASSUMES NUMERIC VALUE, NOT PURE COMPARISON

 * A shear sort demonstration algorithm
 * SortAlgorithm.java, Thu Nov 27 1995
 *
 * author Andrew Kitchen

class ShearSortAlgorithm extends SortAlgorithm {
    private int Log, Rows, Cols;

    void sort(int a[]) throws Exception {
	int pow=1, div=1;
	int h[];

	for(int i=1; i*i<=a.length; i++) 
	    if (a.length % i == 0) div = i;
	Rows = div; Cols = a.length / div;
	for(Log=0; pow<=Rows; Log++) 
	    pow = pow * 2;

	h = new int[Rows];
	for (int i=0; i<Rows; i++)
	    h[i]=i*Cols;

	for (int k=0; k<Log; k++) {
	    for (int j=0; j<Cols/2; j++) {
		for (int i=0; i<Rows; i++)
	            sortPart1(a,i*Cols,(i+1)*Cols,1,(i%2==0?true:false));
		apause(h);
		for (int i=0; i<Rows; i++)
	            sortPart2(a,i*Cols,(i+1)*Cols,1,(i%2==0?true:false));
		apause(h);
	    }
	    for (int j=0; j<Rows/2; j++) {
		for (int i=0; i<Cols; i++)
	            sortPart1(a,i,Rows*Cols+i,Cols,true);
		apause(h);
		for (int i=0; i<Cols; i++)
	            sortPart2(a,i,Rows*Cols+i,Cols,true);
	        apause(h);
	    }
	}
	for (int j=0; j<Cols/2; j++) {
	    for (int i=0; i<Rows; i++)
	        sortPart1(a,i*Cols,(i+1)*Cols,1,true);
	    apause(h);
	    for (int i=0; i<Rows; i++)
	        sortPart2(a,i*Cols,(i+1)*Cols,1,true);
	    apause(h);
	}
	for (int i=0; i<Rows; i++)
	    h[i]=-1;
	apause(h);
    }

    private void sortPart1(int a[], int Lo, int Hi, int Nx, boolean Up) throws Exception {
	    for (int j = Lo; j+Nx<Hi; j+=2*Nx) 
		if ((Up && a[j] > a[j+Nx]) || !Up && a[j] < a[j+Nx]) {
		    int T = a[j];
		    a[j] = a[j+Nx];
		    a[j+Nx] = T;
		}
    }

    private void sortPart2(int a[], int Lo, int Hi, int Nx, boolean Up) throws Exception {
	    for (int j = Lo+Nx; j+Nx<Hi; j+=2*Nx) 
		if ((Up && a[j] > a[j+Nx]) || !Up && a[j] < a[j+Nx]) {
		    int T = a[j];
		    a[j] = a[j+Nx];
		    a[j+Nx] = T;
		}
    }
}

*/

