// STGLIB/stbase.h
// Copyright 1999,2002 by StG Net

// base class
//
// Provides base functionality for i/o handling, including mechanisms to handle
// reading data from class to an i/o path, and writing data from a buffer
// to the i/o path.  This class must be inherited to be used, and the inheriting
// class must provide the methods to read and write the data.
//
// class inheriting *must* define at the minimum the _Read()
// and _Write() calls (these are pure virtual to make sure)
//
// examples include files and sockets (see StFile and StSock)
//
// EXAMPLE USES
//
// !buffer << file("name"); - erase buffer, then load contents of file into buffer
// buffer >> !file("name"); - erase file, then write contents of buffer to file
//
// For convenience, these can also be reversed:
//
// !file("name") << header << data; - erase file, write two consecutive buffers
//
// sock << file("data");
// !file("result") << file("one") << file("two");
//
// and buffer to buffer
//
// !message << prefix << "foobar" << checksum;
//
// The << and >> operators simply read from one side and *add* to the other side

// STGLIB version2 (2004)
// modified StBase to reduce overhead by including base error handling without
// pre-allocated string (char array) class.  Error handling is now dynamic, when
// an error occurs the error trace info is created at that point.


// Modifications 2005-06-08
// having difficulties with stream vs. storage objects
// certain functions assume reading from a stream object and do so repeatedly
// and: having issues with repeatedly copying data to process it
// so:-
// changing base to understand the concept of _possible_ direct access to
// a storage area memory section on a read-only basis.

// base now to understand different types of data objects

// Packet - has storage, but each write erases previous copy
// Stream - does not show storage but may internally contain storage, reads until zero returned
// Buffer - multiple writes accumulate, storage
// Section - specifies a portion of another buffer, with a start index and length

// interaction between different types must be defined.


// ### NEW RULES:
// 1) If direct access to storage is available, those contents are assumed to be entire set of data
// 2) If direct access not available, it is a stream, thus must read until zero return

// buffer: reads entire buffer, writes add to existing, direct r/o access allowed
// packet: same as buffer but erases previous data on write
// stream/pipe: reads data with zero on end, writes normal, no access
// section: same as buffer but storage is linked to another buffer



#ifndef STGLIB_STBASE
#define STGLIB_STBASE

#pragma message("using stbase.h")

#ifdef STBASE_DEBUG

#ifndef STBASE_DEBUG_FILE
FILE *_dbg_file_=stdout;
#else
FILE *_dbg_file_=fopen(STBASE_DEBUG_FILE,"w");
#endif

void _STGLIB_Dump(const StByte *data,StSize size)
{
	StSize orig_size=size;
	StSize loop,line;
	unsigned char databyte;
	static unsigned char buf[80];
	unsigned char *pbuf;
	unsigned int value;
	char *hex_digit="0123456789ABCDEF";

	if (size==-1 || !data || !size)
		return;

	line=0;
	while (size)
	{
		pbuf=buf;
		loop=4;
		value=line;
		pbuf+=4;
		while (loop--)
		{
			*--pbuf=hex_digit[value&15];
			value>>=4;
		}
		pbuf+=4;
		*pbuf++=':';
		*pbuf++=' ';

		loop=0;
		while (loop<16)
		{
			if (loop==size) break;

			databyte=*(data+loop);
			value=databyte>>4;
			*pbuf++=hex_digit[value&15];
			*pbuf++=hex_digit[databyte&15];
			*pbuf++=' ';
			loop++;
		}
		while (loop<16)
		{
			*pbuf++=' ';
			*pbuf++=' ';
			*pbuf++=' ';
			loop++;
		}
		*pbuf++=' ';
		loop=0;
		while (loop<16)
		{
			if (loop==size) break;

			databyte=*(data+loop);
			databyte&= 127;
			if (databyte<32 || databyte>126) databyte='.';
			*pbuf++=databyte;
			loop++;
		}
		if (size>16) size-=16; else size=0;
		if (size) data+=16;

		*pbuf++='\n';
		*pbuf=0;
		line+=16;

		fputs((const char*)buf,_dbg_file_);
		fflush(_dbg_file_);
	}
}

