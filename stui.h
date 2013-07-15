// STUI - user interface primitives
//
// Construct quick and dirty user interface dynamically based on live program data
// Extensible to most any text or graphical or even HTTP based environment

#ifndef STGLIB_STUI
#define STGLIB_STUI

#pragma message("using stui.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stbox.h"

class StUiDriver;


// this class may be relocated out to a separate driver-specific file
class StUiDriverElement
{
public:
	
};


// this is created on first instantiation of StUi
StUiDriver *_stglib_global_StUiDriver;
#define driver (*_stglib_global_StUiDriver)

class StUiElement
{
//	StUiDriverElement de;
public:
	StString Name;
	void Message(const char *msg)
	{
		!Name<<msg;
	}
	void Button(const char *name)
	{
		!Name<<name;
	}
};

// declared here, but defined after ui driver class loaded
class StUi:public StBox<StUiElement>
{
public:
	StUi(const char *title);
	StUiElement *Dialog(void);
};


// *****************************************
// THIS SECTION TO BE RELOCATED TO stuitty.h

// the driver stores global information on the display globally,
// and keeps field specific 

class StUiDriver
{
public:
	StUiDriver()
	{
	}
};


// *****************************************



// Define StUI functions here
StUi::StUi(const char *title)
{
	if (_stglib_global_StUiDriver)
		_stglib_global_StUiDriver=new StUiDriver;
}
StUiElement *StUi::Dialog(void)
{
	// activate driver, return selected element or NULL for abort
	return(NULL);
}


#endif