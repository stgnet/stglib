// play wav file

#include "/src/stglib/stglib.h"
#include "/src/stglib/staudio.h"
#include "/src/stglib/stwave.h"

int main(int argc,char **argv)
{
	if (!*++argv)
	{
		StdOutput<<"use: play (file.wav)\n";
		exit(1);
	}

	StWaveFile source(*argv);
	if (source._Error)
	{
		StdError<<"could not open file: "<<source._GetErrorString()<<"\n";
		exit(2);
	}

	StdOutput<<"       Format="<<source._WaveFormat.Format<<"\n";
	StdOutput<<"     Channels="<<source._WaveFormat.Channels<<"\n";
	StdOutput<<"SamplesPerSec="<<source._WaveFormat.SamplesPerSec<<"\n";
	StdOutput<<"BitsPerSample="<<source._WaveFormat.BitsPerSample<<"\n";

	StAudioOutput speaker(source._WaveFormat);
	if (speaker._Error)
	{
		StdError<<"could not open speaker: "<<speaker._GetErrorString()<<"\n";
		exit(3);
	}

	source>>speaker;

	return(0);
}
