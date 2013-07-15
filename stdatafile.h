// STGLIB/stdatafile.h
// Copyright 1999,2002 by StG Net
//
// data file class
//
// Provides a flexible tree-structure data "object" storage mechanism:
// - each object can be any size
// - free space handling
// - split block allocation system (multiples of 2):
//    - contiguous data access for each object
//    - like sized blocks grouped together for effeciency
//
// block header is:
//
//   00   01   02   03   04   05   06   07   08   09   0A
// -------------------------------------------------------------
// | BS | TC | NP : NP | CP : CP | DP : DP | DS : .. | Data... |
// -------------------------------------------------------------
//
// Smallest possible BS is 3 (8), thus ptrs can be *8
//

#ifndef STGLIB_STDATAFILE
#define STGLIB_STDATAFILE

#pragma message("using stdatafile.h")

// this include gets base and everything we need
#include "/stglib/stcore.h"

#include "/stglib/stbuffer.h"

#define STDF_BlockSize (16)

class StDataFile;





#define hex(x) ((x)>9?(x)+'A'-10:(x)+'0')

void dump(unsigned char *b,int l)
{
	int x,y;
	unsigned char d;
	unsigned char a[80];
	unsigned char *p;
	unsigned int v;

	y=0;
	while (l)
	{
		p=a;
		x=4;
		v=y;
		p+=4;
		while (x--)
		{
			*--p=hex(v&15);
			v>>=4;
		}
		p+=4;
		*p++=':';
		*p++=' ';

		x=0;
		while (x<16)
		{
			if (x==l) break;

			d=*(b+x);
			v=d>>4;
			*p++=hex(v&15);
			*p++=hex(d&15);
			*p++=' ';
			x++;
		}
		while (x<16)
		{
			*p++=' ';
			*p++=' ';
			*p++=' ';
			x++;
		}
		*p++=' ';
		x=0;
		while (x<16)
		{
			if (x==l) break;

			d=*(b+x);
			d&= 127;
			if (d<32 || d>126) d='.';
			*p++=d;
			x++;
		}
		if (l>16) l-=16; else l=0;
		b+=16;

		*p=0;
		printf("%s\n",a);
		y+=16;
	}
}


//////////////////////////////////////////////////////////////////////
//  StDataFileBlockType - enum for block header type code
//////////////////////////////////////////////////////////////////////
enum StDataFileBlockType
{
	STDFBT_Root,
	STDFBT_Data,
	STDFBT_Free=0xFF
};

//////////////////////////////////////////////////////////////////////
//  StDataFilePointer - handle block pointers
//////////////////////////////////////////////////////////////////////
// for now, this just maps to the variable
// needs to be changed to use the first byte as power of two adder
class StDataFilePointer
{
	StSize _Ptr;
public:
	inline operator StSize()
	{
		return(_Ptr*STDF_BlockSize);
	}
	inline void operator=(StSize value)
	{
		_Ptr=(value/STDF_BlockSize);
	}
	inline void operator+=(StSize value)
	{
		_Ptr+=(value/STDF_BlockSize);
	}
	StDataFilePointer()
	{
		_Ptr=0;
	}

	inline void operator!(void)
	{
		_Ptr=0;
	}
};

//////////////////////////////////////////////////////////////////////
//  StDataFileBlockSize - handle the block size byte
//////////////////////////////////////////////////////////////////////
class StDataFileBlockSize
{
	StByte BlockSize;

public:
	StDataFileBlockSize()
	{
		BlockSize=0;
	}
	inline void operator!(void)
	{
		BlockSize=0;
	}
	inline operator StSize()
	{
		return(1<<(BlockSize+4));
	}
	inline StDataFileBlockSize& operator=(StSize value)
	{
		BlockSize=1;
		while (value>16)
		{
			BlockSize++;
			value>>=1;
		}
		return(*this);
	}
	inline operator--()
	{
		BlockSize--;
	}
	inline operator++()
	{
		BlockSize++;
	}
};

class StDataFileHeader
{
public:
	StDataFileBlockSize BlockSize;
	StByte Type;
	St2Byte Filler;
	StDataFilePointer Next;		// next block (relative!)
	StDataFilePointer Child;	// child block (relative!)
	StDataFilePointer Desc;		// desc block (relative?)

	// data starts at hdr + 16 bytes, 1st 4 bytes is data size
	//	StSize DataSize;			// data size (root block: file size)

	inline void operator!(void)
	{
		!BlockSize;
		Type=STDFBT_Root;
		Filler=0;
		!Next;
		!Child;
		!Desc;
//		DataSize=0;
	}
	StDataFileHeader()
	{
//		!(*this);


	}

};