#define STBASE_DEBUG_CON(name) {strcat(_DebugName,"/"); strcat(_DebugName,name); fprintf(_dbg_file_,"BASE CONSTRUCTED: %s\n",_DebugName);fflush(_dbg_file_);}
#define STBASE_DEBUG_OP(where,op,data,size) {fprintf(_dbg_file_,"BASE %s %s %s: %d\n",where,op,this->_DebugName,size); _STGLIB_Dump(data,size);}
#define STBASE_DEBUG_OP2(where,op,to) {fprintf(_dbg_file_,"BASE %s %s %s %s\n",where,this->_DebugName,op,to->_DebugName);fflush(_dbg_file_);}
#define STBASE_DEBUG_OPV(where,op,val) {fprintf(_dbg_file_,"BASE %s %s %s %d (0x%x)\n",where,this->_DebugName,op,val,val);fflush(_dbg_file_);}
#define STBASE_DEBUG_DES(name) {fprintf(_dbg_file_,"BASE DESTROYED: %s\n",name);fflush(_dbg_file_);}
#define STBASE_DEBUG_ERR(where) {fprintf(_dbg_file_,"BASE ERROR %s %s %s\n",where,this->_DebugName,_GetErrorStrPtr());fflush(_dbg_file_);}
#define STBASE_DEBUG_MSG(msg,arg) {fprintf(_dbg_file_,"%s%s\n",(const char *)(msg),(const char *)(arg));fflush(_dbg_file_);}
#else
#define STBASE_DEBUG_CON(a)
#define STBASE_DEBUG_OP(a,b,c,d)
#define STBASE_DEBUG_OP2(a,b,c)
#define STBASE_DEBUG_OPV(a,b,c)
#define STBASE_DEBUG_DES(a)
#define STBASE_DEBUG_ERR(a)
#define STBASE_DEBUG_MSG(a,b)
#endif


#include "/src/stglib/stcore.h"
#include "/src/stglib/sterror.h"
#include "/src/stglib/starray.h"


// New concept: base "types"
// pipe = continuous stream, rhs returned from >>
// buffer = expandable memory allocation, individual writes accumulate, invidual reads stream buffer
// struct = read/write only fixed size, otherwise error
// packet = short buffers in successsion


//////////////////////////////////////////////////////////////////////
//  StBase - generic i/o & error handling base class
//////////////////////////////////////////////////////////////////////
// base class
class StBase //:public StErBase
{
protected:
	// this is a pointer to another base class, so that the error string
	// can be read/written using stglib style methods
	StBase *_ErrorString;
	StBase *_Notify;
	StBase *_PreRead;

public:


#ifdef STBASE_DEBUG
	char _DebugName[256];
#endif
	
	StError _Error;

	StBase()
	{
		STGLIB_CON("StBase");
		_ErrorString=_Notify=NULL;
		_PreRead=NULL;
		_Error=StErr_NONE;

#ifdef STBASE_DEBUG
		strcpy(_DebugName,"/");
#endif
	
	}
/*
	StBase(const char *name)
	{
		STGLIB_CON("StBase");
		_ErrorString=_Notify=NULL;
		_Error=StErr_NONE;

#ifdef STBASE_DEBUG
		strcpy(_DebugName,"/");
		strcat(_DebugName,name);
#endif
	}
*/
#ifdef STBASE_DEBUG
	void _SetDebugName(const char *name)
	{
		STBASE_DEBUG_CON(name);
	}
#endif

	virtual ~StBase()
	{
		STGLIB_DES("StBase");
		if (_ErrorString)
		{
			STBASE_DEBUG_OPV("~StBase","delete _ErrorString",_ErrorString);
			delete _ErrorString;
		}
		// %%% Need to do something about notify here!!!
		if (_PreRead)
		{
			delete _PreRead;
		}
	}

	// Notify Changed mechanism needs to be updated
	// needs to maintain a list of objects (notified & changed)
	// and have entries deleted at object destruction!
	void _AddNotify(StBase *me)
	{
		this->_Notify=me;
	}
	virtual void _NotifyChanged(StBase *changed)
	{
		// someone called us to say something changed
	}
	
	// ### error handling ###

	// allocate error string (defined outside, uses StString class)
	StBase &_GetErrorString(void);

