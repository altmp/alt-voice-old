#include "COpusEncoder.h"
#include "CVoiceException.h"

COpusEncoder::COpusEncoder(int sampleRate, int channels)
{
	int opusErr;
	encoder = opus_encoder_create(sampleRate, channels, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK || encoder == nullptr)
		throw CVoiceException(AltVoiceError::OpusEncoderCreateError);
}

COpusEncoder::~COpusEncoder()
{
	opus_encoder_destroy(encoder);
}

int COpusEncoder::EncodeFloat(void* pcmData, size_t size, void* output, size_t outputSize)
{
	int len = opus_encode_float(encoder, (const float*)pcmData, size, (unsigned char*)output, outputSize);
	if (len < 0 || len > outputSize)
		return 0;
	return len;
}

int COpusEncoder::EncodeShort(void* pcmData, size_t size, void* output, size_t outputSize)
{
	int len = opus_encode(encoder, (const opus_int16*)pcmData, size, (unsigned char*)output, outputSize);
	if (len < 0 || len > outputSize)
		return 0;
	return len;
}
