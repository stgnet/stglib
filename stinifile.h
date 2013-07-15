// STGLIB/StIniFile.h
// Copyright 1999,2002 by Scott Griepentrog & StG Net

// Ini File

#ifndef STGLIB_StIniFile
#define STGLIB_StIniFile

#pragma message("using stinifile.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/ststring.h"
#include "/src/stglib/stbox.h"
#include "/src/stglib/stfile.h"
#include "/src/stglib/stparse.h"
#include "/src/stglib/stfilter.h"

//
// USE:
//
// Instantiate the class with the file to read in:
//
// StIniFile("file.ini") ini_file;
//
// Read parameter:
//
// char *value=ini_file("section","param");
// 
// Set value:
//
// ini_file("section","param")=value;
//


//////////////////////////////////////////////////////////////////////
// StIniFileEntry - base class for directory entries (holds stamps)
//////////////////////////////////////////////////////////////////////
class StIniFileEntry
{
public:
	StString	Param;
	StString	Value;
	StString	Comment;

	StIniFileEntry()
	{
	}

	virtual ~StIniFileEntry()
	{
	}

	bool operator==(const char *s)
	{
		return(Param==s);
	}
	
};

//////////////////////////////////////////////////////////////////////
// StIniFileEntry - base class for directory entries (holds stamps)
//////////////////////////////////////////////////////////////////////
class StIniFileSection:public StBox<StIniFileEntry>
{
public:
	StString	Section;

	StIniFileSection()
	{
	}

	virtual ~StIniFileSection()
	{
	}

	bool operator==(const char *s)
	{
		return(Section==s);
	}
	
	StIniFileEntry* operator()(const char *s)
	{
		StIniFileEntry *entry=StBox<StIniFileEntry>::operator()(s);
		if (!entry)
		{
			entry=+(*this);
			entry->Param=s;
		}
		return(entry);
	}
};



//////////////////////////////////////////////////////////////////////
// StIniFile - load file and parse INI format structure
//////////////////////////////////////////////////////////////////////

class StIniFile:public StBox<StIniFileSection>
{
	StString filename;
	int Changed;
public:

	StIniFileSection* operator()(const char *s)
	{
		StIniFileSection *section=StBox<StIniFileSection>::operator()(s);
		if (!section)
		{
			section=+(*this);
			section->Section<<s;
		}
		return(section);
	}

	StIniFile(const char *file)
	{
		// read file, line by line
		// parse sections and values

		StIniFileSection *curr_sect=NULL;

		filename=file;

		StFile ini(filename);
		StFilterTextLine line(ini);

		while (1)
		{
			StParseString param;
			StParseAfter<';'> comment(param);
			StParseEnclosed<'[',']'> section(param);
			StParseAfter<'='> value(param);

			param<<line;
			if (line._Error)
				break;

			if (~section)
			{
				// set the section
				curr_sect=(*this)(section.GetString());
				continue;
			}
			if (!curr_sect)
			{
				// create "" (null) section
				curr_sect=(*this)("");
			}
			// save the value
			StIniFileEntry *entry=(*curr_sect)(param.GetString());
			!entry->Value<<value;
			if (~comment)
				!entry->Comment<<comment;
		}

		// start with changed flag set to zero
		Changed=0;
	}

	~StIniFile()
	{
		// if changed flag set, rewrite file
		if (!Changed)
			return;

		StFile ini(filename);
		StFilterTextLine line(ini);

		StBoxRef<StIniFileSection> sect(*this);
		while (++sect)
		{
			line<<"["<<sect->Section<<"]\n";

			StBoxRef<StIniFileEntry> entry(*sect);
			while (++entry)
			{
				if (~(entry->Param))
					line<<entry->Param<<"="<<entry->Value;
				if (~(entry->Comment))
					line<<";"<<entry->Comment;

				line<<"\n";
			}
		}
	}

	void _NotifyChanged(void)
	{
		// string value changed, will need to write output file
		Changed=1;
	}

	StString &operator()(const char *section,const char *param)
	{
		StIniFileEntry *entry=(*((*this)(section)))(param);
		// ask the string to tell me if it is changed
		entry->Value._Notify=this;
		return(entry->Value);
	}

};

#endif