	// same thing only returns *char (prb should avoid using)
	const char *_GetErrorStrPtr(void);

	// write error string to path, return exit (error) value
	int _ShowError(StBase &out)
	{
		out<<_GetErrorString()<<"\n";
		return((int)_Error);
	}

	// write error string to path, and exit
	int _ShowErrorExit(StBase &out)
	{
		out<<_GetErrorString()<<"\n";
		exit(_Error);
	}
	void _Die(const char *msg)
	{
		write(2,msg,strlen(msg));
		write(2,": ",2);
		//write(2,_GetErrorString(),strlen(_GetErrorString()));
		write(2,"\n\r",2);
		exit(1);
	}

	// return descriptive string based on error code
	const char *_ErrorDescription(void)
	{
		// vc2005 says _sys_nerr is deprecated
		const char *desc=0;
/*		if (_Error<_sys_nerr)
			desc=_sys_errlist[_Error];
		else
*/		{
			if (_Error>StErr_STGLIB_DEFINED && _Error<StErr_LAST_DEFINED)
				desc=_STGLIB_Error[_Error-StErr_STGLIB_DEFINED];
		}
		if (!desc) desc="Unknown";
		return(desc);
	}
	void _ResetError(void)
	{
		_Error=StErr_NONE;
		if (_ErrorString)
		{
			delete _ErrorString;
			_ErrorString=0;
		}
	}

	// create new error
	StSize _Err(StError code,const char *Where=0,const char *Function=0);
	// copy error from another source
	StSize _Err(StBase &Source);
	// create error from stdlib's errno variable
	StSize _Errno(const char *Where=0,const char *Function=0);

/* define this later
	{
		_Error=code;
		StBase &msg=_GetErrorString();

		!msg;
		if (Where)
			msg<<Where<<": ";
		if (Function)
			msg<<Function<<": ";

		msg<<" "<<_ErrorDescription()<<" ("<<_Error<<")";

		return(StErr_RETURN);
	}

*/
	virtual StByte *_GetPointer(void)
	{
		// this should *NEVER* be called!
		// force an error here, rather than later when the pointer is dereferenced
		throw "undefined use of GetPointer";
		return(0);
	}

	// reset call (erase current contents)
	virtual void _Reset(void)
	{
		// do nothing by default
		return;
	}


	// low-level read/write calls (minimum necessary implemented)
	virtual StSize _Write(const StByte *data,StSize size)
	{
		data;
		size;
		return(_Err(StErr_NotImplemented,"_Write"));
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		data;
		size;
		return(_Err(StErr_NotImplemented,"_Read"));
	}

	// add data to beggining of buffer
	virtual StSize _Insert(const StByte *data,StSize size)
	{
		data;
		size;
		return(_Err(StErr_NotImplemented,"_Insert"));
	}
	// read data which is removed from start of buffer (like read_and_delete) 
	virtual StSize _Remove(StByte *data,StSize size)
	{
		data;
		size;
		return(_Err(StErr_NotImplemented,"_Remove"));
	}


	// push data back to class for later re-reading
	StSize _UnRead(StByte *data,StSize size); // defined after string/buffer loaded

	// get data from 
	StSize _ReRead(StByte *data,StSize size)
	{
		if (!_PreRead) return(0); // this should never happen
		StSize got=_PreRead->_Remove(data,size);
		if (!~(*_PreRead))
		{
			delete _PreRead;
			_PreRead=NULL;
		}
		return(got);
	}


	// return number of bytes currently available to read (if known)
	virtual StSize _Available(void)
	{
		STBASE_DEBUG_OP("StBase","AVAILABLE (default)",0,0);
		// for classes that don't define this, return zero
		return(0);
	}

	// higher level read/write calls to transfer data between classes
	virtual StSize _ReadFrom(StBase &Source)
	{
/*		StdError<<"StBase: WARNING: class not implementing _Read(base) and may fail\n";
		StByte buf[2048];
		StSize got=Source._Read(buf,sizeof(buf));
		if (Source._Error)
			return(_Err(Source));
		return(_Write(buf,got));
*/
		// derived class should declare how to read from another class, if possible
		return(_Err(StErr_NotImplemented,"_ReadFrom"));
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		// this class didn't define how to write to another, 
		// so ask the destination class to read from me
		StSize got=Destination._ReadFrom(*this);
		if (Destination._Error)
			return(_Err(Destination));
		return(got);

		// derived class should declare how to write to another class, if possible
//		return(_Err(StErr_NotImplemented,"_WriteTo"));
	}

