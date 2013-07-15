// STGLIB/stfile.h
// Copyright 1999 by Scott Griepentrog & StG Net

// file class

// operates standard io files using stbase method
// is basically a wrapper of fopen/fread/fwrite/etc for now, however,
// it always operates in binary mode (no text processing as on MSDOS)

/* EXAMPLE CODE:

	// READING
	StFile test1("test1.txt");
	test1>>StdOutput;
	if (test1._Error)
	{
		StdOutput<<test1._ErMsg;
	}


	// WRITING
	StFile test2("test2.txt");
	test2<<"Test";
	if (test2._Error)
	{
		StdOutput<<test2._ErMsg;
	}
*/

#ifndef STGLIB_STFILE
#define STGLIB_STFILE

#pragma message("using stfile.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stbase.h"
#include "/src/stglib/ststring.h"

//////////////////////////////////////////////////////////////////////
// StFile - access disk files
//////////////////////////////////////////////////////////////////////

enum StFileOpenMethod
{
	StFileOpenDefault=0,		// used only to detect default parameter
	StFileOpenReadOnlyExisting,	// RO, error if file doesn't exist
	StFileOpenExistingOrFail,	// RW, error if file doesn't exist
	StFileOpenExistingOrCreate,	// RW, create file if doesn't exist
	StFileOpenCreateOrAppend,	// RW, append existing or create new
	StFileOpenCreateOrFail,		// RW, create file or error if exists

	// popen
	StFileOpenAppOutput,		// RO from output of application given as filename
	StFileOpenAppInput,			// WO to input of application given as filename

	// dups for convenience
	StFileOpenReadOnly=		StFileOpenReadOnlyExisting,
	StFileOpenExisting=		StFileOpenExistingOrFail,
	StFileOpenCreateOrExisting=	StFileOpenExistingOrCreate,
	StFileOpenAppend=		StFileOpenCreateOrAppend,
	StFileOpenCreate=		StFileOpenCreateOrFail,
	StFileOpenOverwrite=		StFileOpenExistingOrCreate,
	StFileOpenNormal=		StFileOpenExistingOrCreate,


};

class StFile:public StBase
{
// inherited from StBase:
//	StError _Error;
//	StString _ErMsg;
protected:
	FILE *_FileHandle;
	char _HandleSupplied;
//	char _EndOfFile;	// change this to returning StErr_EndOfFile
	char _Ascii;			// not really used in stglib - files assumed to be binary
	StFileOpenMethod _Method;

	void StFileInit()
	{
		_FileHandle=NULL;
		_HandleSupplied=0;
//		_EndOfFile=0;
		_Ascii=0;
		_Method=StFileOpenReadOnlyExisting;
		_FlushAfterWrite=0;
	}

public:
	StString _FilePath;
	int _FlushAfterWrite;

	StFile()
	{
		STGLIB_CON("StFile");
		STBASE_DEBUG_CON("StFile");
		StFileInit();
	}
/*	StFile(unsigned index)
	{
		STGLIB_CON("StFile");
		STBASE_DEBUG_CON("StFile");
		StFileInit();
	}
*/
	StFile(const char *path,
		StFileOpenMethod method=StFileOpenReadOnlyExisting)
	{
		STGLIB_CON("StFile");
		STBASE_DEBUG_CON("StFile");
		STBASE_DEBUG_CON(path);
		StFileInit();
		!_FilePath<<path;
		_Method=method;
	}
	StFile(StString &path,
		StFileOpenMethod method=StFileOpenReadOnlyExisting)
	{
		STGLIB_CON("StFile");
		STBASE_DEBUG_CON("StFile");
		STBASE_DEBUG_CON((const char *)path);
		StFileInit();
		!_FilePath<<path;
		_Method=method;
	}
	StFile(FILE *fp,
		StFileOpenMethod method=StFileOpenReadOnlyExisting)
	{
		STGLIB_CON("StFile");
#ifdef STBASE_DEBUG
		if (fp==stdin)
			strcpy(_DebugName,"<stdin>");
		else if (fp==stdout)
			strcpy(_DebugName,"<stdout>");
		else if (fp==stderr)
			strcpy(_DebugName,"<stderr>");
		else
			strcpy(_DebugName,"<handle>");
#endif

		StFileInit();
		_HandleSupplied=1;
		_FileHandle=fp;
		_Method=method;
		!_FilePath<<"<handle>";

		// okay, this is so totally a hack...
//		if (_FileHandle==stdin)
//			_SetNonBlocking();

	}

	virtual ~StFile()
	{
		STGLIB_DES("StFile");
		if (_FileHandle && !_HandleSupplied)
			fclose(_FileHandle);
	}

/*	void _SetNonBlocking(void)
	{
#ifndef WIN32
		STBASE_DEBUG_OP2("StFile","SetNonBlocking",this);
		fcntl(fileno(_FileHandle), F_SETFL, O_NONBLOCK|fcntl(fileno(_FileHandle),F_GETFL)); 
#endif
	}
*/

	void _SetAscii(void)
	{
		_Ascii=1;
	}


