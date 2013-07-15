
// WAV files contain audio data in RIFF format

#ifndef STGLIB_STWAVE
#define STGLIB_STWAVE

#pragma message("using stwave.h")

#include "/src/stglib/stfile.h"
#include "/src/stglib/staudio.h"

class StWaveID:public StBaseData
{
public:
	char ID[4];

	StWaveID():StBaseData((StByte*)&ID[0],4)
	{
//		STBASE_DEBUG_NAME("StWaveID");
	}

	inline StWaveID& operator=(const char *s)
	{
		ID[0]=s[0];
		ID[1]=s[1];
		ID[2]=s[2];
		ID[3]=s[3];
		return(*this);
	}

	inline bool operator!=(const char *s)
	{
		return
		(	!(
			(ID[0]==s[0]) && 
			(ID[1]==s[1]) && 
			(ID[2]==s[2]) && 
			(ID[3]==s[3])
			)
		);
	}
};


class StWaveChunk:public StBaseData
{
public:
	char ID[4];
	UInt32 Size;

	StWaveChunk():StBaseData((StByte*)&ID,8)
	{
//		STBASE_DEBUG_NAME("StWaveChunk");
	}

	inline StWaveChunk& operator=(const char *s)
	{
		ID[0]=s[0];
		ID[1]=s[1];
		ID[2]=s[2];
		ID[3]=s[3];
		return(*this);
	}

	inline bool operator!=(const char *s)
	{
		return
		(	!(
			(ID[0]==s[0]) && 
			(ID[1]==s[1]) && 
			(ID[2]==s[2]) && 
			(ID[3]==s[3])
			)
		);
	}
};




class StWaveFile:public StBase
{
private:
	StFile file;
	StWaveChunk _FileHeader;
	StWaveID _FileType;
	StWaveChunk _FileChunk;
	UInt32 _DataSize;
public:
	StAudioFormat _WaveFormat;
	StSize _WriteChunkSizeOnClose;

	StWaveFile(const char *FileName,StAudioFormat format)
	{
		_WriteChunkSizeOnClose=0;

		file._FileOpen(FileName,StFileOpenOverwrite);
		if (file._Error)
		{
			_Err(file);
			return;
		}

		_FileHeader="RIFF";
		_FileHeader>>file;
		_WriteChunkSizeOnClose+=~_FileHeader;

		_FileType="WAVE";
		_FileType>>file;
		_WriteChunkSizeOnClose+=~_FileType;

		_FileChunk="fmt ";
		_FileChunk.Size=STWAVEFORMAT_SIZE;
		_FileChunk>>file;
		_WriteChunkSizeOnClose+=~_FileChunk;

		_WaveFormat=format;
		_WaveFormat>>file;
		_WriteChunkSizeOnClose+=STWAVEFORMAT_SIZE;

		_FileChunk="data";
		_FileChunk.Size=0;	// this is patched on close
		_FileChunk>>file;

		_DataSize=0;
	}
	StWaveFile(const char *FileName)
	{
		_WriteChunkSizeOnClose=0;
//		STBASE_DEBUG_NAME("StWaveFile");
//		STBASE_DEBUG_NAME(FileName);
		_DataSize=0;
		file._FileOpen(FileName);
		if (file._Error)
		{
			_Err(file);
			return;
		}

		_FileHeader<<file;
		if (_FileHeader._Error)
		{
			_Err(_FileHeader);
			return;
		}
		if (_FileHeader!="RIFF")
		{
			_Err(StErr_InvalidDataFormat,"StWaveFile","Not labelled as RIFF file");
			return;
		}
		_FileType<<file;
		if (_FileType._Error)
		{
			_Err(_FileType);
			return;
		}
		if (_FileType!="WAVE")
		{
			_Err(StErr_InvalidDataFormat,"StWaveFile","Not WAVE file");
			return;
		}

		// get the format chunk
		_FileChunk<<file;
		if (_FileChunk._Error)
		{
			_Err(_FileChunk);
			return;
		}
		if (_FileChunk!="fmt ")
		{
			_Err(StErr_InvalidDataFormat,"StWaveFile","Chunk not fmt");
			return;
		}
		// %%% TODO: This is often failing because some files have it set to 18, not 16
		// fix by adjusting the read length
		if (_FileChunk.Size!=STWAVEFORMAT_SIZE)
		{
			_Err(StErr_InvalidDataLength,"StWaveFile","Format chunk wrong size");
			return;
		}

		_WaveFormat<<file;
		if (_WaveFormat._Error)
		{
			_Err(_FileHeader);
			return;
		}
//		StdOutput<<"Format:"<<_WaveFormat.Format<<" Channels:"<<_WaveFormat.Channels<<" sam/sec:"<<_WaveFormat.SamplesPerSec<<"\n";
//		StdOutput<<"BitsPerSample:"<<_WaveFormat.BitsPerSample<<"\n";


		// get the data chunk
		_FileChunk<<file;
		if (_FileChunk._Error)
		{
			_Err(_FileChunk);
			return;
		}
		if (_FileChunk!="data")
		{
			_Err(StErr_InvalidDataFormat,"StWaveFile","Chunk not fmt");
			return;
		}

		_DataSize=_FileChunk.Size;

	}

