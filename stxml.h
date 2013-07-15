// STGLIB/sxml.h
// Copyright 1999 by StG Net

#ifndef STGLIB_STXML
#define STGLIB_STXML

#pragma message("using stxml.h")

#include "/src/stglib/stcore.h"

// filter input text stream for XML tags and separate tables
// optional: pass in list of tags to match
class StXmlParse:public StBase
{
};

class StXml:public StBase
{
	StBase *base;
	StString tag;
public:
	StXml(StBase &b,const char *t)
	{
		base=&b;
		tag<<t;

		(*base)<<"<"<<tag<<">";
	}

	StXml(StBase &b,const char *t,const char *opts)
	{
		base=&b;
		tag<<t;

//		(*base)<<"<"<<tag<<" xml:link=\"simple\" show=\"replace\" href=\""<<href<<"\">";
		(*base)<<"<"<<tag<<" "<<opts<<">";
	}

	~StXml()
	{
		(*base)<<"</"<<tag<<">\n";
	}

	void Output(const char *s)
	{
		// encode data for xml quoted characters

		StString e;
		while (*s)
		{
			switch (*s)
			{
			case '&':
				e<<"&amp;";
				break;
			case '<':
				e<<"&lt;";
				break;
			case '>':
				e<<"&gt;";
				break;
			case '\"':
				e<<"&quot;";
				break;
			case '\'':
				e<<"&apos;";
				break;
			default:
				e+=*s;
				break;
			}
			s++;
		}
		(*base)<<e;
	}


	// pass down base class links to actual output class
	virtual StSize _Write(StByte *data,StSize size)
	{
		return(base->_Write(data,size));
	}

};

void StXmlTag(StBase &out,const char *tag,const char *msg)
{
	StXml temp(out,tag);
	temp.Output(msg);
}
void StXmlTagLink(StBase &out,const char *tag,const char *msg,const char *url)
{
	StXml temp(out,tag);
	{
		StString href;
		href<<"href="<<url;
		StXml link(temp,"a",href);
		link.Output(msg);
	}
}

/* from wikipedia:
XML character entity references

Unlike traditional HTML with its large range of character entity references, in XML there are only five predefined character entity references. These are used to escape characters that are markup sensitive in certain contexts:

    * &amp; ? & (ampersand, U+0026)
    * &lt; ? < (left angle bracket, less-than sign, U+003C)
    * &gt; ? > (right angle bracket, greater-than sign, U+003E)
    * &quot; ? " (quotation mark, U+0022)
    * &apos; ? ' (apostrophe, U+0027)
*/

#endif
