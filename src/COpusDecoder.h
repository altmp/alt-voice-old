#pragma once
#include "IOpusDecoder.h"
#include <opus.h>

class COpusDecoder : public IOpusDecoder
{
	OpusDecoder* decoder;
public:
	COpusDecoder(int sampleRate, int channels);
	~COpusDecoder();

	int DecodeFloat(void* opusData, size_t size, void* pcmData, size_t outputSize, bool fec) override;
	int DecodeShort(void* opusData, size_t size, void* pcmData, size_t outputSize, bool fec) override;
};