	~StWaveFile()
	{
		if (_WriteChunkSizeOnClose)
		{
			_FileChunk="data";
			_FileChunk.Size=_DataSize;
			file._FileSeek(_WriteChunkSizeOnClose);
			_FileChunk>>file;
			file._FileClose();
		}
	}

	StSize _Available(void)
	{
		// return size info from header
		return(_DataSize);
	}

	StSize _Read(StByte *data,StSize size)
	{
		if (size>_DataSize)
			size=_DataSize;
		while (size%_WaveFormat.BlockAlign)
			--size;

		StSize got=file._Read(data,size);
		if (!got) return(got); // not error, but end of file
		if (got!=size)
			return(_Err(StErr_InvalidDataLength,"StWaveFile","Read Chunk"));
		return(got);
	}

	StSize _Write(StByte *data,StSize size)
	{
		StSize went=file._Write(data,size);
		if (went!=size)
			return(_Err(file)); //StErr_InvalidDataLength,"StWaveFile","Write Chunk"));
		_DataSize+=went;
		return(went);
	}
};



#endif



#ifdef BOGUS
// ******************************************************************************** OLD SAMPLE CODE

#include <io.h>

enum DDCRET
{
   DDC_SUCCESS,           // The operation succeded
   DDC_FAILURE,           // The operation failed for unspecified reasons
   DDC_OUT_OF_MEMORY,     // Operation failed due to running out of memory
   DDC_FILE_ERROR,        // Operation encountered file I/O error
   DDC_INVALID_CALL,      // Operation was called with invalid parameters
   DDC_USER_ABORT,        // Operation was aborted by the user
   DDC_INVALID_FILE       // File format does not match
};


const char *DDCRET_String ( DDCRET );   // See source\ddcret.cpp


//#define  TRUE     1
//#define  FALSE    0

typedef int dBOOLEAN;

//typedef unsigned char BYTE;

typedef unsigned char        UINT8;
typedef signed   char        INT8;

typedef unsigned short int   UINT16;
typedef signed   short int   INT16;
//typedef unsigned long  int   UINT32;
//typedef signed   long  int   INT32;

#ifdef __BORLANDC__
	#if sizeof(UINT16) != 2
	  #error Need to fix UINT16 and INT16
	#endif

	#if sizeof(UINT32) != 4
	  #error Need to fix UINT32 and INT32
	#endif
#endif



#pragma pack(1)

//unsigned long FourCC ( const char *ChunkName );
UINT32 FourCC ( const char *ChunkName );


