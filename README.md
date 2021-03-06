stglib
======

StG C++ Template Library

Scott Griepentrog
scott@stg.net
317-644-2228

PURPOSE:
------------

1) Rapid C++ Coding using a _single_ cpp file for entire application
	* No makefiles necessary
	* No precompiled headers
	* No keeping track of multiple .cpp files
	* No definitions (-DTHIS_THAT_AND_OTHER)
	* No rewriting code over and over (move it into stglib)
	* No researching best method for given task - s/b in library
	* Result: elegant and small apps, easily written, easily read
	* Side-effect: complexity of lib classes to keep flexibility
	* Requirement: good documentation of class methods
	* Easy to compile:
		- Linux/cygwin: g++ file.cpp -o file ; ./file
		- Win32/msvc: create console app from existing code,
			compile, run

2) Multiple platforms supported without needing to change code
	* Linux (gcc)
	* Windows (msvc6, msvc8)
	* Borland (x86)
	* Cygwin (gcc)
	* Posix
	* etc...

3) Easier stream or pipe style IO processing, including filters
	* StOpen file("file.txt"); file>>StdOutput;
	* StOpen file("file.txt"); StFilterTextLine filter;
		file>>filter>>StdOutput;

4) All memory allocation dynamic, no buffer overflows, no size limits
	* StArray<type> provides auto-resizing array
	* StBuffer provides array of char, interacts with IO
	* StString provides zero terminated string
	* StBox<type> provides containers with insert/del/sort/rnd/etc
	* StPipe provides FIFO stream handling
	* StFilter provides stream modification

5) Most classes inherit StBase class for common IO and Error handling
	* _Error - integer error number
	* _ErrorString - descriptive error message
	* _Read() - unified method of reading bytes from class
	* _Write() - unified method of writing bytes to class
	* and many other utility functions

6) Easily extensible by inheriting existing classes and overloading



INSTALLATION:
-----------------

1) svn co http://stglib.googlecode.com/svn/stglib

2) put contents of stglib directory in path /src/stglib
	(yes, it's hard coded, deal with it)

3) put your code wherever (must be same letter drive on Windows)

4) compile and run, that's it.


INCLUDES:
------------

1) All stglib based cpp must include "/src/stglib/stglib.h" first

2) Good idea to also include "/src/stglib/stdio.h", although probably already in

3) Include whatever other files you need for various components

4) Multiple .cpp's including stglib cannot be compiled together.
	Only one .cpp can include the stglib - if you have multiple
	.cpp's (e.g. g++ *.cpp -o program) then others may be "normal"
	c stdlib includes and only one includes the stglib instead
	(of course only that one can use stglib classes!)

COMPONENTS:
---------------

stagi.h		Asterisk Gateway Interface library calls (unfinished)
starray.h	Automatic resizing arrays (template)
staudio.h	Audio formats and soundcard IO
stbase.h	Base class for all stglib IO
stbitmap.h	Bitmap (.bmp) handling compatible with windows (unfinished)
stbox.h		Container class (put classes in the "box")
stbuffer.h	Automatic resizing byte array for IO buffering
stchange.h	Change notification mechanism (unfinshed)
stcsv.ha	CSV handling (unfinished)
stdatafile.h	Structured object storage into files (unfinished)
stdevgdi.h	Incomplete graphical interface (currently win32 only)
stdio.h		Standard IO (In/Out/Error)
stdirectory.h	Directory scanning
stdump.h	Utility to dump memory contents (handy for debugging)
sterror.h	Defines for _Error numbers
stfield.h	Mechanism for mapping object structure (unfinished)
stfifo.h	Template for arbitrary object FIFO
stfile.h	File IO using C standard IO (fopen/fclose)
stfilter.h	Filters (and base) for manipulating stream inline
stfont.h	Font for GDI (unfinished)
stgdi.h		Incomplete graphical interface (currently win32 only)
stglib.h	Main include file for library (must be included first)
sthttp.h	Http handling (unfinished)
stinifile.h	INI file handling (unfinished)
stmutex.h	Mutex for multi-threaded (unfinished)
stmysql.h	Mysql queries (requires libmysqlclient)
stpalmdb.h`	Palm database read and write
stparse.h	String parsing functions and base
stpipe.h	Byte stream FIFO
strect.h	Rectangular area handling (used by GDI)
stserial.h	Serial port IO
stserver.h	Mutli-threaded server handling (unifnished)
stmath.h	Math functions
stsockaddr.h	Handling for TCPIP addressing methods
stsock.h	Base class for TCPIP sockets
stsockif.h	Enumeration of Ethernet Interfaces
stsockraw.h	Raw packet processing
stsocktcp.h	TCP socket IO
stsockudp.h	UDP socket IO
stspeech.h	Speech processing (unfinished)
ststringbuf.h	Disused partial string manipulation (see StParse.h)
ststring.h	String handling generics
ststruct.h	Wrap a predefined structure with stglib IO processing
sttag.h		XML & HTML tag injection
sttcpservice.h	Disused tcp multithreaded server
stthread.h	Multiple thread handling
sttime.h	Date and Time processing
stui.h		Generic UI (unfinished)
stwave.h	Read/write of audio .WAV files
stwindow.h	Disused windows gui interface
stxml.h		XML generation/parsing (unfinished)

The following compnents are included automatically by others,
	and do not need (and should not) be included manually.

stglibpt.h	Definitions of stglib types prior to platform
stp_cygwin.h	Platform includes for cygwin
stp_gunix.h	Platform includes for generic unix (used by cygwin,posix)
stplatform.h	Determine platform and include proper files
stpostfx.h	Definitions of cross-class functions
stp_posix.h	Platform includes for posix 
stp_turboc.h	Platform includes for borland turbo c++
stp_win32.h	Platform includes for microsoft windows msvc
stsockwse.h	Definitions for windows sockets

