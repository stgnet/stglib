
// play/record audio from windows audio "wave" interface

// this also defines "standard" formats that can be used

// for win32, must add "winmm.lib" to list of library modules

#ifndef STGLIB_STAUDIO
#define STGLIB_STAUDIO

#pragma message("using staudio.h")
#ifdef WIN32
#pragma comment(linker, "/defaultlib:winmm.lib")
#endif

#include "/src/stglib/stdio.h"
#include "/src/stglib/stfilter.h"
#include "/src/stglib/stmath.h"

// Normal CD audio rate is 44100 samples/sec, stereo (2 channels), linear PCM at 16 bits per sample
// uLaw is 8000 samples/sec, mono, ulaw at 8 bits per sample

// governing format is four parameters:
// 1) sample rate
// 2) encoding format
// 3) number of channels

// Encoding formats (from wavesurfer):
// 1) Linear 16
// 2) MuLaw (g.711)
// 3) ALaw
// 4) Linear 8 bit, offset
// 5) Linear 8 bit
// 6) Linear 24 bit
// 7) Linear 32 bit
// 8) Floating point

// Other compression formats for telephony
// G.711, G.723.1, G.726, G.728, G.729


enum StAudioCodec
{
	StAudio_Linear8=	WAVE_FORMAT_PCM|0x8000,
	StAudio_Linear16=	WAVE_FORMAT_PCM,
	StAudio_uLaw=		WAVE_FORMAT_MULAW,

} ;

#define STWAVEFORMAT_SIZE (2+2+4+4+2+2)
class StAudioFormat:public StBaseData
{
public:
	UInt16 Format;
	UInt16 Channels;
	UInt32 SamplesPerSec;
	UInt32 AverageBytesPerSec;
	UInt16 BlockAlign;
	UInt16 BitsPerSample;
	UInt16 cbSize;	// for windows API calls

	inline StByte _BytesPerSample(StAudioCodec codec)
	{
		if (codec==StAudio_Linear16)
			return(2);
		return(1);
	}

	// read/write will access only main structure, not size value
	StAudioFormat():StBaseData((StByte*)&Format,STWAVEFORMAT_SIZE)
	{
//		STBASE_DEBUG_NAME("StAudioFormat");
		cbSize=STWAVEFORMAT_SIZE;
	}
	StAudioFormat(StAudioCodec codec,unsigned long samples,unsigned char channels):StBaseData((StByte*)&Format,STWAVEFORMAT_SIZE)
	{
//		STBASE_DEBUG_NAME("StAudioFormat");

//		memset(&WaveFormat,0,sizeof(WaveFormat));
		Format = codec&0x7FFF; 
		Channels = channels; 
		SamplesPerSec = samples; 
		AverageBytesPerSec = samples*channels*_BytesPerSample(codec); 
		BlockAlign = channels*_BytesPerSample(codec);	// this needs to be computed based on FormatTag!!!
		BitsPerSample = 8*_BytesPerSample(codec);
		cbSize = STWAVEFORMAT_SIZE;
	}
	inline WAVEFORMATEX* operator&(void)
	{
		return((WAVEFORMATEX*)&Format);
	}
};



/*
MMRESULT IsFormatSupported(LPWAVEFORMATEX pwfx, UINT uDeviceID) 
{ 
    return (waveInOpen( 
        NULL,                 // ptr can be NULL for query 
        uDeviceID,            // the device identifier 
        pwfx,                 // defines requested format 
        NULL,                 // no callback 
        NULL,                 // no instance data 
        WAVE_FORMAT_QUERY));  // query only, do not open device 
} 
*/

#ifdef STGLIB_STP_GUNIX

// assuming use of /dev/dsp for speaker/microphone
class StAudioOutput:public StBase
{
protected:
	int dsp;
	int arg;

public:
	StAudioOutput(StAudioFormat &format)
	{
//		WaveFormat=format;
		dsp=open("/dev/dsp",O_RDWR);
		if (dsp==-1)
			_Errno("StAudioOutput","open(/dev/dsp)");
		else
		{
			arg=format.BitsPerSample;
			if (ioctl(dsp,SOUND_PCM_WRITE_BITS,&arg)==-1)
				_Errno("StAudioOutput","ioctl BITS");

			arg=format.Channels;
			if (ioctl(dsp,SOUND_PCM_WRITE_CHANNELS,&arg)==-1)
				_Errno("StAudioOutput","ioctl CHANNELS");

			arg=format.SamplesPerSec;
			if (ioctl(dsp,SOUND_PCM_WRITE_RATE,&arg)==-1)
				_Errno("StAudioOutput","ioctl RATE");
		}
		
	}

	StSize _Write(StByte *data,StSize size)
	{
		int sent=write(dsp,data,size);
		if (sent==-1)
			return(_Errno("StAudioOutput","write"));

		return(sent);
	}
};