enum RiffFileMode
{
   RFM_UNKNOWN,      // undefined type (can use to mean "N/A" or "not open")
   RFM_WRITE,        // open for write
   RFM_READ          // open for read
};


struct RiffChunkHeader
{
   UINT32    ckID;       // Four-character chunk ID
   UINT32    ckSize;     // Length of data in chunk
};


class RiffFile
{
   RiffChunkHeader   riff_header;      // header for whole file

protected:
   RiffFileMode      fmode;            // current file I/O mode
   FILE             *file;             // I/O stream to use
   DDCRET  Seek ( long offset );

public:
   RiffFile();
   ~RiffFile();

   RiffFileMode CurrentFileMode() const   {return fmode;}

   DDCRET Open ( const char *Filename, RiffFileMode NewMode );
   DDCRET Write  ( const void *Data, unsigned NumBytes );
   DDCRET Read   (       void *Data, unsigned NumBytes );
   DDCRET Expect ( const void *Data, unsigned NumBytes );
   DDCRET Close();

   long    CurrentFilePosition() const;

   DDCRET  Backpatch ( long FileOffset,
                       const void *Data,
                       unsigned NumBytes );
};


struct WaveFormat_ChunkData
{
   UINT16         wFormatTag;       // Format category (PCM=1)
   UINT16         nChannels;        // Number of channels (mono=1, stereo=2)
   UINT32         nSamplesPerSec;   // Sampling rate [Hz]
   UINT32         nAvgBytesPerSec;
   UINT16         nBlockAlign;
   UINT16         nBitsPerSample;

   void Config ( UINT32   NewSamplingRate   = 44100,
                 UINT16   NewBitsPerSample  =    16,
                 UINT16   NewNumChannels    =     2 )
   {
      nSamplesPerSec  = NewSamplingRate;
      nChannels       = NewNumChannels;
      nBitsPerSample  = NewBitsPerSample;
      nAvgBytesPerSec = (nChannels * nSamplesPerSec * nBitsPerSample) / 8;
      nBlockAlign     = (nChannels * nBitsPerSample) / 8;
   }

   WaveFormat_ChunkData()
   {
      wFormatTag = 1;     // PCM
      Config();
   }
};


struct WaveFormat_Chunk
{
   RiffChunkHeader         header;
   WaveFormat_ChunkData    data;

   WaveFormat_Chunk()
   {
      header.ckID     =   FourCC("fmt");
      header.ckSize   =   sizeof ( WaveFormat_ChunkData );
   }

   dBOOLEAN VerifyValidity()
   {
      return header.ckID == FourCC("fmt") &&

             (data.nChannels == 1 || data.nChannels == 2) &&

             data.nAvgBytesPerSec == ( data.nChannels *
                                       data.nSamplesPerSec *
                                       data.nBitsPerSample    ) / 8   &&

             data.nBlockAlign == ( data.nChannels *
                                   data.nBitsPerSample ) / 8;
   }
};


#define  MAX_WAVE_CHANNELS   2


struct WaveFileSample
{
   INT16  chan [MAX_WAVE_CHANNELS];
};


class WaveFile: private RiffFile
{
   WaveFormat_Chunk   wave_format;
   RiffChunkHeader    pcm_data;
   long               pcm_data_offset;  // offset of 'pcm_data' in output file
   UINT32             num_samples;

public:
   WaveFile();

   DDCRET OpenForWrite ( const char  *Filename,
                         UINT32       SamplingRate   = 44100,
                         UINT16       BitsPerSample  =    16,
                         UINT16       NumChannels    =     2 );

   DDCRET OpenForRead ( const char *Filename );

   DDCRET ReadSample   ( INT16 Sample [MAX_WAVE_CHANNELS] );
   DDCRET WriteSample  ( const INT16 Sample [MAX_WAVE_CHANNELS] );
   DDCRET SeekToSample ( unsigned long SampleIndex );