	void _FileClose(void)
	{
		STBASE_DEBUG_OP2("StFile","FileClose",this);
//		_EndOfFile=0;
		if (_HandleSupplied)
		{
			// presume that we can't ourselves close the handle,
			// but that the class is to be reused on another file
			// aka freopen(stdin,"file")
			_HandleSupplied=0;
		}
		else
		{
			if (_FileHandle)
				fclose(_FileHandle);
		}
		_FileHandle=NULL;
	}
	StSize _FileOpen(const char *path=0,
		StFileOpenMethod method=StFileOpenDefault)
	{
		STBASE_DEBUG_OP2("StFile","FileOpen",this);
		// insure that anything that was open is now closed
		_FileClose();
		_ResetError();

		// set new path if given
		if (path)
			!_FilePath<<path;

		// and new method if supplied
		if (method)
			_Method=method;

		if (_Method==StFileOpenAppOutput)
		{
			_FileHandle=popen(_FilePath,"r");
			if (!_FileHandle)
				return(_Errno("StFileOpen","popen"));
			return(StErr_NONE);
		}
		if (_Method==StFileOpenAppInput)
		{
			_FileHandle=popen(_FilePath,"w");
			if (!_FileHandle)
				return(_Errno("StFileOpen","popen"));
			return(StErr_NONE);
		}

		// check existing (at least readable)
		char mode[4];
		strcpy(mode,_Ascii?"r":"rb");
		FILE *existing=fopen(_FilePath,mode);

		// set the fopen mode
		switch (_Method)
		{
		case StFileOpenReadOnlyExisting:	// RO, error if file doesn't exist
			if (!existing)
				return(_Errno("StFileOpen","fopen"));
			_FileHandle=existing;
			return(StErr_NONE);
			break;
		case StFileOpenExistingOrFail:		// RW, error if file doesn't exist
			if (!existing)
				return(_Errno("StFileOpen","fopen"));
		case StFileOpenExistingOrCreate:	// RW, create file if doesn't exist
			strcpy(mode,"w");
			break;
		case StFileOpenCreateOrAppend:		// RW, append existing or create new
			strcpy(mode,"a+");
			break;
		case StFileOpenCreateOrFail:		// RW, create file or error if exists
			if (existing)
			{
				fclose(existing);
				return(_Err(StErr_FileExists,"StFile","_FileOpen-CreateOrFail"));
			}
			break;
		default:
			return(_Err(StErr_NotPermitted,"StFile","_FileOpen no method!?"));
		}
		if (existing)
			fclose(existing);

		if (!_Ascii)
			strcat(mode,"b");

		_FileHandle=fopen(_FilePath,mode);
		if (!_FileHandle)
			return(_Errno("StFileOpen","fopen"));

		return(StErr_NONE);
	}
	void operator()(const char *path,
		StFileOpenMethod method=StFileOpenDefault)
	{
		_FileOpen(path,method);
	}

	StSize _Available(void)
	{
		STBASE_DEBUG_OP2("StFile","Available",this);
		if (!_FileHandle)
			if (_FileOpen())
				return(0);

		StSize current=ftell(_FileHandle);
		if (current==(-1))
			return(0);
		fseek(_FileHandle,0L,2);
		StSize end=ftell(_FileHandle);
		fseek(_FileHandle,(long)current,0);
		return(end-current);
	}

	StSize _FileSize(void)
	{
		STBASE_DEBUG_OP2("StFile","FileSize",this);
		if (!_FileHandle)
			if (_FileOpen())
				return(0);

		StSize current=ftell(_FileHandle);
		fseek(_FileHandle,0L,2);
		StSize end=ftell(_FileHandle);
		fseek(_FileHandle,(long)current,0);
		return(end);
	}

