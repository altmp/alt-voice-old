#pragma once
#include <portaudio.h>
#include <opus.h>

#include "ISoundInput.h"
#include "CRingBuffer.h"

#define FRAME_SIZE_OPUS 480
#define MAX_PACKET_SIZE 32768

using Sample = float;

class CSoundInput: public ISoundInput
{
	static int OnInputCallback(const void *inputBuffer, void *outputBuffer,
		unsigned long framesPerBuffer,
		const PaStreamCallbackTimeInfo* timeInfo,
		PaStreamCallbackFlags statusFlags,
		void *userData);
	uint32_t _sampleRate;
	uint32_t _framesPerBuffer;
	uint32_t _bitRate;
	uint32_t _micLever;

	float micGain = 1.f;

	PaStream* stream = nullptr;
	OpusEncoder* enc = nullptr;

	OnVoiceCallback cb = nullptr;
	RingBuffer<Sample, 96000> _ringBuffer;
public:
	CSoundInput(int sampleRate, int framesPerBuffer, int bitrate);
	~CSoundInput();

	bool EnableInput() override;
	bool DisableInput() override;
	void ChangeMicGain(float gain) override;

	float GetCPULoad() override;

	void RegisterCallback(OnVoiceCallback callback) override;
};

