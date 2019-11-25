#pragma once

class IOpusEncoder
{
public:
	virtual int EncodeFloat(void* pcmData, size_t size, void* output, size_t outputSize) = 0;
	virtual int EncodeShort(void* pcmData, size_t size, void* output, size_t outputSize) = 0;
};
