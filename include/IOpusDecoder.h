#pragma once

class IOpusDecoder
{
public:
	virtual int DecodeFloat(void* opusBuffer, size_t size, void* pcmData, size_t outputSize, bool fec = true, bool missing = false) = 0;
	virtual int DecodeShort(void* opusBuffer, size_t size, void* pcmData, size_t outputSize, bool fec = true, bool missing = false) = 0;
};