	virtual bool operator==(StBase &match)
	{
		// this is a totally quick-n-dirty default routine that should be replaced
		// must be replaced if reading object affects data (like file or pipe)
		// presume that there would be a difference within 2048 bytes
		StByte left[2048];
		StByte right[2048];
		StSize left_got=_Read(left,sizeof(left));
		if (left_got==sizeof(left))
			return(false);	// assume difference past 2047 bytes
		StSize right_got=match._Read(right,sizeof(right));
		if (left_got!=right_got)
			return(false);

		return(memcmp(left,right,left_got)==0);
	}
	virtual bool operator!=(StBase &match)
	{
		return(!(operator==(match)));
	}
	virtual bool operator<(StBase &match)
	{
		StByte left[2048];
		StByte right[2048];
		StSize left_got=_Read(left,sizeof(left));
		if (left_got==sizeof(left))
			return(false);	// assume difference past 2047 bytes
		StSize right_got=match._Read(right,sizeof(right));
		if (left_got!=right_got)
			return(false);

		return(memcmp(left,right,left_got)<0);
	}
	virtual bool operator>(StBase &match)
	{
		StByte left[2048];
		StByte right[2048];
		StSize left_got=_Read(left,sizeof(left));
		if (left_got==sizeof(left))
			return(false);	// assume difference past 2047 bytes
		StSize right_got=match._Read(right,sizeof(right));
		if (left_got!=right_got)
			return(false);

		return(memcmp(left,right,left_got)>0);
	}



	// start/stop read/write calls to allow setup/teardown around single operation
	virtual inline void _ReadStart(StBase &Source)
	{
		Source;
	}
	virtual inline void _ReadStop(StBase &Source)
	{
		Source;
	}
	virtual inline void _WriteStart(StBase &Destination)
	{
		Destination;
	}
	virtual inline void _WriteStop(StBase &Destination)
	{
		Destination;
	}

	// if defined, rhs of operation is returned, not lhs
	// permitting operations in the middle to act like a
	// pipe or filter, handing off data to next instead
	// of all operations affecting only first term.
	inline virtual char _IsPipe(void)
	{
		return(0);
	}

	// if defined, buffer will be reset before each read
	inline virtual char _IsPacket(void)
	{
		return(0);
	}

	// if defined, multiple reads until zero length will occur
	inline virtual char _IsStream(void)
	{
		return(0);
	}

	// map << and >> operators to perform i/o


	// StG 2008-07-25: new transfer method:
	// _WriteTo is now pure virtual, must be defined
	// _Transfer asks only for _WriteTo, which may ask
	// destination class to _ReadFrom(it)

	inline virtual StSize _Transfer(StBase *from,StBase *dest)
	{
		StSize got=0;

		dest->_ReadStart(*from);
		from->_WriteStart(*dest);

		dest->_Error=StErr_NONE;
		from->_Error=StErr_NONE;

		// new 2007-03-15 StG
		// if from is a pipe, prefer to have it write to next dest
//		if (from->_IsPipe())
//		{
			// try the from writing dest
			got=from->_WriteTo(*dest);
			if (from->_Error==StErr_NotImplemented)
			{
				from->_Err(StErr_NotPossible,"StBase","Not Possible");
				dest->_Err(*from);
				STBASE_DEBUG_ERR("_Transfer");
				throw "StBase Transfer Failed";
			}
//		}
//		else
//		{
//			got=dest->_ReadFrom(*from);
//			if (dest->_Error==StErr_NotImplemented)
//			{
//				got=0;
//				dest->_Error=StErr_NONE;
//
//				// try the from writing dest
//				got=from->_WriteTo(*dest);
//				if (from->_Error==StErr_NotImplemented)
//				{
//					from->_Err(StErr_NotPossible,"StBase","Not Possible");
//					dest->_Err(*from);
//					STBASE_DEBUG_ERR("_Transfer");
//					throw "StBase Transfer Failed";
//				}
//				if (from->_Error)
//					dest->_Err(*from);
//			}
//		}
		if (dest->_Error && !from->_Error)
			from->_Err(*dest);

		dest->_ReadStop(*from);
		from->_WriteStop(*dest);

		return(got);
	}
	inline virtual StBase& operator<< (StBase &Source)
	{
		STBASE_DEBUG_OP2("StBase","<<",(&Source));

		if (Source._IsPipe())
		{
			throw "operator<< used on pipe enabled class";
			// must do obj>>pipe>>obj instead of obj<<pipe<<obj
		}

		_Transfer(&Source,this);

//		if (Source._IsPipe())
//			return(Source);
		return(*this);
	}
	inline virtual StBase& operator>> (StBase &that)
	{
		STBASE_DEBUG_OP2("StBase",">>",(&that));

		_Transfer(this,&that);

		if (that._IsPipe())
			return(that);
		return(*this);
	}

