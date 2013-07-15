// STGLIB/stfilter.h
// Copyright 1999,2002 by Scott Griepentrog & StG Net

// filter classes

// StFilterTextLine - text cr/lf handling and line break on input


#ifndef STGLIB_STFILTER
#define STGLIB_STFILTER

#pragma message("using stfilter.h")

#include "/src/stglib/stcore.h"
#include "/src/stglib/stbase.h"
#include "/src/stglib/stpipe.h"

//////////////////////////////////////////////////////////////////////
// StFilterText - filter for text mode & line break handling
//////////////////////////////////////////////////////////////////////

class StFilterTextLine:public StBase
{
	friend class StHttp;
	StPipe InputBuffer;
	StByte newline;
	StByte eof;
public:
	StBase *_FilterBase;

	StFilterTextLine(StBase &filtering)
	{
		_FilterBase=&filtering;
		newline=0;
		eof=0;
	}
	StFilterTextLine()
	{
		// note: using this requires later definition of base!!!
		_FilterBase=NULL;
		newline=0;
		eof=0;
	}
	void operator()(StBase &filtering)
	{
		_FilterBase=&filtering;
	}

	virtual ~StFilterTextLine()
	{
	}

	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	StSize _Write(const StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StFilterTextLine","WRITE",data,size);
		// is there a '\n' in the data being written?
		const StByte *scan=data;
		const StByte *end=data+size;
		StSize written=0;
		while (1)
		{
			while (scan<end)
			{
				if (*scan=='\n')
					break;
				scan++;
			}
			StSize chunk=(StSize)(scan-data);
			if (chunk)
				_FilterBase->_Write(data,chunk);
			written+=chunk;
			if (scan==end)
				return(written);

			// write CRLF...
			_FilterBase->_Write((StByte*)"\x0D\x0A",2);
			written+=2;
			data=++scan;
		}
	}

	StSize _WriteTo(StBase &Destination)
	{
		STBASE_DEBUG_OP2("StFilterTextLine","WriteTo",this);
		StSize sent=0;
		while (1)
		{
		STBASE_DEBUG_OP2("StFilterTextLine","Loop1",this);
			StByte *start=InputBuffer;
			StByte *scan=start;
			StByte *end=scan+~InputBuffer;

			while (scan<end)
			{
		STBASE_DEBUG_OP2("StFilterTextLine","Loop2",this);
				if (*scan!=0x0D && *scan!=0x0A)
				{
					// not crlf
//					if (sent<size)
					{
						Destination._Write(scan,1);
						++scan;
						sent++;
					}
//					else
//						scan++;
					continue;
				}

				// preset definition of new line by first rcvd
				if (!newline)
					newline=*scan;

				if (*scan!=newline)
				{
					// skip over the 'other' character and continue
					scan++;
					continue;
				}

				// we are about to return the newline - check the 'next' character
				// to see if it needs to be removed first too (fixes alignment issues)
				if (scan+1<end)
					if (scan[1]==0x0D || scan[1]==0x0A)
						if (scan[1]!=newline)
							scan++;

				// remove from input buffer what is about to be returned
				InputBuffer._Delete((StSize)(scan-start+1));

//				STBASE_DEBUG_OP("StFilterTextLine","READ",data_orig,data-data_orig);
				return(sent);
			}
			// remove from buffer anything parsed & passed to app's buffer
			if (scan>start)
				InputBuffer._Delete((StSize)(scan-start));

			// ### THIS IS BROKEN!!!
			// it's assuming that the input is a stream, where it may instead be a buffer
			// need to differentiate!

			// try to read more into input buffer
			if (!eof)
			{
		STBASE_DEBUG_OP2("StFilterTextLine","GetMore",this);
				//InputBuffer<<(*_FilterBase);

		// NOOOoooooo! you can't do transfer here, it tries to fill the buffer!
		//StSize got=_Transfer(_FilterBase,&InputBuffer);

				StSize got=InputBuffer._ReadFrom(*_FilterBase);
		
		STBASE_DEBUG_OP2("StFilterTextLine","Got",this);
				if (!got || _FilterBase->_Error)
					eof=1;

// What was this trying to do??
//				if (!((*_FilterBase)._IsStream()))
//					eof=1;
			}
			else
				if (scan==end)
					break;
		}
		if (InputBuffer._Error)
			return(_Err(InputBuffer));
		return(_Err(StErr_EndOfFile,"StFilterTextLine","_Read()"));
	}
	