//////////////////////////////////////////////////////////////////////
//  StDataEmpty - empty data block
//////////////////////////////////////////////////////////////////////

class StDataFileEmpty:public StArray<StByte>
{
public:
	StDataFileEmpty()
	{
		_ArrayResize(256);
		// force used count
		_ArrayUsed=256;
	}
	operator unsigned char*()
	{
		return((unsigned char*)_ArrayPtr);
	}
} StDataFileEmpty;

//////////////////////////////////////////////////////////////////////
//  StDataFile
//////////////////////////////////////////////////////////////////////

class StDataFileIO:public StFile
{
protected:
	StDataFileHeader RootHdr;
	StDataFilePointer RootPtr;


	StSize _ReadBlockHdr(StDataFilePointer &where,StDataFileHeader &header)
	{
		if (_FileSeek(where)) return(StErr_RETURN);
		return(_Read((StByte*)&header,sizeof(header)));
	}

	StSize _WriteBlockHdr(StDataFilePointer &where,StDataFileHeader &header)
	{
		if (_FileSeek(where)) return(StErr_RETURN);
printf("Writing to %08X (hdr):\n",where);
dump((StByte*)&header,sizeof(header));
		return(_Write((StByte*)&header,sizeof(header)));

	}
	StSize _WriteBlockData(StDataFilePointer &where,StByte *data,StSize size)
	{
		if (_FileSeek(where+sizeof(RootHdr))) return(StErr_RETURN);
printf("Writing to %08X (%d data):\n",where,size);
		if (_Write((StByte*)&size,sizeof(size))==StErr_RETURN) return(StErr_RETURN);
dump(data,size);
		return(_Write(data,size));
	}

	void _UpdateRoot(void)
	{
		_WriteBlockHdr(RootPtr,RootHdr);
	}

	StSize _WipeBlock(StDataFilePointer &ptr,StDataFileHeader &hdr)
	{
		// skip the header
		ptr+=sizeof(hdr);

		if (_FileSeek(ptr)) return(StErr_RETURN);
		StSize Left=hdr.BlockSize-sizeof(hdr);
		while (Left)
		{
			if (Left<=~StDataFileEmpty)
				return(_Write(StDataFileEmpty,Left));

			_Write((StByte*)StDataFileEmpty,~StDataFileEmpty);
			Left-=~StDataFileEmpty;
		}
		return(0);
	}


	

/*	// write as many empty (free) blocks as are needed to fill specified size
	StSize _DataFileEmptyWrite(StDataFilePointer &ptr,StSize size)
	{
		StDataFileBlockSize block;
		block=size;
		if (block==size)
		{
			// yeah! write this and we're done!
			StDataFileHeader Empty;
			Empty.BlockSize=block;
			Empty.Type=STDFBT_Free;
			Empty.Next=Root.Next;
			Root.Next=ptr;
			_DataFileHeaderWrite(ptr,Empty);
			_DataFileZeroData(ptr,Empty);
			return(_Error);
		}
		--block;
		_DataFileEmptyWrite(ptr,size-block);
		ptr+=(size-block);
		_DataFileEmptyWrite(ptr,block);
		return(0);
	}

	StSize _DataFileExpand(StSize BytesNeeded)
	{
		if (!Root.DataSize)
		{
			// file is empty - write initial header
			Root.BlockSize=sizeof(Root);
			Root.DataSize=1; // datasize is in blocks just like ptrs
			
			_DataFileHeaderWrite(RootPtr,Root);
		}

		// calculate new file size
		StDataFileBlockSize FileSizeNeeded;
		FileSizeNeeded=Root.DataSize+BytesNeeded;
		StSize NewEmptySize=FileSizeNeeded-Root.DataSize;

		// set ptr to new block
		StDataFilePointer NewEmptyPtr;
		NewEmptyPtr=Root.DataSize;

		// set new data file to root block
		Root.DataSize=FileSizeNeeded;

		// call recursive loop to write out empty block(s)
		_DataFileEmptyWrite(NewEmptyPtr,NewEmptySize);

		// update root
		_DataFileHeaderWrite(RootPtr,Root);

		return(_Error);
	}
	
*/
public:
	void _DataFileOpen(char *Path)
	{
		if (_FileOpen(Path,"r+b"))
		{
			// oops, file doesn't already exist!
			_Error=StErr_NONE;

			// create empty file...
			if (_FileOpen(Path,"w+b"))
				return;

			// and write the root header
			!RootHdr;
//			_DataFileExpand(1024);

			// manually write header rather than use expand for  now
			RootHdr.Next=sizeof(RootHdr);
			_UpdateRoot();
			return;
		}

		_ReadBlockHdr(RootPtr,RootHdr);
	}


