// stg template library
// storage of name=value pairs

#ifndef STGLIB_pair
#define STGLIB_pair

#include "/src/stglib/stcore.h"
#include "/src/stglib/stbox.h"
#include "/src/stglib/stparse.h"

// helpful notification on win32 compilers
#pragma message("using stpair.h")

class StPairEntry
{
public:
	StString Name;
	StString Value;

	StPairEntry()
	{
	}

	virtual ~StPairEntry()
	{
	}
};

class StPair:public StBox<StPairEntry>
{
public:
	StPair()
	{
	}

	virtual ~StPair()
	{
	}

	StString& operator()(const char *match)
	{
		StBoxRef<StPairEntry> scan(*this);
		while (++scan)
		{
			if (scan->Name==match)
				return(scan->Value);
		}
		StPairEntry &add=+(*this);
		add.Name<<match;
		return(add.Value);
	}
	StString & operator()(StBase &name)
	{
		StString match;
		name>>match;
		StBoxRef<StPairEntry> scan(*this);
		while (++scan)
		{
			if (scan->Name==match)
				return(scan->Value);
		}
		StPairEntry &add=+(*this);
		add.Name<<match;
		return(add.Value);
	}
	int _Defined(const char *match)
	{
		StBoxRef<StPairEntry> scan(*this);
		while (++scan)
		{
			if (scan->Name==match)
				return(1);
		}
		return(0);
	}
	StSize _AddEquals(StBase& string)
	{
		StParseString left;
		StParseAfter<'='> right(left);

		string>>left;
		if (~left)
			!((*this)(left))<<right;
		return(~left);
	}
};

#endif
