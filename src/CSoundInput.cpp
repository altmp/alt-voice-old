#include <chrono>
#include <fstream>
#include "CSoundInput.h"
#include "CVoiceException.h"

#define EXIT_ON_ERROR(hres, error)  \
              if (FAILED(hres)) { lastError = error; goto Exit; }

//https://forum.juce.com/t/float-to-decibel-conversion/1841
float CSoundInput::LinearToDecibel(float linear)
{
	if (linear != 0.0f)
		return 20.0f * log10(linear);
	else
		return -144.0f;  // effectively minus infinity
}

double CSoundInput::GetSignalMultiplierForVolume(double volume) 
{	//https://github.com/almoghamdani/audify/blob/master/src/rt_audio.cpp#L422
	// Explained here: https://stackoverflow.com/a/1165188
	return (std::pow(10, volume) - 1) / (10 - 1);
}

void CSoundInput::GainPCM(Sample* data, size_t framesCount)
{
	for (int i = 0; i < framesCount; ++i)
		data[i] *= micGain;
}

void CSoundInput::OnPcmData(int16_t* data, uint32_t framesCount)
{
	_ringBuffer->Write((const Sample*)data, framesCount);
	while (_ringBuffer->BytesToRead() >= FRAME_SIZE_OPUS)
	{
		_ringBuffer->Read((Sample*)opusInputFrameBuffer, FRAME_SIZE_OPUS);

		if (noiseSuppressionEnabled)
			Denoise(opusInputFrameBuffer);

		int16_t micLevel = 0;
		for (int i = 0; i < FRAME_SIZE_OPUS; ++i)
		{
			if (opusInputFrameBuffer[i] > micLevel)
				micLevel = opusInputFrameBuffer[i];
		}
		
		if (normalizationEnabled)
			Normalize(opusInputFrameBuffer, FRAME_SIZE_OPUS);
		
		GainPCM(opusInputFrameBuffer, FRAME_SIZE_OPUS);

		if (rawCb) rawCb(opusInputFrameBuffer, FRAME_SIZE_OPUS * sizeof(Sample), (float)micLevel / MAXSHORT);

		int len = enc->EncodeShort(opusInputFrameBuffer, FRAME_SIZE_OPUS, packet, MAX_PACKET_SIZE);

		if (len < 0 || len > MAX_PACKET_SIZE) return;
		if (cb) cb(packet, len, (float)micLevel / MAXSHORT);
	}
}

void CSoundInput::Denoise(Sample* buffer)
{
	if (denoiseSt)
	{
		// pcm / 2 is an epic workaround on RNNoise distortion
		for (int i = 0; i < FRAME_SIZE_OPUS; ++i) floatBuffer[i] = buffer[i] / 2;

		for (int i = 0; i < FRAME_SIZE_OPUS; i += 480)
			rnnoise_process_frame(denoiseSt, floatBuffer + i, floatBuffer + i);

		for (int i = 0; i < FRAME_SIZE_OPUS; ++i) buffer[i] = floatBuffer[i] * 2;
	}
}

void CSoundInput::Normalize(Sample* buffer, size_t frameSize)
{
	Sample maxFrame = 0;
	for (int i = 0; i < frameSize; ++i)
	{
		Sample s = abs(buffer[i]);
		if (s > maxFrame)
			maxFrame = s;
	}

	if (normalizeMax == 0.f || maxFrame > normalizeMax || normalizeMax / maxFrame < 0.5)
		normalizeMax = maxFrame;
	else
		normalizeMax = (normalizeMax * (NORMALIZE_FRAME_COUNT - 1) + maxFrame) / NORMALIZE_FRAME_COUNT;

	if (normalizeMax <= 1.f)
		return;

	float gain = MAXSHORT / normalizeMax / 2;
	gain = min(gain, 10);

	for (int i = 0; i < frameSize; ++i)
		buffer[i] *= gain;
}

CSoundInput::CSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate): _sampleRate(sampleRate), _framesPerBuffer(framesPerBuffer), _bitRate(bitrate)
{
	AltVoiceError lastError = AltVoiceError::Ok;
	HRESULT hr = S_OK;

	_ringBuffer = new RingBuffer<Sample>(sampleRate * sizeof(Sample));

	//create input device here -------------------------------------------------------------------------
	auto ret = g_deviceManager.OpenInputStream(sampleRate, FRAME_SIZE_OPUS, [this](int16_t* data, uint32_t size) { this->OnPcmData(data, size); });
	if(ret != AltVoiceError::Ok)
		EXIT_ON_ERROR(-1, ret);

	try {
		enc = new COpusEncoder(sampleRate, 1, bitrate);
	}
	catch (const CVoiceException& e) {
		EXIT_ON_ERROR(-1, e.GetCode());
	}

	denoiseSt = rnnoise_create(NULL);
	if (!denoiseSt)
		EXIT_ON_ERROR(-1, AltVoiceError::DenoiseInitError);

Exit:
	if (lastError != AltVoiceError::Ok)
	{
		if (enc)
			delete enc;

		if (denoiseSt)
			rnnoise_destroy(denoiseSt);

		throw CVoiceException(lastError);
	}
}


CSoundInput::~CSoundInput()
{
	delete _ringBuffer;

	if (enc)
		delete enc;

	if (denoiseSt)
		rnnoise_destroy(denoiseSt);
}

bool CSoundInput::EnableInput()
{
	if (!inputActive)
	{
		inputActive = true;
		return true;
	}
	return false;
}

bool CSoundInput::DisableInput()
{
	if (inputActive)
	{
		inputActive = false;
		return true;
	}
	return false;
}

void CSoundInput::ChangeMicGain(float gain)
{
	micGain = GetSignalMultiplierForVolume(gain);
}

bool CSoundInput::ChangeDevice(char * deviceName)
{
	return true;
}

void CSoundInput::RegisterCallback(OnVoiceCallback callback)
{
	cb = callback;
}

void CSoundInput::RegisterRawCallback(OnVoiceCallback callback)
{
	rawCb = callback;
}

void CSoundInput::SetNoiseSuppressionStatus(bool enabled)
{
	noiseSuppressionEnabled = enabled;
}

void CSoundInput::SetNormalizationEnabled(bool enabled)
{
	normalizationEnabled = enabled;
	normalizeMax = 0.f;
}
