// stg template library
// automatic html/xml tags in io stream
// use: "test">> StTag("TAG") >> output;
//  or: buffer>>StTag("i")>>StTag("b")>>StTag("p","align=center")>>output;
// Note: when used as a pipe, it must be left to right!

#ifndef STGLIB_tag
#define STGLIB_tag

#include "/src/stglib/stcore.h"

// helpful notification on win32 compilers
#pragma message("using sttag.h")

/* version 1 with StFilter as base
class StTag:public StFilter
{
	StString _Tag;
	StString _Opt;
public:

	StTag(const char *tag,const char *opt=0)
	{
		_Tag<<tag;
		if (opt)
			_Opt<<opt;
	}
	virtual ~StTag()
	{
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		_Output("<");
		_Output(_Tag);
		if (~_Opt)
		{
			_Output(" ");
			_Output(_Opt);
		}
		_Output(">");

		_Output(data,size);

		_Output("<");
		_Output(_Tag);
		_Output(">\n");

		return(size);
	}
};
*/

/* version 2 with StBase */
class StTag:public StBase
{
	StString _Tag;
	StString _Content;
public:
	// opt is public so that it can be more easily modified
	StString _Opt;

	// store the tag and opts when constructed
	StTag(const char *tag,const char *opt=0)
	{
		STGLIB_CON("StTag");
		_Tag<<tag;
		if (opt)
			_Opt<<opt;
	}
	virtual ~StTag()
	{
		STGLIB_DES("StTag");
	}


	StTag& operator()(const char *opt)
	{
		!_Opt<<opt;
		return(*this);
	}

	// tell StBase::_Transport that this is a pipe
	// so that the data flows from one class to next
	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	// accept content to put between <tag> and </tag>
	virtual StSize _Write(const StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StTag","WRITE",data,size);
		return(_Content._Write(data,size));
	}

	// write output stream with tags following XML rules
	virtual StSize _WriteTo(StBase &Destination)
	{
//		STBASE_DEBUG_OP2("StTag","WriteTo",&Destination);

		Destination<<"<"<<_Tag;
		if (~_Opt)
			Destination<<" "<<_Opt;
		if (!~_Content)
		{
			Destination<<"/>";
			return(1);
		}
		Destination<<">";
		Destination<<_Content;
		Destination<<"</"<<_Tag<<">\n";
		!_Content;
		return(1);
	}
};


class StTagInput:public StTag
{
public:
	StTagInput(const char *name,
		const char *value,
		const char *type="text"):StTag("input")
	{
		_Opt<<"type=\""<<type<<"\"";
		_Opt<<" name=\""<<name<<"\"";
		_Opt<<" value=\""<<value<<"\"";
	}
	virtual ~StTagInput()
	{
	}
};

#endif