	StSize _Read(StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StFilterTextLine","Read",data,size);
		StByte *data_orig=data;

		StSize sent=0;
		while (1)
		{
			StByte *start=InputBuffer;
		 	StByte *scan=start;
			StByte *end=scan+~InputBuffer;

			while (scan<end)
			{
				if (*scan!=0x0D && *scan!=0x0A)
				{
					// not crlf
					if (sent<size)
					{
						*data++=*scan++;
						sent++;
					}
					else
						scan++;
					continue;
				}

				// preset definition of new line by first rcvd
				if (!newline)
					newline=*scan;

				if (*scan!=newline)
				{
					// skip over the 'other' character and continue
					scan++;
					continue;
				}

				// we are about to return the newline - check the 'next' character
				// to see if it needs to be removed first too (fixes alignment issues)
				if (scan+1<end)
					if (scan[1]==0x0D || scan[1]==0x0A)
						if (scan[1]!=newline)
							scan++;

				// remove from input buffer what is about to be returned
				InputBuffer._Delete((StSize)(scan-start+1));

				STBASE_DEBUG_OP("StFilterTextLine","READ",data_orig,data-data_orig);
				return(sent);
			}
			// remove from buffer anything parsed & passed to app's buffer
			if (scan>start)
				InputBuffer._Delete((StSize)(scan-start));

			// ### THIS IS BROKEN!!!
			// it's assuming that the input is a stream, where it may instead be a buffer
			// need to differentiate!

			// try to read more into input buffer
			if (!eof)
			{
				//InputBuffer<<(*_FilterBase);
				StSize got=_Transfer(_FilterBase,&InputBuffer);
				if (!got || _FilterBase->_Error)
					eof=1;
				if (!((*_FilterBase)._IsStream()))
					eof=1;
			}
			else
				if (scan==end)
					break;
		}
		if (InputBuffer._Error)
			return(_Err(InputBuffer));
		return(_Err(StErr_EndOfFile,"StFilterTextLine","_Read()"));

	}
};



// calls a user-defined filter on a byte-by-byte basis
//
// data in >> (filter) >> fifo >> data out
//

class StFilterByte:public StBase
{
public:
	StFifoBuffer _Fifo;

	StFilterByte()
	{
		STBASE_DEBUG_CON("StFilterByte");
	}

	virtual ~StFilterByte()
	{
		STBASE_DEBUG_DES("StFilterByte");
	}

	inline virtual char _IsPipe(void)
	{
		return(1);
	}

	virtual StByte _Filter(StByte b)=0;

	StSize _Write(const StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StFilterByte","WRITE (PRE-FILTER)",data,size);
		StSize count=size;
		while (count--)
			_Fifo+=_Filter(*data++);
		return(size);
	}
	StSize _Read(StByte *data,StSize size)
	{
		STBASE_DEBUG_OP("StFitlerByte","READ FIFO (to me)",0,size);
		return(_Fifo._Read(data,size));
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		return(_Fifo._WriteTo(Destination));
	}
	virtual StSize _Read(StBase &Source)
	{
		StBuffer temp;

		StSize got=temp._ReadFrom(Source);
		// now parse temp through filter
		StByte *data=temp;
		StSize count=~temp;
		while (count--)
			_Fifo+=_Filter(*data++);

		return(got);
	}
};


// calls a user-defined filter
class StFilter:public StBase
{
public:
	StFifoBuffer _FifoInput;
	StFifoBuffer _FifoOutput;


	StFilter()
	{
		STBASE_DEBUG_CON("StFilter");
//		_FifoInput._SetDebugName("FifoInput");
//		_FifoOutput._SetDebugName("FifoOutput");
	}

	virtual ~StFilter()
	{
		STBASE_DEBUG_DES("StFilter");
	}

	inline virtual char _IsPipe(void)
	{
		return(1);
	}
	virtual StSize _Available(void)
	{
		return(_FifoOutput._Available());
	}

	virtual StSize _Filter(StByte *data,StSize size)=0;