	StSize _Write(const StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StFile","WRITE",data,size);
		if (_Method==StFileOpenReadOnlyExisting)
		{
			if (!_FileHandle)
				_Method=StFileOpenNormal;
			else
				return(_Err(StErr_ReadOnly,"StFile","_Write - file opened readonly"));
		}

		if (!_FileHandle)
			if (_FileOpen())
				return(0);

		StSize put=(StSize)fwrite(data,1,size,_FileHandle);

		if (put!=size)
		{
			if (ferror(_FileHandle))
				return(_Errno("StFile","fwrite"));
		}
		if (_FlushAfterWrite)
			fflush(_FileHandle);
		return(put);
	}
	StSize _Read(StByte *data,StSize size)
	{
		STBASE_DEBUG_OP2("StFile","READing",this);
		if (!_FileHandle)
			if (_FileOpen())
				return(0);

		STBASE_DEBUG_OP2("StFile","READing2",this);
//		if (_EndOfFile)
//		{
//		STBASE_DEBUG_OP2("StFile","READing EOF",this);
//			// return end of file once
//			_EndOfFile=0;
//			return(0);
//		}

//		if (StFileIsTerm(_FileHandle))
//			if (size>1)
//				size=1;

		if (_FileHandle==stdin && size>1) size=1;

		STBASE_DEBUG_OP("StFile","PRE-READ",(StByte*)&size,sizeof(size));


		// okay, two ways to do input
		// both take in data/size and return got
		StSize got=0;

#ifdef WIN32
		// windows way
		{
			// use ReadFile instead of fread because
			// it returns partial read (what's available)

			DWORD BytesRead=0;
			HANDLE handle=(HANDLE)_get_osfhandle(_fileno(_FileHandle)); // ugly
			if (!ReadFile(handle,data,(DWORD)size,&BytesRead,NULL))
			{
				// an error occured
				return(_Errno("StFile","ReadFile"));
			}
			else if (!BytesRead)
			{
				//errno=StErr_EndOfFile;
//				_EndOfFile=1;
			}

			got=BytesRead;
		}

#else
		// linux/posix way 
		while (1)
		{
			errno=0;
			got=(StSize)fread(data,1,size,_FileHandle);
	STBASE_DEBUG_OP("StFile","post fread got",(StByte*)&got,sizeof(got));
	STBASE_DEBUG_OP("StFile","post fread errno",(StByte*)&errno,sizeof(errno));
			if (got)
				break;

			if (!errno || feof(_FileHandle))
			{
				// end of file!
				//errno=StErr_EndOfFile;
//				_EndOfFile=1;
				break;
			}
			
			if (errno!=EAGAIN)
				break;

			// non blocking has been turned on,
			// and there is currently no input available
			// so block on input for a sec
			struct timeval tv={1,0};
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(fileno(_FileHandle),&fds);
			select(fileno(_FileHandle)+1,&fds,NULL,NULL,&tv);
		}
/*		if (got!=size)
		{
			if (errno)
				return(_Errno("StFile","fread"));

			// is it end of file?
			if (feof(_FileHandle))
			{
				// yup
//				_EndOfFile=1;
				// but return was was read!
				return(got);
			}
		}
*/
	STBASE_DEBUG_OP("StFile","done errno",(StByte*)&errno,sizeof(errno));
		if (errno && errno!=EAGAIN)
			return(_Errno("StFile","fread"));
#endif


		STBASE_DEBUG_OP("StFile","READ",data,got);

		return(got);
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		STBASE_DEBUG_OP2("StFile","WriteTo",this);
		// The target destination class does not care to read bits and pieces from the file,
		// or random access it or anything.  It wants the whole file dumped to it.
		// So grab a large chunk of the file and send it.

		const StSize buffer_size=4096;
		StBuffer buffer;


		buffer._ArrayAllocate(buffer_size);
		StByte *data=buffer._GetPointer();
		StSize total=0;

		while (!_Error) //!_EndOfFile)
		{
			StSize got=_Read(data,buffer_size);
			if (!got)
				break;
			if (_Error) 
				return(StErr_RETURN);
			total+=Destination._Write(data,got);
		}
		return(total);

//		return(_Err(StErr_NotImplemented,"_WriteTo"));
	}

	StSize _FileRewind(void)
	{
		STBASE_DEBUG_OP2("StFile","FileRewind",this);
//		_EndOfFile=0;
		if (!_FileHandle)
			return(_Err(StErr_BadFileHandle,"StError::_FileRewind"));

		if (fseek(_FileHandle,0L,SEEK_SET))
			return(_Errno("StFile","fseek"));

		return(0);
	}
	StSize _FileSeek(StSize position)
	{
		STBASE_DEBUG_OP2("StFile","FileSeek",this);
//		_EndOfFile=0;
		if (!_FileHandle)
			return(_Err(StErr_BadFileHandle,"StError::_FileSeek"));

		if (fseek(_FileHandle,(long)position,SEEK_SET))
			return(_Errno("StFile","fseek"));

		return(0);
	}


	StSize _FileErase(void)
	{
		STBASE_DEBUG_OP2("StFile","FileErase",this);
		if (_HandleSupplied)
			return(_Err(StErr_BadFileHandle,
				"StError::_FileErase","not supported on supplied handle"));

		if (_FileHandle)
			_FileClose();

		if (StFileDelete(_FilePath))
			return(_Errno("StFile","unlink"));

		return(StErr_NONE);
	}
	inline void operator!(void)
	{
		_FileErase();
	}


/*
	inline virtual StBase& operator<< (StBase &that)
	{
		return(StBase::operator<<(that));
	}
*/

};

#ifdef BOGUS_OLD_DONT_USE
class StStringFile:public StStringFunctions<StFile>
{
public:
	StStringFile()
	{
		STGLIB_CON("StStringFile");
	}
	StStringFile(FILE *fp)
	{
		STGLIB_CON("StStringFile");
		_FileOpenHandle(fp);
	}
	virtual ~StStringFile()
	{
		STGLIB_DES("StStringFile");
	}

/*	StStringFile& operator= (StStringFile& copyfrom)
	{
		StArray<StByte>::operator=(copyfrom);
		return(*this);
	}
*/

	/*
	inline virtual StBase& operator<< (StBase &that)
	{
		return(StBase::operator<<(that));
	}
	*/


};
#endif


#endif
