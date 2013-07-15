// stg template library
// table for reproducing 16 bit linear sine without sin()

#ifndef STGLIB_sine
#define STGLIB_sine

#include "/src/stglib/stcore.h"
#include "/src/stglib/stmath.h"

// helpful notification on win32 compilers
#pragma message("using stsine.h")


// sine table for 8khz 16bit linear with 256 entries

// to use: define unsigned short (16 bit value) as accumulator, 
//         add incrementor value to it each sample, use upper
//         byte as index into table to get 16 bit linear signed

// 8.192 = 65536 (incrementor resolution) / 8000 (sample rate)

//#define INDEX_FREQ(x) ((unsigned short)((((long)(x))*8192)/1000))
//#define SINE_VALUE(x) (sinetable[(x)>>8])


short StSineLevelMultiplier[]=
{
	// level*multipler/4096 for dB = 3 to -80
	5786, 5157, 4596,		// 3, 2, 1,
	4096, 3651, 3254, 2900, 2584,	// 0, -1, ...
	2303, 2053, 1830, 1631, 1453,
	1295, 1154, 1029, 917,  817,
	728,  649,  579,  516,  460,
	410,  365,  325,  290,  258,
	230,  205,  183,  163,  145,
	129,  115,  103,  92,   82,
	73,   65,   58,   52,   46,
	41,   36,   32,   29,   26,
	23,   20,   18,   16,   14,
	13,   11,   10,   9,    8,
	7,    6,    6,    5,    5,
	4,    4,    3,    3,    3,
	2,    2,    2,    2,    1,
	1,    1,    1,    1,    1,
	1,    1,    1,    0,    0,
	0,				// -80
};

short StSineTable0dBm[]=
{
	// 256 entry table of 16 bit linear sine at 0dBm (telephony)
	0,      556,    1111,   1666,   2220,   2773,   3324,   3873,
	4419,   4963,   5504,   6041,   6575,   7105,   7631,   8152,
	8668,   9179,   9685,   10184,  10678,  11165,  11645,  12119,
	12585,  13043,  13494,  13936,  14370,  14795,  15212,  15619,
	16017,  16405,  16784,  17152,  17510,  17857,  18194,  18520,
	18834,  19137,  19429,  19709,  19977,  20233,  20477,  20708,
	20927,  21134,  21327,  21508,  21676,  21831,  21973,  22101,
	22216,  22318,  22406,  22481,  22543,  22590,  22624,  22645,
	22652,  22645,  22624,  22590,  22543,  22481,  22406,  22318,
	22216,  22101,  21973,  21831,  21676,  21508,  21327,  21134,
	20927,  20708,  20477,  20233,  19977,  19709,  19429,  19137,
	18834,  18520,  18194,  17857,  17510,  17152,  16784,  16405,
	16017,  15619,  15212,  14795,  14370,  13936,  13494,  13043,
	12585,  12119,  11645,  11165,  10678,  10184,  9685,   9179,
	8668,   8152,   7631,   7105,   6575,   6041,   5504,   4963,
	4419,   3873,   3324,   2773,   2220,   1666,   1111,   556,
	0,      -556,   -1111,  -1666,  -2220,  -2773,  -3324,  -3873,
	-4419,  -4963,  -5504,  -6041,  -6575,  -7105,  -7631,  -8152,
	-8668,  -9179,  -9685,  -10184, -10678, -11165, -11645, -12119,
	-12585, -13043, -13494, -13936, -14370, -14795, -15212, -15619,
	-16017, -16405, -16784, -17152, -17510, -17857, -18194, -18520,
	-18834, -19137, -19429, -19709, -19977, -20233, -20477, -20708,
	-20927, -21134, -21327, -21508, -21676, -21831, -21973, -22101,
	-22216, -22318, -22406, -22481, -22543, -22590, -22624, -22645,
	-22652, -22645, -22624, -22590, -22543, -22481, -22406, -22318,
	-22216, -22101, -21973, -21831, -21676, -21508, -21327, -21134,
	-20927, -20708, -20477, -20233, -19977, -19709, -19429, -19137,
	-18834, -18520, -18194, -17857, -17510, -17152, -16784, -16405,
	-16017, -15619, -15212, -14795, -14370, -13936, -13494, -13043,
	-12585, -12119, -11645, -11165, -10678, -10184, -9685,  -9179,
	-8668,  -8152,  -7631,  -7105,  -6575,  -6041,  -5504,  -4963,
	-4419,  -3873,  -3324,  -2773,  -2220,  -1666,  -1111,  -556,
};

class StSine
{
public:
	unsigned short index;
	unsigned short accum;
	unsigned long multiplier;
	int silent;

	StSine()
	{
		StRandom rnd;
		accum=rnd(0);	// preset phase with random value
	}

	// note: only whole number frequency and dBm are accepted
	// dBm value is 0 for near-DFS loudness, and for each -6
	// below that is about half as loud, with -80 being silent

	short operator()(int rate,int freq,int db)
	{
		// set index speed through table based on rate and freq
		unsigned long temp;
		temp=65536000L/(long)rate;
		temp=temp*(long)freq/1000L;
		index=(unsigned short)temp;

		if (!freq) db=-80;

		// set divisor from dB
		if (db>3)
			db=3;	// above +3 dB peak exceeds DFS of 16bit linear
		if (db<-80)
			db=-80;	// lowest amplitude is -80 dB

		multiplier=StSineLevelMultiplier[3-db];

		return(0);
	}
	short operator()(void)
	{
		// provide sample, increment index
		accum+=index;
		long temp=StSineTable0dBm[accum>>8]*multiplier;
		return((unsigned short)(temp>>12));
	}
};

#endif
