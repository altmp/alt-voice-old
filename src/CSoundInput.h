#pragma once
#include <opus.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "ISoundInput.h"
#include "CRingBuffer.h"

#define FRAME_SIZE_OPUS 480
#define MAX_PACKET_SIZE 32768

using Sample = float;

class CSoundInput: public ISoundInput
{
	void OnVoiceInput();
	uint32_t _sampleRate;
	uint32_t _framesPerBuffer;
	uint32_t _bitRate;

	float micGain = 1.f;

	OpusEncoder* enc = nullptr;

	OnVoiceCallback cb = nullptr;
	RingBuffer<Sample, 100000> _ringBuffer;

	ALCdevice* inputDevice = nullptr;
	std::thread* inputStreamThread = nullptr;
	Sample* transferBuffer = nullptr;
	bool inputActive = false;
	int sleepTime = 0;
public:
	CSoundInput(int sampleRate, int framesPerBuffer, int bitrate);
	~CSoundInput();

	bool EnableInput() override;
	bool DisableInput() override;
	void ChangeMicGain(float gain) override;

	void RegisterCallback(OnVoiceCallback callback) override;
};