	StDataFileIO(char *Path)
	{
		// sanity check
		if (sizeof(StDataFileHeader)!=STDF_BlockSize)
		{
			_Err(StErr_CompiledStructureSizedWrong,"StDataFileHeader!=16");
			return;
		}
		if (sizeof(StSize)!=4)
		{
			_Err(StErr_CompiledStructureSizedWrong,"StSize!=4");
			return;
		}

		_DataFileOpen(Path);

	}

	// functioned defined later, as they interact with StDataFile
	void _SetDescPtr(StDataFile &df,StDataDescriptor &desc);
	void _WriteData(StDataFile &parent,StDataFile &df,StBase &source);
};

//////////////////////////////////////////////////////////////////////
//  StDataFile
//////////////////////////////////////////////////////////////////////
class StDataFile:public StBase
{
	friend class StDataFileIO;

	StDataFile *_Parent;
	StDataFileIO *_IO;

	StDataFilePointer _Ptr;
	StDataFileHeader _Hdr;

	StDataFilePointer _DescPtr;
	StBuffer temp;

public:
	StDataFile(char *filename)
	{
		_Parent=0;
		_IO=new StDataFileIO(filename);
		if (_IO->_Error)
			_Err(*_IO);
	}
	StDataFile(StDataFile &parent,StDataDescriptor &desc)
	{
		_Parent=&parent;
		_IO=_Parent->_IO;

		_IO->_SetDescPtr(*this,desc);
	}
	virtual ~StDataFile()
	{
		if (!_Parent)
		{
			// special case - this is the master instance
			// that allocated the _IO class
			delete _IO;
		}
	}

	void _Reset(void)
	{
		// unlink entire child chain
	}

	virtual void _ReadStart(StBase &Source)
	{
		!temp;
	}
	virtual StSize _Read(StBase &Source)
	{
		temp._Read(Source);
		// app should be unaware of read error?
		return(0);
	}
	virtual void _ReadStop(StBase &Source)
	{
		_IO->_WriteData(*_Parent,*this,temp);
	}

	virtual void _WriteStart(StBase &Destination)
	{
	}
	virtual StSize _WriteTo(StBase &Destination)
	{
		return(0);
	}
	virtual void _WriteStop(StBase &Destination)
	{
	}

	virtual StBase& _RecurseLevel(StBase &object)
	{
		StDataDescriptor Dsub("sub-file");
		// create new sub-level instance
		StDataFile *child=new StDataFile(*this,*(object.GetDataDescriptor()));

		child->_BaseDataDescriptor(Dsub);

		// write my data (BUT HOW???)
//		object.operator>>(*child);

		return(*child);
	}
};


//////////////////////////////////////////////////////////////////////
//  StDataFileIO Functions (that reference StDataFile)
//////////////////////////////////////////////////////////////////////

void StDataFileIO::_SetDescPtr(StDataFile &df,StDataDescriptor &Descriptor)
{
	StDataFilePointer DescPtr;
	StDataFileHeader DescHdr;
	StSize DataSize;
	
	// create header
	DataSize=~Descriptor;
	DescHdr.BlockSize=sizeof(DescHdr)+sizeof(DataSize)+DataSize;

	// locate it and reserve it
	DescPtr=RootHdr.Next;
	RootHdr.Next+=DescHdr.BlockSize;

	// link it into descriptor list
	DescHdr.Next=RootHdr.Desc-DescPtr;
	RootHdr.Desc=DescPtr;

	// write it
	_WriteBlockHdr(DescPtr,DescHdr);
	_WriteBlockData(DescPtr,Descriptor,DataSize);

	// and update root (to store links)
	_UpdateRoot();
}

void StDataFileIO::_WriteData(StDataFile &parent,StDataFile &df,StBase &Source)
{
	StBuffer DataBuffer;
	DataBuffer<<Source;

	StSize DataSize=~DataBuffer;
	!df._Hdr;
	df._Hdr.BlockSize=sizeof(df._Hdr)+sizeof(DataSize)+DataSize;

	// allocate block from root
	df._Ptr=RootHdr.Next;
	RootHdr.Next+=df._Hdr.BlockSize;

	// link into parent
	df._Hdr.Next=parent._Hdr.Child-df._Ptr;
	parent._Hdr.Child=df._Ptr;

	// write it
	_WriteBlockHdr(df._Ptr,df._Hdr);
	_WriteBlockData(df._Ptr,DataBuffer,DataSize);

	// update parent
	_WriteBlockHdr(parent._Ptr,parent._Hdr);

	// update root
	_UpdateRoot();
}


#include "/stglib/stpostfx.h"

#endif