   // The following work only with 16-bit audio
   DDCRET WriteData ( const INT16 *data, UINT32 numData );
   DDCRET ReadData  ( INT16 *data, UINT32 numData );

   // The following work only with 8-bit audio
   DDCRET WriteData ( const UINT8 *data, UINT32 numData );
   DDCRET ReadData  ( UINT8 *data, UINT32 numData );

   DDCRET ReadSamples  ( INT32 num, WaveFileSample[] );

   DDCRET WriteMonoSample    ( INT16 ChannelData );
   DDCRET WriteStereoSample  ( INT16 LeftChannelData, INT16 RightChannelData );

   DDCRET ReadMonoSample ( INT16 *ChannelData );
   DDCRET ReadStereoSample ( INT16 *LeftSampleData, INT16 *RightSampleData );

   DDCRET Close();

   UINT32   SamplingRate()   const;    // [Hz]
   UINT16   BitsPerSample()  const;
   UINT16   NumChannels()    const;
   UINT32   NumSamples()     const;

   // Open for write using another wave file's parameters...

   DDCRET OpenForWrite ( const char *Filename,
                         WaveFile &OtherWave )
   {
      return OpenForWrite ( Filename,
                            OtherWave.SamplingRate(),
                            OtherWave.BitsPerSample(),
                            OtherWave.NumChannels() );
   }

   long CurrentFilePosition() const
   {
      return RiffFile::CurrentFilePosition();
   }
};

#pragma pack()


UINT32 FourCC ( const char *ChunkName )
{
   UINT32 retbuf = 0x20202020;   // four spaces (padding)
   char *p = ((char *)&retbuf);

   // Remember, this is Intel format!
   // The first character goes in the LSB

   for ( int i=0; i<4 && ChunkName[i]; i++ )
   {
	  *p++ = ChunkName[i];
   }

   return retbuf;
}

RiffFile::RiffFile()
{
   file = 0;
   fmode = RFM_UNKNOWN;

   riff_header.ckID = FourCC("RIFF");
   riff_header.ckSize = 0;
}


RiffFile::~RiffFile()
{
   if ( fmode != RFM_UNKNOWN )
   {
	  Close();
   }
}


DDCRET RiffFile::Open ( const char *Filename, RiffFileMode NewMode )
{
   DDCRET retcode = DDC_SUCCESS;

   if ( fmode != RFM_UNKNOWN )
   {
	  retcode = Close();
   }

   if ( retcode == DDC_SUCCESS )
   {
	  switch ( NewMode )
	  {
		 case RFM_WRITE:
			  file = fopen ( Filename, "wb" );
			  if ( file )
			  {
				 // Write the RIFF header...
				 // We will have to come back later and patch it!

				 if ( fwrite ( &riff_header, sizeof(riff_header), 1, file ) != 1 )
				 {
					fclose(file);
					unlink(Filename);
					fmode = RFM_UNKNOWN;
					file = 0;
				 }
				 else
				 {
					fmode = RFM_WRITE;
				 }
			  }
			  else
			  {
				 fmode = RFM_UNKNOWN;
				 retcode = DDC_FILE_ERROR;
			  }
			  break;

		 case RFM_READ:
			  file = fopen ( Filename, "rb" );
			  if ( file )
			  {
				 // Try to read the RIFF header...

				 if ( fread ( &riff_header, sizeof(riff_header), 1, file ) != 1 )
				 {
					fclose(file);
					fmode = RFM_UNKNOWN;
					file = 0;
				 }
				 else
				 {
					fmode = RFM_READ;
				 }
			  }
			  else
			  {
				 fmode = RFM_UNKNOWN;
				 retcode = DDC_FILE_ERROR;
			  }
			  break;

		 default:
			  retcode = DDC_INVALID_CALL;
	  }
   }

   return retcode;
}