	// map ! operator to reset
	inline StBase& operator! ()
	{
		_Reset();
		return(*this);
	}

	// map ~ operator to bytes available
	inline StSize operator~ (void)
	{
		return(_Available());
	}

	// allows write of objects to different instances
	// of 'that' class for each recursion level
	virtual StBase& _RecurseLevel(StBase &box)
	{
		box;
		return(*this);
	}

	// string-like handling moved from StString

	// provide ability to write standard c strings
	StBase& operator<< (const char *cstring)
	{
		if (cstring)
		{
			const char *pend=cstring;
			while (*pend)
				pend++;
			STBASE_DEBUG_OP("StBase","WRITE",(StByte*)cstring,pend-cstring);
			_Write((StByte*)cstring,(StSize)(pend-cstring));
		}
		return(*this);
	}

	StBase& _Dec(const unsigned long value,int digits)
	{
		if (digits>1)
		{
			(*this)._Dec(value/10,digits-1);
		}
		{
			StByte ch='0'+(StByte)(value%10);
			_Write(&ch,1);
		}
		return(*this);
	}

	StBase& operator<< (const unsigned long value)
	{
		if (value>9)
		{
			(*this)<<(value/10);
		}
//		_Write((StByte)('0'+value%10));
		{
			StByte ch='0'+(StByte)(value%10);
			STBASE_DEBUG_OP("StBase","WRITE",&ch,1);
			_Write(&ch,1);
		}
		return(*this);
	}
	StBase& operator<< (const unsigned int value)
	{
		StByte ch;

		if (value>9)
		{
			(*this)<<(value/10);
		}
		ch='0'+value%10;
//		_Write((StByte)('0'+value%10));
		STBASE_DEBUG_OP("StBase","WRITE",&ch,1);
		_Write(&ch,1);
		return(*this);
	}
	StBase& operator<< (int value)
	{
		if (value<0)
		{
			STBASE_DEBUG_OP("StBase","WRITE",(StByte*)"_",1);
			_Write((StByte*)"-",1);
			value=-value;
		}

		if (value>9)
		{
			(*this)<<(value/10);
		}
//		_Write('0'+value%10);
		{
			StByte ch='0'+value%10;
			STBASE_DEBUG_OP("StBase","WRITE",&ch,1);
			_Write(&ch,1);
		}
		return(*this);
	}
	StBase& operator<< (long value)
	{
		if (value<0)
		{
			STBASE_DEBUG_OP("StBase","WRITE",(StByte*)"_",1);
			_Write((StByte*)"-",1);
			value=-value;
		}

		if (value>9)
		{
			(*this)<<(value/10);
		}
		StByte ch='0'+(StByte)(value%10);
		STBASE_DEBUG_OP("StBase","WRITE",&ch,1);
		_Write(&ch,1);
		return(*this);
	}
	StBase& operator>> (int &value)
	{
		value=0;
		char buf[32];

		StSize got=_Read((StByte*)buf,31);
		if (got>=0)
			buf[got]=0;

		value=atoi(buf);
		return(*this);
	}
	StBase& operator>> (long &value)
	{
		value=0;
		char buf[32];

		StSize got=_Read((StByte*)buf,31);
		if (got>=0)
			buf[got]=0;

		value=atol(buf);
		return(*this);
	}

};