#endif

#ifdef STGLIB_STP_WIN32

class StAudioOutput:public StBase
{
	friend void CALLBACK StAudio_WaveOutCallback(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);

protected:
//	WAVEHDR WaveHdr;
//	WAVEFORMATEX WaveFormat;
	StAudioFormat WaveFormat;
	HWAVEOUT hWaveOut;
	StSize BlocksInQueue;



public:

	StAudioOutput(StAudioFormat &format)
	{
//		STBASE_DEBUG_NAME("StAudioOutput");
		BlocksInQueue=0;

		WaveFormat=format;
		hWaveOut=NULL;
		if (waveOutOpen((LPHWAVEOUT)&hWaveOut,WAVE_MAPPER,(LPWAVEFORMATEX)&format,(DWORD)StAudio_WaveOutCallback,(DWORD)this,CALLBACK_FUNCTION))
		{
			_Err(StErr_IOFailure,"waveOutOpen");
			return;
		}
	}

	virtual ~StAudioOutput()
	{
		// don't close while there are blocks in the queue
		while (BlocksInQueue)
		{
//			StdOutput<<"Waiting...???\n";
			Sleep(10L);
		}
		waveOutClose(hWaveOut);
	}
/*
	void _Wait(void)
	{
		while (BlocksInQueue>1)
			Sleep(5L);
	}
*/
	void _AddBlockToQueue(StByte *block,StSize size)
	{
		while (BlocksInQueue>1)
			Sleep(1L);

		WAVEHDR *pWaveHdr=new WAVEHDR;

		pWaveHdr->lpData=(char*)block;
		pWaveHdr->dwBufferLength=(DWORD)size;
		pWaveHdr->dwFlags=0L;
		pWaveHdr->dwLoops=0L;
		waveOutPrepareHeader(hWaveOut,pWaveHdr,sizeof(WAVEHDR));

		++BlocksInQueue;

		UINT wReturn;
		wReturn=waveOutWrite(hWaveOut,pWaveHdr,sizeof(WAVEHDR));
		if (wReturn!=MMSYSERR_NOERROR)
		{
			_Err(StErr_IOFailure,"waveOutWrite");
		}
	}

	virtual StSize _Read(StBase &Source)
	{
		while (1)
		{
			StSize get=~Source;
			if (get>WaveFormat.AverageBytesPerSec)
				get=WaveFormat.AverageBytesPerSec;
			if (!get || Source._Error)
				break;

			StByte *block=new StByte[get];
			StSize got=Source._Read(block,get);
			if (Source._Error)
			{
				delete block;
				return(_Err(Source));
			}
			_AddBlockToQueue(block,got);
			if (!Source._IsPipe())
				break;
		}
		return(1);
	}

	StSize _Write(const StByte *data,StSize size)
	{
		if (!size)
			return(0);

		StByte *block=new StByte[size];
		memcpy(block,data,size);
		_AddBlockToQueue(block,size);
		return(size);
	}
};