DDCRET RiffFile::Write ( const void *Data, unsigned NumBytes )
{
   if ( fmode != RFM_WRITE )
   {
	  return DDC_INVALID_CALL;
   }

   if ( fwrite ( Data, NumBytes, 1, file ) != 1 )
   {
	  return DDC_FILE_ERROR;
   }

   riff_header.ckSize += NumBytes;

   return DDC_SUCCESS;
}


DDCRET RiffFile::Close()
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( fmode )
   {
	  case RFM_WRITE:
		   if ( fflush(file) ||
				fseek(file,0,SEEK_SET) ||
				fwrite ( &riff_header, sizeof(riff_header), 1, file ) != 1 ||
				fclose(file) )
		   {
			  retcode = DDC_FILE_ERROR;
		   }
		   break;

	  case RFM_READ:
		   fclose(file);
		   break;
   }

   file = 0;
   fmode = RFM_UNKNOWN;

   return retcode;
}


long RiffFile::CurrentFilePosition() const
{
   return ftell ( file );
}


DDCRET RiffFile::Seek ( long offset )
{
   fflush ( file );

   DDCRET rc;

   if ( fseek ( file, offset, SEEK_SET ) )
   {
	  rc = DDC_FILE_ERROR;
   }
   else
   {
	  rc = DDC_SUCCESS;
   }

   return rc;
}


DDCRET RiffFile::Backpatch ( long FileOffset,
							 const void *Data,
							 unsigned NumBytes )
{
   if ( !file )
   {
	  return DDC_INVALID_CALL;
   }

   if ( fflush(file) ||
		fseek ( file, FileOffset, SEEK_SET ) )
   {
	  return DDC_FILE_ERROR;
   }

   return Write ( Data, NumBytes );
}


DDCRET RiffFile::Expect ( const void *Data, unsigned NumBytes )
{
   char *p = (char *)Data;

   while ( NumBytes-- )
   {
	  if ( fgetc(file) != *p++ )
	  {
		 return DDC_FILE_ERROR;
	  }
   }

   return DDC_SUCCESS;
}


DDCRET RiffFile::Read ( void *Data, unsigned NumBytes )
{
   DDCRET retcode = DDC_SUCCESS;

   if ( fread(Data,NumBytes,1,file) != 1 )
   {
	  retcode = DDC_FILE_ERROR;
   }

   return retcode;
}


//-----------------------------------------------------------------------

WaveFile::WaveFile()
{
   pcm_data.ckID = FourCC("data");
   pcm_data.ckSize = 0;
   num_samples = 0;
}


DDCRET WaveFile::OpenForRead ( const char *Filename )
{
   // Verify filename parameter as best we can...
   if ( !Filename )
   {
	  return DDC_INVALID_CALL;
   }

   DDCRET retcode = Open ( Filename, RFM_READ );

   if ( retcode == DDC_SUCCESS )
   {
	  retcode = Expect ( "WAVE", 4 );

	  if ( retcode == DDC_SUCCESS )
	  {
		 retcode = Read ( &wave_format, sizeof(wave_format) );

		 if ( retcode == DDC_SUCCESS &&
			  !wave_format.VerifyValidity() )
		 {
			// This isn't standard PCM, so we don't know what it is!

			retcode = DDC_FILE_ERROR;
		 }

		 if ( retcode == DDC_SUCCESS )
		 {
			pcm_data_offset = CurrentFilePosition();

			// Figure out number of samples from
			// file size, current file position, and
			// WAVE header.



			retcode = Read ( &pcm_data, sizeof(pcm_data) );
			num_samples = filelength(fileno(file)) - CurrentFilePosition();
			num_samples /= NumChannels();
			num_samples /= (BitsPerSample() / 8);
		 }
	  }
   }

   return retcode;
}


