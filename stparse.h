// STGLIB/stparse.h
// string parsing using sections


#ifndef STGLIB_STPARSE
#define STGLIB_STPARSE

#pragma message("using stparse.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststring.h"

class StParseBase:public StStringSection
{
protected:
	StParseBase *_next;
	StParseBase *_child;

public: 
	StParseBase()
	{
		_next=0;
		_child=0;
	}

	virtual ~StParseBase()
	{
		if (_next)
			_next=_next->_next;
	}

	virtual void Process(StParseBase &target)
	{
	}

	void _ParseAll(StParseBase &target)
	{
//StdOutput<<"Walking "<<(const)&(*this)<<"\n";

		StParseBase *scan=_child;
		while (scan)
		{
//StdOutput<<"  this="<<(const)this<<" scan="<<(const)scan<<" target="<<(const)&target<<"\n";

//StdOutput<<" "<<(const)scan<<" Process "<<(const)&target<<"\n";		
			

			scan->_SectionMatch(*this);
			scan->Process(*this);
			scan->_ParseAll(*this);
			scan=scan->_next;
		}

//StdOutput<<"Done\n";
	}

	void _ParseAddStep(StParseBase &step)
	{
//StdOutput<<(const)&(*this)<<" adding "<<(const)&(step)<<"\n";
		StParseBase *scan=_child;
		if (!scan)
		{
			_child=&step;
		}
		else
		{
			while (scan->_next)
				scan=scan->_next;
			scan->_next=&step;
		}
	}

	//	virtual void _Reset(void)
//	{
//		_Offset=0;
//		_Length=0;
//	}

};

class StParseString:public StParseBase
{
protected:
	StString _Storage;

public:
	StParseString()
	{
		(*this)(_Storage);
	}

	virtual ~StParseString()
	{
	}

	// allow array to duplicate storage to prevent pointer/alloc corruption!
	StParseString& operator= (StParseString& copyfrom)
	{
		_Storage=copyfrom._Storage;
		return(*this);
	}

/*	inline operator const char *(void)
	{
		return((const char*)_Storage);
	}
*/

	virtual inline void _ReadStart(StBase &Source)
	{
		!_Storage;
	}


	// wrap the write call with extra post-processing
	virtual StSize _Write(const StByte *data,StSize size)
	{
		// map the write to the storage buffer
		size=_Storage._Write(data,size);
		if (_Storage._Error)
			return(_Err(_Storage));

		StParseBase::_Reset();
		_ParseAll(*this);
		return(size);
	}

	// wrap the read call with extra post-processing
	virtual StSize _Read(StBase &Source)
	{
		StSize size=_Storage._ReadFrom(Source);
		if (_Storage._Error)
			return(_Err(_Storage));

		StParseBase::_Reset();
		_ParseAll(*this);

		return(size);
	}

	virtual void _Reset(void)
	{
		!_Storage;
	}
};

// parse up to terminating character (which is removed)
template <StByte Term> class StParseBefore:public StParseBase
{
public:
	StParseBefore(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

//StdOutput<<(const)&(*this)<<"Parsing "<<target<<"@"<<(const)&(target)<<" for "<<Term<<"\n";

		StByte *scan=start;
		while (scan<end)
		{
			if (*scan==Term)
			{
//				(*this)(start-_Reference(),scan-start);
				(*this)(start,scan-start);
				++scan;
//				target(scan-_Reference(),end-scan);
				target(scan,end-scan);
//StdOutput<<"Result: "<<(*this)<<"|"<<target<<"\n";
				
				
				
				return;
			}
			++scan;
		}
		(*this)(start,end-start);
		target(end,0);
	}
};

// parse first x characters
template <int count> class StParseFirst:public StParseBase
{
public:
	StParseFirst(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *cut=start;
		if (cut+count>end)
			cut=end;

		(*this)(start,cut-start);
		target(cut,end-cut);
	}
};

class StParseCSV:public StParseBase
{
	StArrayObj<StString> element;

public:
	StParseCSV(StParseBase &target)
	{
		// attach this section to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *scan=start;
		while (scan<end)
		{
			if (*scan==',')
			{
				
				element[~element]._Write(start,scan-start);
//				(*this)(start,scan-start);
//				_Write(element(~element));
//				element[~element]<<(*this);
				++scan;
				target(scan,end-scan);
				return;
			}
			++scan;
		}
		(*this)(start,end-start);
		element[~element]<<(*this);
		target(end,0);
	}

	inline StStringSection& operator()(StByte *ptr,StSize length)
	{
		return(StStringSection::operator ()(ptr,length));
	}
	inline StStringSection& operator()(StString &sb)
	{
		return(StStringSection::operator ()(sb));
	}

	inline StStringSection& operator()(StStringSection &sbs)
	{
		return(StStringSection::operator ()(sbs));
	}
	inline StString& operator()(StSize index)
	{
		return(element[index]);
	}

};


// obtain string AFTER terminating character (which is removed)
template <StByte Term> class StParseAfter:public StParseBase
{
public:
	StParseAfter(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

//StdOutput<<(const)&(*this)<<"Parsing "<<target<<"@"<<(const)&(target)<<" for "<<Term<<"\n";
		StByte *scan=start;
		while (scan<end)
		{
			if (*scan==Term)
			{
				target(start,scan-start);
				++scan;
				(*this)(scan,end-scan);
//StdOutput<<"Result: "<<(*this)<<"|"<<target<<"\n";
				return;
			}
			++scan;
		}
		target(start,end-start);
		(*this)(end,0);
//StdOutput<<"xesult: "<<(*this)<<"|"<<target<<"\n";
	}
};

