#pragma once
#include "VoiceError.h"

class CVoiceException
{
	AltVoiceError _exception;
public:
	CVoiceException(AltVoiceError voiceException) : _exception(voiceException) {};
	AltVoiceError GetCode() const
	{
		return _exception;
	}
};