	inline StSize _Output(const char *s)
	{
		_FifoOutput<<s;
		return(1);
	}
	inline StSize _Output(StBase &Source)
	{
		_FifoOutput<<Source;
		return(1);
	}
	inline StSize _Output(StByte data)
	{
		_FifoOutput._Write(&data,1);
		return(1);
	}
	inline StSize _Output(StByte *data,StSize size)
	{
		// add to output buffer
		return(_FifoOutput._Write(data,size));
	}

	// higher level read/write calls to transfer data between classes
	virtual StSize _Read(StBase &Source)
	{
//		Source;
		// derived class should declare how to read from another class, if possible
//		return(_Err(StErr_NotImplemented,"_Read"));
		return(_FifoInput._Read(Source));
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		// write output buffer to destination base
		// just pass request down to fifo and let it do the work
		return(_FifoOutput._WriteTo(Destination));
	}

	StSize _Write(const StByte *data,StSize size)
	{
		// add incoming data to input buffer
		_FifoInput._Write(data,size);

		// ask user to filter what is present in buffer
		StSize processed=_Filter((StByte *)_FifoInput,~_FifoInput);

		if (processed>~_FifoInput) //size)
		{
			return(_Err(StErr_Overflow,"filter processed>size"));
		}
		if (processed==StErr_RETURN)
		{
			if (!_Error)
				return(_Err(StErr_Unknown,"Filter aborted"));
			return(StErr_RETURN);
		}

		// delete what user has processed
		_FifoInput._ArrayDelete(0,processed);
		return(size);
	}
	StSize _Read(StByte *data,StSize size)
	{
		return(_FifoOutput._Read(data,size));
	}
	virtual void _FilterEnd(void)
	{
	}

	virtual inline void _ReadStop(StBase &Destination)
	{
		// when write complete, make sure all of input buffer is filtered
		while (~_FifoInput)
		{
			// but stop if the filter returns zero or error
			StSize processed=_Filter((StByte *)_FifoInput,~_FifoInput);
			if (!processed || processed==StErr_RETURN)
				break;

			_FifoInput._ArrayDelete(0,processed);
		}
		// when input complete, call _FilterEnd so that filter function can flush output if needed
		_FilterEnd();
	}

};

class StFilterToH:public StFilter
{
	StString VariableName;
	StSize length;
public:
	StFilterToH(const char *name)
	{
		VariableName<<name;
		length=0;
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		StSize return_size=size;
		if (!length)
		{
			// write the header
			_FifoOutput<<"// Generated by StFitlerToH\n";
			_FifoOutput<<"unsigned char "<<VariableName<<"[]={\n";
		}
		while (size--)
		{
			if (!(length&7))
			{
				// beginning of a line
				_FifoOutput<<"\t";
			}
			_FifoOutput<<"0x"<<StHex(*data)<<",";

			++length;
			++data;

			if (!(length&7))
			{
				// end of a line
				_FifoOutput<<"\n";
			}
		}
		return(return_size);
	}
	virtual void _FilterEnd(void)
	{
		_FifoOutput<<"\n};\n#define "<<VariableName<<"_SIZE "<<(unsigned int)length<<"\n";
	}
};

// dos/windows requires CRLF instead of just '\n'
class StFilterDosText:public StFilter
{
public:
	StFilterDosText()
	{
		STBASE_DEBUG_CON("StFilterDosText");
	}
	virtual ~StFilterDosText()
	{
		STBASE_DEBUG_DES("StFilterDosText");
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		StByte *end=data+size;
		while (data<end)
		{
			if (*data=='\n')
				_Output((StByte*)"\x0D\x0A",2);
			else
				_Output(data,1);
			++data;
		}
		// this must return the size incoming, not outgoing
		return(size);
	}
};

class StFilterReadIntelHex:public StFilter
{
public:
	StFilterReadIntelHex()
	{
		STBASE_DEBUG_CON("StFilterReadIntelHex");
	}
	virtual ~StFilterReadIntelHex()
	{
		STBASE_DEBUG_DES("StFilterReadIntelHex");
	}
	virtual StSize _Filter(StByte *data,StSize size)
	{
		StByte *end=data+size;
		while (data<end)
		{
		}
	}
};

#endif