DDCRET WaveFile::OpenForWrite ( const char  *Filename,
								UINT32       SamplingRate,
								UINT16       BitsPerSample,
								UINT16       NumChannels )
{
   // Verify parameters...

   if ( !Filename ||
		(BitsPerSample != 8 && BitsPerSample != 16) ||
		NumChannels < 1 || NumChannels > 2 )
   {
	  return DDC_INVALID_CALL;
   }

   wave_format.data.Config ( SamplingRate, BitsPerSample, NumChannels );

   DDCRET retcode = Open ( Filename, RFM_WRITE );

   if ( retcode == DDC_SUCCESS )
   {
	  retcode = Write ( "WAVE", 4 );

	  if ( retcode == DDC_SUCCESS )
	  {
		 retcode = Write ( &wave_format, sizeof(wave_format) );

		 if ( retcode == DDC_SUCCESS )
		 {
			pcm_data_offset = CurrentFilePosition();
			retcode = Write ( &pcm_data, sizeof(pcm_data) );
		 }
	  }
   }

   return retcode;
}


DDCRET WaveFile::Close()
{
   DDCRET rc = DDC_SUCCESS;
   
   if ( fmode == RFM_WRITE )
      rc = Backpatch ( pcm_data_offset, &pcm_data, sizeof(pcm_data) );

   if ( rc == DDC_SUCCESS )
	  rc = RiffFile::Close();

   return rc;
}


DDCRET WaveFile::WriteSample ( const INT16 Sample [MAX_WAVE_CHANNELS] )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nChannels )
   {
	  case 1:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   pcm_data.ckSize += 1;
				   retcode = Write ( &Sample[0], 1 );
				   break;

			  case 16:
				   pcm_data.ckSize += 2;
				   retcode = Write ( &Sample[0], 2 );
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  case 2:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   retcode = Write ( &Sample[0], 1 );
				   if ( retcode == DDC_SUCCESS )
				   {
					  retcode = Write ( &Sample[1], 1 );
					  if ( retcode == DDC_SUCCESS )
					  {
						 pcm_data.ckSize += 2;
					  }
				   }
				   break;

			  case 16:
				   retcode = Write ( &Sample[0], 2 );
				   if ( retcode == DDC_SUCCESS )
				   {
					  retcode = Write ( &Sample[1], 2 );
					  if ( retcode == DDC_SUCCESS )
					  {
						 pcm_data.ckSize += 4;
					  }
				   }
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::WriteMonoSample ( INT16 SampleData )
{
   switch ( wave_format.data.nBitsPerSample )
   {
	  case 8:
		   pcm_data.ckSize += 1;
		   return Write ( &SampleData, 1 );

	  case 16:
		   pcm_data.ckSize += 2;
		   return Write ( &SampleData, 2 );
   }

   return DDC_INVALID_CALL;
}


DDCRET WaveFile::WriteStereoSample ( INT16 LeftSample,
									 INT16 RightSample )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nBitsPerSample )
   {
	  case 8:
		   retcode = Write ( &LeftSample, 1 );
		   if ( retcode == DDC_SUCCESS )
		   {
			  retcode = Write ( &RightSample, 1 );
			  if ( retcode == DDC_SUCCESS )
			  {
				 pcm_data.ckSize += 2;
			  }
		   }
		   break;

	  case 16:
		   retcode = Write ( &LeftSample, 2 );
		   if ( retcode == DDC_SUCCESS )
		   {
			  retcode = Write ( &RightSample, 2 );
			  if ( retcode == DDC_SUCCESS )
			  {
				 pcm_data.ckSize += 4;
			  }
		   }
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}