// ##################################### CODE BELOW REMOVED - BASE TRANSFER NOW RESULTS IN ERROR
#ifdef BOGUS



//////////////////////////////////////////////////////////////////////
//  StPipe - included from another file
//////////////////////////////////////////////////////////////////////

// include stpipe here, so that base is already defined
#include "/src/stglib/stpipe.h"


//////////////////////////////////////////////////////////////////////
//  _BaseTransfer
//////////////////////////////////////////////////////////////////////

// and now finally define the transfer function, using stpipe
StSize _BaseTransfer(StBase &This,StBase &Source,StBase &Destination)
{
	StSize Transferred=0;
	StSize Read;
	StSize Wrote;
	StPipe Pipe;

#ifdef STBASE_DEBUG
fprintf(_dbg_file_,"*** BASE TRANSFER: %s => %s\n",Source._DebugName,Destination._DebugName);
#endif

	while (1)
	{
		Read=Pipe._Read(Source);
		if (!Read)
			break;
		if (Pipe._Error)
			return(This._Err(Pipe));
		Wrote=Pipe._WriteTo(Destination);
		if (Pipe._Error)
			return(This._Err(Pipe));
		if (Read!=Wrote)
			return(This._Err(StErr_DataSizeFailure,"BaseTransfer","Pipe"));
		Transferred+=Wrote;
	}
#ifdef STBASE_DEBUG
fprintf(_dbg_file_,"*** BASE TRANSFER: complete, transfered %d bytes\n",Transferred);
#endif
	return(Transferred);
};

#endif
// #####################################




//////////////////////////////////////////////////////////////////////
//  StBaseServ - base class for StServer<> functionality
//////////////////////////////////////////////////////////////////////
class StBaseServ:public StBase
{
public:
	StBaseServ()
	{
		STGLIB_CON("StBaseServ");
	}

	virtual ~StBaseServ()
	{
		STGLIB_DES("StBaseServ");
	}

	// these would best be pure virtual, but use of StSock would fail...

	// asks acceptor thread to wait for incoming request
	inline virtual StBaseServ* _ServiceAccept(void)
	{
		return(0);
	}

	// Although these next two functions would be best pure virtual
	// to force the service instance to declare them, doing so would
	// make all non-server sockets to fail

	// creates new service from existing acceptor thread
	inline virtual StBaseServ* _ServiceCreate(void)
	{
		return(0);
	}

	// processes the actual service
	inline virtual void _ServiceProcess(void)
	{
	}
};

//////////////////////////////////////////////////////////////////////
//  StBaseData - base class for fixed structures transffered thru i/o
//////////////////////////////////////////////////////////////////////
class StBaseData:public StBase
{
private:
	StByte *_Data;
	StSize _Size;

public:
	StBaseData(StByte *dataptr,StSize datasize)
	{
		STGLIB_CON("StBaseData");
		STBASE_DEBUG_CON("StBaseData");
		_Data=dataptr;
		_Size=datasize;
	}
	StBaseData(StSize datasize)
	{
		STGLIB_CON("StBaseData");
		STBASE_DEBUG_CON("StBaseData");
		_Data=(StByte*)this;
		_Size=datasize;
	}
	virtual ~StBaseData()
	{
		STGLIB_DES("StBaseData");
		STBASE_DEBUG_DES("StBaseData");
	}

