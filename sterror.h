// STGLIB version2 (2004) changed this to be just a list of error codes and descriptions


// definition of error codes

// Each class derived from StBase includes 
// public variable StError _Error; which
// uses the following error code system

// NOTE: the *return* value from each function is
// type StSize, and will be 0 for StErr_NONE or
// -1 for StErr_RETURN, which is returned for any
// error code.  The StError type is *NOT* returned!



// for some strange reason, linux doesn't like to define these...
#ifdef linux
extern int _sys_nerr;
extern __const char *__const _sys_errlist[];
#endif


//#include "/stglib/ststring.h"

#ifndef STGLIB_STERROR
#define STGLIB_STERROR

#pragma message("using sterror.h")

//class StString;

enum StError
{
    /*  0              */  StErr_NONE=0,
    /*  1 EPERM        */  StErr_NotPermitted,
    /*  2 ENOENT       */  StErr_FileNotFound,
    /*  3 ESRCH        */  StErr_ProcessNotFound,
    /*  4 EINTR        */  StErr_Interrupted,
    /*  5 EIO          */  StErr_IOFailure,
    /*  6 ENXIO        */  StErr_MemoryNotFound,
    /*  7 E2BIG        */  StErr_Overflow,
    /*  8 ENOEXEC      */  StErr_NotExecutable,
    /*  9 EBADF        */  StErr_BadFileHandle,
    /* 10 ECHILD       */  StErr_NoChildren,
    /* 11 EAGAIN       */  StErr_NotYetAvailable,
    /* 12 ENOMEM       */  StErr_MemoryFull,
    /* 13 EACCES       */  StErr_PermissionDenied,
    /* 14 EFAULT       */  StErr_BadPointer,
    /* 15 ENOTBLK      */  StErr_NotBlockDevice,
    /* 16 EBUSY        */  StErr_Busy,
    /* 17 EEXIST       */  StErr_FileExists,
    /* 18 EXDEV        */  StErr_EXDEV,
    /* 19 ENODEV       */  StErr_DeviceNotFound,
    /* 20 ENOTDIR      */  StErr_NotDirectory,
    /* 21 EISDIR       */  StErr_IsDirectory,
    /* 22 EINVAL       */  StErr_InvalidArgument,
    /* 23 ENFILE       */  StErr_TooManyFilesSystem,
    /* 24 EMFILE       */  StErr_TooManyFiles,
    /* 25 ENOTTY       */  StErr_NotCharDevice,
    /* 26 ETXTBSY      */  StErr_FileBusy,
    /* 27 EFBIG        */  StErr_FileTooLarge,
    /* 28 ENOSPC       */  StErr_DeviceFull,
    /* 29 ESPIPE       */  StErr_CantSeek,
    /* 30 EROFS        */  StErr_ReadOnly,
    /* 31 EMLINK       */  StErr_TooManyLinks,
    /* 32 EPIPE        */  StErr_BrokenPipe,
    /* 33 EDOM         */  StErr_DomainError,
    /* 34 ERANGE       */  StErr_OutOfRange,
    /* 35 EUCLEAN      */  StErr_StructureFailure,
    /* 36 EDEADLK      */  StErr_DeadLock,
    /* 37 UNKNOWN      */  StErr_Unknown,
    /* 38 ENAMETOOLONG */  StErr_NameTooLong,
    /* 39 ENOLCK       */  StErr_LockTableFull,
    /* 40 ENOSYS       */  StErr_NotImplemented,
    /* 41 ENOTEMPTY    */  StErr_DirectoryNotEmpty,
    /* 42 EILSEQ       */  StErr_IllegalSequence,

	StErr_STGLIB_DEFINED=256,
	StErr_InvalidDataFormat,
	StErr_InvalidDataLength,
	StErr_CompiledStructureSizedWrong,
	StErr_EndOfFile,
	StErr_Timeout,
	StErr_DataSizeFailure,
	StErr_NotPossible,

	StErr_LAST_DEFINED
};

const char *_STGLIB_Error[]=
{
	"DEFINED?",				// StErr_STGLIB_DEFINED
	"Invalid Data Format",	// StErr_InvalidDataFormat
	"Invalid Data Length",	// StErr_InvalidDataLength
	"Wrong Sized Struct",	// StErr_CompiledStructureSizedWrong
	"End Of File",			// StErr_EndOfFile
	"Timeout",				// StErr_Timeout
	"Data Size Failure",	// StErr_DataSizeFailure
	"NotPossible",			// StErr_NotPossible
};

/* STGLIB version2 (2004) - removed StErBase, some functionality in StBase now
class StErBase
{
public:
	StError _Error;
	StString _ErMsg;

	StErBase()
	{
		_Error=StErr_NONE;
	}

	virtual ~StErBase()
	{
	}

	const char *_ErrorDescription(void)
	{
		if (_Error<_sys_nerr)
			return(_sys_errlist[_Error]);

		if (_Error>StErr_STGLIB_DEFINED && _Error<StErr_LAST_DEFINED)
			return(_STGLIB_Error[_Error-StErr_STGLIB_DEFINED]);

		return("Unknown");
	}

	// create new error
	StSize _Err(StError code,char *Where=0,char *Function=0)
	{
		_Error=code;

		!_ErMsg;
		if (Where)
			_ErMsg<<Where<<": ";
		if (Function)
			_ErMsg<<Function<<": ";

		_ErMsg<<" "<<_ErrorDescription()<<" ("<<_Error<<")";

		return(StErr_RETURN);
	}

	// copy error from another instance
	StSize _Err(StErBase &Source)
	{
		_Error=Source._Error;
		_ErMsg=Source._ErMsg;
		return(StErr_RETURN);
	}

	StSize _Errno(char *Where=0,char *Function=0)
	{
		// take the C std errno and 
		// set _Error and _ErMsg

		!_ErMsg;
		if (Where)
			_ErMsg<<Where<<": ";
		if (Function)
			_ErMsg<<Function<<": ";

		
		if (errno<_sys_nerr)
			_ErMsg<<_sys_errlist[errno];
		else
			_ErMsg<<"Unknown code: "<<errno;

		_Error=(StError)errno;

		if (_Error>=StErr_LAST_DEFINED)
			_ErMsg<<" NODEF:"<<_Error;

		return(StErr_RETURN);
	}


};
*/

#endif
