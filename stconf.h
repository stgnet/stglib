// stg template library
// process configuration (name=value) pairs (from file or other base)
// currently is read-only (no update supported)

#ifndef STGLIB_conf
#define STGLIB_conf

#include "/src/stglib/stcore.h"
#include "/src/stglib/stpair.h"
#include "/src/stglib/stparse.h"
#include "/src/stglib/stfilter.h"

// helpful notification on win32 compilers
#pragma message("using stconf.h")

class StConf:public StPair
{
	StString empty;
public:
	StConf(StBase &source)
	{
		StFilterTextLine Line(source);

		StString Input;
		while (!Line._Error)
		{
			StParseString value;
			StParseAfter<'#'> comment1(value);
			StParseAfter<';'> comment2(value);
			StParseBefore<'='> name(value);
			Line>>value;
			if (~name)
				!(StPair::operator()(name))<<value;
		}
	}

	virtual ~StConf()
	{
	}
};

#endif