void CALLBACK StAudio_WaveOutCallback(HWAVEOUT hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{
//	uMsg Value Meaning 
//	WOM_CLOSE Sent when the device is closed using the waveOutClose function. 
//	WOM_DONE 
//	WOM_OPEN Sent when the device is opened using the waveOutOpen function. 

	WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
	StAudioOutput* pAudioOutput= (StAudioOutput*)(dwInstance);

	switch (uMsg)
	{
	case WOM_CLOSE:
		break;

	case WOM_DONE: // Sent when the device driver is finished with a data block sent using the waveOutWrite function. 
		// dwParam1 = ptr to WaveHdr
//		StdOutput<<"Done "<<dwParam1<<" "<<dwParam2<<"\n";
		pAudioOutput->BlocksInQueue--;
		delete[] pWaveHdr->lpData;
		delete pWaveHdr;
		break;

	case WOM_OPEN:
		break;

	default:
		StdOutput<<"Unknown Msg: "<<uMsg<<"\n";
		break;
	}
}






//void CALLBACK StAudio_WaveInCallback(HWAVEIN hwo,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);



class StAudioInput:public StPipe
{

	friend void CALLBACK StAudio_WaveInCallback(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

protected:
	WAVEHDR WaveHdr;
//	WAVEFORMATEX WaveFormat;
	HWAVEIN hWaveIn;
	StSize BlockSize;

	WAVEHDR	WaveHdr1,WaveHdr2;

public:
//	StSize BlocksInQueue;

	StAudioInput(StAudioFormat &format,StSize block_size=0) //StAudio_Format format,unsigned long samples,unsigned char channels,StSize block_size=0)
	{
		if (!block_size)
			BlockSize=format.SamplesPerSec/10; // get 10 blocks per sec
		else
			BlockSize=block_size;

		BlockSize*=format.Channels;
		BlockSize*=((format.BitsPerSample+7)/8);


		MMRESULT mmre=waveInOpen((LPHWAVEIN)&hWaveIn,WAVE_MAPPER,(LPWAVEFORMATEX)&format,(DWORD)StAudio_WaveInCallback,(DWORD)this,CALLBACK_FUNCTION);
		if (mmre)
		{
			char buf[256];
			waveInGetErrorText(mmre,buf,sizeof(buf));
			_Err(StErr_IOFailure,"waveInOpen",buf);
			return;
		}
		// allocate input buffers


		WaveHdr1.lpData=new char[BlockSize];
		WaveHdr1.dwBufferLength=BlockSize;
		WaveHdr1.dwBytesRecorded=0;
		WaveHdr1.dwUser=0;
		WaveHdr1.dwFlags=0;
		WaveHdr1.dwLoops=0;
		WaveHdr1.lpNext=0;
		

		WaveHdr2.lpData=new char[BlockSize];
		WaveHdr2.dwBufferLength=BlockSize;
		WaveHdr2.dwBytesRecorded=0;
		WaveHdr2.dwUser=0;
		WaveHdr2.dwFlags=0;
		WaveHdr2.dwLoops=0;
		WaveHdr2.lpNext=0;
		
		waveInPrepareHeader(hWaveIn,&WaveHdr1,sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn,&WaveHdr1,sizeof(WAVEHDR));

		waveInPrepareHeader(hWaveIn,&WaveHdr2,sizeof(WAVEHDR));
		waveInAddBuffer(hWaveIn,&WaveHdr2,sizeof(WAVEHDR));

		mmre=waveInStart(hWaveIn);
		if (mmre)
		{
			char buf[256];
			waveInGetErrorText(mmre,buf,sizeof(buf));
			_Err(StErr_IOFailure,"waveInStart",buf);
			return;
		}
	}

	virtual ~StAudioInput()
	{
		waveInClose(hWaveIn);
	}

};




void CALLBACK StAudio_WaveInCallback(HWAVEIN hwi, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	if (uMsg!=WIM_DATA)
		return;

	WAVEHDR* pWaveHdr = ( (WAVEHDR*)dwParam1 );
	StAudioInput* pAudioInput= (StAudioInput*)(dwInstance);

	if (!hwi) return;
	if (!pWaveHdr) return;
	if (!pAudioInput) return;

	if ((pWaveHdr->dwFlags & WHDR_DONE) == WHDR_DONE) 
	{
//		pWaveHdr->dwFlags = 0;
		//if ( pAudioInput->IsError(
		
		waveInUnprepareHeader(hwi, pWaveHdr, sizeof(WAVEHDR));

		// put it in my pipe
		pAudioInput->_Write((StByte*)pWaveHdr->lpData,pWaveHdr->dwBytesRecorded);

//StdOutput<<pWaveHdr->dwBytesRecorded<<"\n";
		// don't delete it, re-use it
//		delete[] pWaveHdr->lpData;
//		pWaveHdr->lpData = NULL;
	}

	// if (shutting down) return here

	pWaveHdr->dwBufferLength=pAudioInput->BlockSize;
	pWaveHdr->dwFlags=0;
	waveInPrepareHeader(hwi,pWaveHdr,sizeof(WAVEHDR));
	waveInAddBuffer(hwi,pWaveHdr,sizeof(WAVEHDR));
};

#endif 

// conversion tool
class StAudioConvert:public StFilter
{
	StAudioFormat from;
	StAudioFormat to;
public:
	StAudioConvert(StAudioFormat in,StAudioFormat out)
	{
//		STBASE_DEBUG_NAME("StAudioConvert");
		from=in;
		to=out;
	}

	StSize _Filter(StByte *data,StSize size)
	{

// %%% THIS NEEDS TOTALLY REWRITTEN!!!

		// difficulty is that data is passed around in bytes,
		// but may actually be a stream of 16 bit values
		StInt2 *sample=(StInt2*)data;
		StInt2 *output=(StInt2*)data;

		size/=2; // convert to samples
		size/=2; // reduce to every other sample
		if (size&1)
			--size;

		StSize processed=0;

		while (size)
		{
			*output=(sample[0]+sample[1])/2;
			_Output((StByte*)output,2);
			processed+=4;
			sample+=2;
			--size;
		}
		return(processed);
	}
};

#define StAudioBP4kTaps 35
static int StAudioBP4kCoeffs[35] = 
{
	30,	// 0.00091610
	98,	// 0.00300110
	104,	// 0.00317510
	0,	// 0.00001310
	-65,	// -0.00199510
	143,	// 0.00438910
	481,	// 0.01470210
	388,	// 0.01185510
	-212,	// -0.00647110
	-375,	// -0.01145410
	621,	// 0.01896510
	1657,	// 0.05059510
	767,	// 0.02341510
	-1617,	// -0.04937410
	-1809,	// -0.05522910
	3046,	// 0.09296310
	10443,	// 0.31872910
	14046,	// 0.42866710
	10443,	// 0.31872910
	3046,	// 0.09296310
	-1809,	// -0.05522910
	-1617,	// -0.04937410
	767,	// 0.02341510
	1657,	// 0.05059510
	621,	// 0.01896510
	-375,	// -0.01145410
	-212,	// -0.00647110
	388,	// 0.01185510
	481,	// 0.01470210
	143,	// 0.00438910
	-65,	// -0.00199510
	0,	// 0.00001310
	104,	// 0.00317510
	98,	// 0.00300110
	30	// 0.00091610
};

class StAudioBP4k:public StFilter
{
	StArray<StInt2> buffer;
public:
	StSize _Filter(StByte *data,StSize size)
	{
		StInt2 *sample=(StInt2*)data;
		StInt2 output;
		StSize samples=size/2;
		StSize processed=0;
		StInt4 total;
		size=samples*2;
		while (samples--)
		{
			total=0;
			int tap=StAudioBP4kTaps;
			while (tap)
			{
				--tap;
				total+=(long)buffer[tap]*(long)StAudioBP4kCoeffs[tap];
				if (tap) buffer[tap]=buffer[tap-1];
			}
			buffer[0]=*sample++;
			output=(StInt2)(total>>16);
			_Output((StByte*)&output,2);
		}
		return(size);
	}
};

#define Pi 3.1415926535

class StAudioDesignFilter:public StArray<StInt2>
{
public:
	void BandPass(StSize SampleRate, StSize LowCut, StSize HighCut,int Taps=51)
	{
		StArray <double> Coef;
		double Sum,TmpFloat;
		StSize MaxFrequency=SampleRate/2;
		if (!HighCut)
			HighCut=MaxFrequency;

		if (HighCut>MaxFrequency)
			HighCut=MaxFrequency;

		if (LowCut>HighCut)
			LowCut=HighCut;

		// frequency = half sample rate * fraction
		double low=(double)LowCut/MaxFrequency;
		double high=(double)HighCut/MaxFrequency;

		// taps must be odd
		if (!(Taps&1))
			++Taps;

		int HalfLen=(Taps-1)/2;


		// bandpass filter
		Coef[HalfLen]=high-low;
		int cnt=1;
		while (cnt<=HalfLen)
		{
			TmpFloat=Pi*cnt;
			Coef[HalfLen+cnt]=2.0*sin((high-low)/2.0*TmpFloat)*cos((high+low)/2.0*TmpFloat)/TmpFloat;
			Coef[HalfLen-cnt]=Coef[HalfLen+cnt];
			++cnt;
		}

		TmpFloat=2.0*Pi/(Taps-1.0);
		Sum=0.0;
		cnt=0;
		while (cnt<Taps)
		{
			Coef[cnt]*=(0.54-0.46*cos(TmpFloat*cnt));
			Sum+=Coef[cnt];
			++cnt;
		}
/*
		Coef[HalfLen]+=1;
		Sum+=1;

		cnt=0;
		while (cnt<Taps)
		{
			Coef[cnt]/=fabs(Sum);
			++cnt;
		}
*/
		// Convert to 16 bit signed value
		cnt=0;
		while (cnt<Taps)
		{
			(*this)[cnt]=(StInt2)(Coef[cnt]*32768);
//			StdOutput<<(*this)[cnt]<<"\n";
			++cnt;
		}
	}
};

class StAudioBandPass:public StFilter
{
	StArray<StInt2> buffer;
	StAudioDesignFilter filter;
public:
	StAudioBandPass(StSize SampleRate, StSize LowCut, StSize HighCut,int Taps=51)
	{
//		STBASE_DEBUG_NAME("StAudioBandPass");
		filter.BandPass(SampleRate,LowCut,HighCut,Taps);
	}
	StSize _Filter(StByte *data,StSize size)
	{
		StInt2 *sample=(StInt2*)data;
		StInt2 output;
		StSize samples=size/2;
		StSize processed=0;
		StInt4 total;
		size=samples*2;
		while (samples--)
		{
			total=0;
			int tap=~filter;
			while (tap)
			{
				--tap;
				total+=(long)buffer[tap]*(long)filter[tap];
				if (tap) buffer[tap]=buffer[tap-1];
			}
			buffer[0]=*sample++;
			output=(StInt2)(total>>16);
			_Output((StByte*)&output,2);
		}
		return(size);
	}
};


#endif
