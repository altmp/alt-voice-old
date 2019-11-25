#pragma once
#include "IOpusEncoder.h"
#include <opus.h>

class COpusEncoder : public IOpusEncoder
{
	OpusEncoder* encoder;
public:
	COpusEncoder(int sampleRate, int channels);
	~COpusEncoder();

	int EncodeFloat(void* pcmData, size_t size, void* output, size_t outputSize) override;
	int EncodeShort(void* pcmData, size_t size, void* output, size_t outputSize) override;
};