	virtual StSize _Write(const StByte *data,StSize size)
	{
		// add data to buffer
		if (!data) 
			return(0);

		if (size!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseData","_Write"));

		memcpy(_Data,data,size);
		return(size);
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (size>_Size)
			size=_Size;

		if (size)
		{
			memcpy(data,_Data,size);
		}
		return(size);
	}

	virtual StSize _ReadFrom(StBase &Source)
	{
		StSize Got=Source._Read((StByte*)_Data,_Size);
		if (Source._Error)
			return(_Err(Source));
		if (Got!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseData","_ReadFrom"));
		return(Got);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		STBASE_DEBUG_OP("StBaseData","WRITE",(StByte*)_Data,_Size);
		StSize Wrote=Destination._Write((StByte*)_Data,_Size);
		if (Destination._Error)
			return(_Err(Destination));
		if (Wrote!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseData","_WriteTo"));
		return(Wrote);
	}

	virtual StSize _Available(void)
	{
		STBASE_DEBUG_OP("StBase","AVAILABLE",0,_Size);
		return(_Size);
	}



};


//////////////////////////////////////////////////////////////////////
//  StBaseStatic - read only base class for fixed data string
//////////////////////////////////////////////////////////////////////
class StBaseStatic:public StBase
{
private:
	StByte *_Data;
	StSize _Size;

public:
	StBaseStatic(StByte *dataptr,StSize datasize)
	{
		STGLIB_CON("StBaseStatic");
		STBASE_DEBUG_CON("StBaseStatic");
		_Data=dataptr;
		_Size=datasize;
	}
	StBaseStatic(StSize datasize)
	{
		STGLIB_CON("StBaseStatic");
		STBASE_DEBUG_CON("StBaseStatic");
		_Data=(StByte*)this;
		_Size=datasize;
	}

	StBaseStatic()
	{
		STGLIB_CON("StBaseStatic");
		STBASE_DEBUG_CON("StBaseStatic");
		_Data=NULL;
		_Size=0;
	}
	virtual ~StBaseStatic()
	{
		STGLIB_DES("StBaseStatic");
		STBASE_DEBUG_DES("StBaseStatic");
	}

	inline StBaseStatic& operator()(StByte *ptr,StSize len)
	{
		_Data=ptr;
		_Size=len;
		return(*this);
	}

	virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_InvalidDataLength,"StBaseStatic","_Write"));
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (size>_Size)
			size=_Size;

		if (size)
		{
			memcpy(data,_Data,size);
		}
		return(size);
	}

	virtual StSize _ReadFrom(StBase &Source)
	{
/*		StSize Got=Source._Read((StByte*)_Data,_Size);
		if (Source._Error)
			return(_Err(Source));
		if (Got!=_Size)
*/			return(_Err(StErr_InvalidDataLength,"StBaseStatic","_ReadFrom"));
//		return(Got);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		STBASE_DEBUG_OP("StBaseStatic","WRITE",(StByte*)_Data,_Size);
		StSize Wrote=Destination._Write((StByte*)_Data,_Size);
		if (Destination._Error)
			return(_Err(Destination));
		if (Wrote!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseStatic","_Write"));
		return(Wrote);
	}

	virtual StSize _Available(void)
	{
		STBASE_DEBUG_OP("StBase","AVAILABLE",0,_Size);
		return(_Size);
	}



};

//////////////////////////////////////////////////////////////////////
//  StFileMap - file emulated by fixed i/o map
//////////////////////////////////////////////////////////////////////
class StFileMap:public StBase
{
protected:
	StByte *_Data;
	StSize _Size;
	StSize _Offset;	// offset to next byte(s) to read

public:
	StFileMap(StByte *dataptr,StSize datasize)
	{
		STGLIB_CON("StFileMap");
		_Data=dataptr;
		_Size=datasize;
		_Offset=0;
	}