// obtain string AFTER terminating character sequence (which is removed)
template <StByte Term1,StByte Term2> class StParseAfterTwo:public StParseBase
{
public:
	StParseAfterTwo(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *scan=start;
		while (scan<end-1)
		{
			if (*scan==Term1 && scan[1]==Term2)
			{
				target(start,scan-start);
				++scan;
				++scan;
				(*this)(scan,end-scan);
				return;
			}
			++scan;
		}
		target(start,end-start);
		(*this)(end,0);
	}
};

// obtain string before terminating character sequence (which is removed)
template <StByte Term1,StByte Term2> class StParseBeforeTwo:public StParseBase
{
public:
	StParseBeforeTwo(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *scan=start;
		while (scan<end-1)
		{
			if (*scan==Term1 && scan[1]==Term2)
			{
				(*this)(start,scan-start);
				++scan;
				++scan;
				target(scan,end-scan);
				return;
			}
			++scan;
		}
		target(start,end-start);
		(*this)(end,0);
	}
};

// obtain string AFTER match string
template <const char * match> class StParseAfterString:public StParseBase
{
public:
	StParseAfterString(StParseBase &target)
	{
		(*this)(target);
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *scan=start;
		StByte *mtch=(StByte*)match;
		StByte *last=start;
		while (scan<end)
		{
			mtch=(StByte*)match;
			last=scan;

			while (*mtch && *scan==*mtch && scan<end)
			{
				++scan;
				++mtch;
				if (!*mtch)
				{
					// found match
					target(start,last-start);
					(*this)(scan,end-scan);
					return;
				}
			}
			++scan;
		}

		// failed to match, return null string
		target(start,end-start);
		(*this)(end,0);
	}
};

// trim string of multiple instances of character before or after
template <StByte Trim> class StParseTrim:public StParseBase
{
public:
	StParseTrim(StParseBase &target)
	{
		(*this)(target);
		target._ParseAddStep(*this);
	}
	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;


		// note: target is not moved, only this
		StByte *scan=start;
		while (scan<end && *scan==Trim)
			++scan;
		while (scan<end && *(end-1)==Trim)
			--end;

		(*this)(scan,end-scan);
	}
};

		


// parse up to last instance of terminating character (which is removed)
template <StByte Term> class StParseTermLast:public StParseBase
{
public:
	StParseTermLast(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;
		StByte *match=0;

		StByte *scan=start;
		while (scan<end)
		{
			if (*scan==Term)
			{
				match=scan;
			}
			++scan;
		}
		if (match)
		{
			(target)(start,match-start);
			++match;
			(*this)(match,end-match);
			return;
		}
		(*this)(start,end-start);
		target(end,0);
	}
};



// obtain string between two different quote characters
// NOTE: this was changed to obtain nothing if enclosing characters not found
template <StByte Left,StByte Right> class StParseEnclosed:public StParseBase
{
public:
	StParseEnclosed(StParseBase &target)
	{
		// attach StStringSection to parent target
		(*this)(target);

		// link this instance into target for parsing
		target._ParseAddStep(*this);
	}

	void Process(StParseBase &target)
	{
		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte *scan=start;
		while (scan<end)
		{
			if (*scan==Left)
			{
				++scan;
				StByte *scan2=scan;
				while (scan2<end)
				{
					if (*scan2==Right)
					{
						// found it
						(*this)(scan,scan2-scan);
						++scan2;
						target(scan2,end-scan2);
						return;
					}
					++scan2;
				}
			}
			++scan;
		}
		target(start,end-start);
		(*this)(end,0);
	}
};

template <StByte CharCount> class StParseHex:public StParseBase
{
public:
	unsigned long value;
	StParseHex(StParseBase &target)
	{
		(*this)(target);
		target._ParseAddStep(*this);
	}
	void Process(StParseBase &target)
	{
		value=0;

		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		StByte count=CharCount;
		StByte *scan=start;
		while (scan<end)
		{
			if (*scan<'0')
				break;

			if (*scan>'9' && *scan<'A')
				break;

			if (*scan>'F' && *scan<'a')
				break;

			if (*scan>'f')
				break;

			value<<=4;
			if (*scan>'9')
				value|=((*scan)+9)&0xF;
			else
				value|=(*scan)&0xF;

			++scan;
			--count;
			if (!count)
			{
				(*this)(start,scan-start);
				target(scan,end-scan);
				return;
			}
		}
	}
	operator unsigned long()
	{
		return(value);
	}
};
/*
class StParseCaps:public StParseBase
{
public:
	StParseCaps(StParseBase &target)
	{
		(*this)(target);
		target._ParseAddStep(*this);
	}
	void Process(StParseBase &target)
	{
		value=0;

		StByte *start=(StByte*)target;
		StByte *end=start+~target;

		while (start<end)
		{
			if (*scan>='a' && *scan<='z')
				*scan-=32;
			++scan;
		}
	}
};
*/

#endif
