// STGLIB/stSerial.h
// Copyright 1999 by Scott Griepentrog & StG Net

// Serial class

// operates standard io Serials using stbase method

#ifndef STGLIB_STSERIAL
#define STGLIB_STSERIAL

#pragma message("using stserial.h")

#include "/stglib/stcore.h"
#include "/stglib/stbase.h"
#include "/stglib/ststring.h"

// must include the windows calls
#ifndef _INC_WINDOWS
#define STRICT
#include <windows.h>
#include <windowsx.h>
#endif

//////////////////////////////////////////////////////////////////////
// StSerial - windows com port handling
//////////////////////////////////////////////////////////////////////

enum StSerialParityType
{
	StSerial_NoParity=NOPARITY,
	StSerial_OddParity=ODDPARITY,
	StSerial_EvenParity=EVENPARITY,
	StSerial_MarkParity=MARKPARITY,
	StSerial_SpaceParity=SPACEPARITY,
};

class StSerial:public StBase
{
protected:
	HANDLE _SerialHandle;
	DCB _SerialDeviceControl;

	inline StSize _ErrGLE(char *Where=0,char *Function=0)
	{
		_Error=(StError)GetLastError();

		!_ErMsg;
		if (Where)
			_ErMsg<<Where<<": ";
		if (Function)
			_ErMsg<<Function<<": ";

		// ask windows what the error code means
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,_Error,MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
		_ErMsg<<(StChar*)lpMsgBuf;
		LocalFree(lpMsgBuf);
		
		return(StErr_RETURN);
	}


public:
	StSerial(char *device_name,int baud_rate,int bits,StSerialParityType parity,int stop_bits)
	{
		_SerialHandle=CreateFile(device_name,
			GENERIC_READ|GENERIC_WRITE,
			0, // not shared
			NULL, // no security attributes
			OPEN_EXISTING,
			0, // not overlapped
			NULL // no template
			);

		if (_SerialHandle==INVALID_HANDLE_VALUE)
		{
			_ErrGLE("StSerial","CreateFile");
			return;
		}

		if (!GetCommState(_SerialHandle,&_SerialDeviceControl))
		{
			_ErrGLE("StSerial","GetCommState");
			return;
		}
		_SerialDeviceControl.BaudRate=baud_rate;
		_SerialDeviceControl.ByteSize=bits;
		_SerialDeviceControl.Parity=parity;
		switch (stop_bits)
		{
		case 1: _SerialDeviceControl.StopBits=ONESTOPBIT; break;
		case 2: _SerialDeviceControl.StopBits=TWOSTOPBITS; break;
		}

		// always in binary mode
		_SerialDeviceControl.fBinary=1;

		// enable parity checking only if using parity mode
		_SerialDeviceControl.fParity=0;
		if (parity!=StSerial_NoParity)
			_SerialDeviceControl.fParity=1;

		// these settings are for dumb 3-wire mode - NO handshaking!
		_SerialDeviceControl.fOutxCtsFlow=0;
		_SerialDeviceControl.fOutxDsrFlow=0;
		_SerialDeviceControl.fDtrControl=DTR_CONTROL_ENABLE; // leave DSR on while port open
		_SerialDeviceControl.fDsrSensitivity=0;
		_SerialDeviceControl.fTXContinueOnXoff=0;
		_SerialDeviceControl.fOutX=0;
		_SerialDeviceControl.fInX=0;
		_SerialDeviceControl.fErrorChar=0;
		_SerialDeviceControl.fNull=0;
		_SerialDeviceControl.fRtsControl=RTS_CONTROL_ENABLE; // leave RTS on while port open
		_SerialDeviceControl.fAbortOnError=0;

		if (!SetCommState(_SerialHandle,&_SerialDeviceControl))
		{
			_ErrGLE("StSerial","SetCommState");
			return;
		}

	}

	~StSerial()
	{
		if (_SerialHandle!=INVALID_HANDLE_VALUE)
			CloseHandle(_SerialHandle);
	}

/*	StSize _Available(void)
	{
		if (!_SerialHandle)
			if (_SerialOpen())
				return(0);

		StSize current=ftell(_SerialHandle);
		fseek(_SerialHandle,0L,2);
		StSize end=ftell(_SerialHandle);
		fseek(_SerialHandle,current,0);
		return(end-current);
	}
*/
	StSize _Write(StByte *data,StSize size)
	{
		DWORD wrote_bytes;

		if (!WriteFile(_SerialHandle,(void*)data,(DWORD)size,&wrote_bytes,NULL))
			return(_ErrGLE("StSerial::Write","WriteFile"));

		return((StSize)wrote_bytes);
	}
	StSize _Read(StByte *data,StSize size)
	{
		DWORD read_bytes;

		if (!ReadFile(_SerialHandle,(void*)data,size,&read_bytes,NULL))
			return(_ErrGLE("StSerial::Read","ReadFile"));

		return((StSize)read_bytes);
	}
};



#endif