	virtual StSize _Write(const StByte *data,StSize size)
	{
		return(_Err(StErr_ReadOnly,"StFileMap","_Write"));
	}
	virtual StSize _Read(StByte *data,StSize size)
	{
		if (_Offset+size>_Size)
		{
			size=_Size-_Offset;
		}

		if (size)
		{
			memcpy(data,_Data+_Offset,size);
			_Offset+=size;
		}
		return(size);
	}
/*
	virtual StSize _ReadFrom(StBase &Source)
	{
		StSize Got=Source._Read((StByte*)_Data,_Size);
		if (Source._Error)
			return(_Err(Source));
		if (Got!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseData","_ReadFrom"));
		return(Got);
	}

	virtual StSize _WriteTo(StBase &Destination)
	{
		StSize Wrote=Destination._Write((StByte*)_Data,_Size);
		if (Destination._Error)
			return(_Err(Destination));
		if (Wrote!=_Size)
			return(_Err(StErr_InvalidDataLength,"StBaseData","_Write"));
		return(Wrote);
	}
*/
	virtual StSize _Available(void)
	{
		STBASE_DEBUG_OP("StBase","AVAILABLE",0,_Size-_Offset);
		return(_Size-_Offset);
	}
	virtual void _Reset(void)
	{
		_Offset=0;
	}

};

// STG 2004: define global operator for >>

StBase& operator>> (const char *cstring, StBase &right)
{
	const char *pend=cstring;
	if (!pend) return(right);
	while (*pend)
		pend++;
	right._Write((StByte*)cstring,(StSize)(pend-cstring));

	return(right);
}

/*
StBase& operator<< (StBase &left, StBase &right)
{
	_BaseTransfer(left,right,left);

	return(left);
}
*/

/*
StBase& operator<< (StBase &left, const char *cstring)
{
	const char *pend=cstring;
	if (!pend) return(left);
	while (*pend)
		pend++;
	left._Write((StByte*)cstring,pend-cstring);

	return(left);
}
*/

// need StString
#include "/src/stglib/ststring.h"

// STGLIB 2004: declare error handlers here

// create new error
StSize StBase::_Err(StError code,const char *Where,const char *Function)
{
	_Error=code;

	// avoid creating error string for non-implemented functions
	if (_Error==StErr_NotImplemented)
		return(StErr_RETURN);

	if (!_ErrorString)
	{
		_ErrorString=new StString;
		(*_ErrorString)<<"Bogus";
		StBase *temp=_ErrorString;
		_ErrorString=new StString;
		delete temp;
		STBASE_DEBUG_OPV("_Err","new _ErrorString",_ErrorString);
	}

	StString &msg=*(StString*)_ErrorString;

	!msg;
	if (Where)
		msg<<Where<<": ";
	if (Function)
		msg<<Function<<": ";

	(*(StString*)_ErrorString)<<" "<<_ErrorDescription()<<" ("<<_Error<<")";

	STBASE_DEBUG_ERR("_Err(code)");

	return(StErr_RETURN);
}

// copy error from another instance
StSize StBase::_Err(StBase &Source)
{
	_Error=Source._Error;

	// avoid creating error string for non-implemented functions
	if (_Error==StErr_NotImplemented)
		return(StErr_RETURN);

	if (!_ErrorString)
	{
		_ErrorString=new StString;
		STBASE_DEBUG_OPV("_Err","new _ErrorString",_ErrorString);
	}

	StString &msg=*(StString*)_ErrorString;
	
	// this should make a local copy of the string, not a ptr to same
	msg=*(StString*)Source._ErrorString;

	STBASE_DEBUG_ERR("_Err(&src)");

	return(StErr_RETURN);
}

StSize StBase::_Errno(const char *Where,const char *Function)
{
	// take the C std errno and 
	// set _Error and msg

	if (!_ErrorString)
	{
		_ErrorString=new StString;
		STBASE_DEBUG_OPV("_Err","new _ErrorString",_ErrorString);
	}

	StString &msg=*(StString*)_ErrorString;
	
	!msg;
	if (Where)
		msg<<Where<<": ";
	if (Function)
		msg<<Function<<": ";

	
	if (errno<_sys_nerr)
		msg<<_sys_errlist[errno];
	else
		msg<<"Unknown code: "<<errno;

	_Error=(StError)errno;

	if (_Error>=StErr_LAST_DEFINED)
		msg<<" NODEF:"<<_Error;

	STBASE_DEBUG_ERR("_Errno()");

	return(StErr_RETURN);
}

StBase &StBase::_GetErrorString(void)
{
	if (!_ErrorString)
		_Err(StErr_NONE,"NO ERROR");
	return(*_ErrorString);
}
const char *StBase::_GetErrorStrPtr(void)
{
	if (!_ErrorString)
		return("<none>");
	return((const char *)(_ErrorString->_GetPointer()));
}

StSize StBase::_UnRead(StByte *data,StSize size)
{
	if (!_PreRead)
		_PreRead=new StBuffer;

	// have to insert, since what was read may have been partial from existing _PreRead
	_PreRead->_Insert(data,size);
//	_PreRead->_Write(data,size);
	return(size);
}

#endif