DDCRET WaveFile::ReadSample ( INT16 Sample [MAX_WAVE_CHANNELS] )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nChannels )
   {
	  case 1:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   unsigned char x;
				   retcode = Read ( &x, 1 );
				   Sample[0] = INT16(x);
				   break;

			  case 16:
				   retcode = Read ( &Sample[0], 2 );
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  case 2:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   unsigned char  x[2];
				   retcode = Read ( x, 2 );
				   Sample[0] = INT16 ( x[0] );
				   Sample[1] = INT16 ( x[1] );
				   break;

			  case 16:
				   retcode = Read ( Sample, 4 );
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadSamples ( INT32 num, WaveFileSample sarray[] )
{
   DDCRET retcode = DDC_SUCCESS;
   INT32 i;

   switch ( wave_format.data.nChannels )
   {
	  case 1:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   for ( i=0; i < num && retcode == DDC_SUCCESS; i++ )
				   {
					  unsigned char x;
					  retcode = Read ( &x, 1 );
					  sarray[i].chan[0] = INT16(x);
				   }
				   break;

			  case 16:
				   for ( i=0; i < num && retcode == DDC_SUCCESS; i++ )
				   {
					  retcode = Read ( &sarray[i].chan[0], 2 );
				   }
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  case 2:
		   switch ( wave_format.data.nBitsPerSample )
		   {
			  case 8:
				   for ( i=0; i < num && retcode == DDC_SUCCESS; i++ )
				   {
					  unsigned char x[2];
					  retcode = Read ( x, 2 );
					  sarray[i].chan[0] = INT16 ( x[0] );
					  sarray[i].chan[1] = INT16 ( x[1] );
				   }
				   break;

			  case 16:
				   retcode = Read ( sarray, 4 * num );
				   break;

			  default:
				   retcode = DDC_INVALID_CALL;
		   }
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadMonoSample ( INT16 *Sample )
{
   DDCRET retcode = DDC_SUCCESS;

   switch ( wave_format.data.nBitsPerSample )
   {
	  case 8:
		   unsigned char x;
		   retcode = Read ( &x, 1 );
		   *Sample = INT16(x);
		   break;

	  case 16:
		   retcode = Read ( Sample, 2 );
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::ReadStereoSample ( INT16 *L, INT16 *R )
{
   DDCRET retcode = DDC_SUCCESS;
   UINT8          x[2];
   INT16          y[2];

   switch ( wave_format.data.nBitsPerSample )
   {
	  case 8:
		   retcode = Read ( x, 2 );
		   *L = INT16 ( x[0] );
		   *R = INT16 ( x[1] );
		   break;

	  case 16:
		   retcode = Read ( y, 4 );
		   *L = INT16 ( y[0] );
		   *R = INT16 ( y[1] );
		   break;

	  default:
		   retcode = DDC_INVALID_CALL;
   }

   return retcode;
}


DDCRET WaveFile::SeekToSample ( unsigned long SampleIndex )
{
   if ( SampleIndex >= NumSamples() )
   {
	  return DDC_INVALID_CALL;
   }

   unsigned SampleSize = (BitsPerSample() + 7) / 8;

   DDCRET rc = Seek ( pcm_data_offset + sizeof(pcm_data) +
					  SampleSize * NumChannels() * SampleIndex );

   return rc;
}


UINT32 WaveFile::SamplingRate() const
{
   return wave_format.data.nSamplesPerSec;
}


UINT16 WaveFile::BitsPerSample() const
{
   return wave_format.data.nBitsPerSample;
}


UINT16 WaveFile::NumChannels() const
{
   return wave_format.data.nChannels;
}


UINT32 WaveFile::NumSamples() const
{
   return num_samples;
}


DDCRET WaveFile::WriteData ( const INT16 *data, UINT32 numData )
{
	UINT32 extraBytes = numData * sizeof(INT16);
	pcm_data.ckSize += extraBytes;
	return RiffFile::Write ( data, extraBytes );
}


DDCRET WaveFile::WriteData ( const UINT8 *data, UINT32 numData )
{
	pcm_data.ckSize += numData;
	return RiffFile::Write ( data, numData );
}


DDCRET WaveFile::ReadData ( INT16 *data, UINT32 numData )
{
	return RiffFile::Read ( data, numData * sizeof(INT16) );
}


DDCRET WaveFile::ReadData ( UINT8 *data, UINT32 numData )
{
	return RiffFile::Read ( data, numData );
}


#endif
