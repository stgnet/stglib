// STGLIB/stspeech.h
// Copyright 2005 by StG Net

// need microsoft definitions
#include "/src/stglib/stwin32.h"

#ifndef STGLIB_STSPEECH
#define STGLIB_STSPEECH

#pragma message("using stspeech.h")

#include <atlbase.h>

class StSpeech
{
public:

	operator()(char *string)
	{
		HRESULT				hr = S_OK;
		CComPtr <ISpVoice>		cpVoice;
		CComPtr <ISpObjectToken>	cpToken;
		CComPtr <IEnumSpObjectTokens>	cpEnum;

		//Create a SAPI voice
		hr = cpVoice.CoCreateInstance( CLSID_SpVoice );
		
		//Enumerate voice tokens with attribute "Name=Microsoft Sam?
		if(SUCCEEDED(hr))
		{
			hr = SpEnumTokens(SPCAT_VOICES, L"Name=Microsoft Sam", NULL, &cpEnum);
		}

		//Get the closest token
		if(SUCCEEDED(hr))
		{
			hr = cpEnum ->Next(1, &cpToken, NULL);
		}
		
		//set the voice
		if(SUCCEEDED(hr))
		{
			hr = cpVoice->SetVoice( cpToken);
		}

		//set the output to the default audio device
		if(SUCCEEDED(hr))
		{
			hr = cpVoice->SetOutput( NULL, TRUE );
		}

		//Speak the text file (assumed to exist)
 		//Speak the text ?hello world? synchronously
		if(SUCCEEDED(hr))
		{
			hr = cpVoice->Speak( L"Hello World",  SPF_DEFAULT, NULL );
		}

		//Release objects
		cpVoice.Release ();
		cpEnum.Release();
		cpToken.Release();

	}
};

#endif
