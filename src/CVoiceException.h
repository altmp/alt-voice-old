#pragma once
#include "VoiceError.h"

class CVoiceException
{
	friend class CSoundOutput;
	friend class CSoundInput;
	friend class CStreamPlayer;
	friend class COpusEncoder;
	friend class COpusDecoder;

	AltVoiceError _exception;
	CVoiceException(AltVoiceError voiceException) : _exception(voiceException) {};
public:
	AltVoiceError GetCode() const
	{
		return _exception;
	}
};