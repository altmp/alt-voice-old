#pragma once
#include <mutex>
#include <queue>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <opus.h>

#include "CRingBuffer.h"
#include "IStreamPlayer.h"
#include "C3DSoundOutput.h"

#define NUM_BUFFERS 64
#define MIN_BUFFERS_TO_PLAY 10

class CStreamPlayer: public IStreamPlayer
{
	friend C3DSoundOutput;

	RingBuffer<short, 262144> ringBuffer;

	ALuint buffers[NUM_BUFFERS];
	ALuint source;
	ALuint buffersFilled = 0;

	ALenum format = AL_FORMAT_MONO16;
	ALsizei srate = 0;

	ALfloat currentPos[3] = { 0.f, 0.f, 0.f };
	ALfloat currentVel[3] = { 0.f, 0.f, 0.f };
	ALfloat currentDir[3] = { 0.f, 0.f, 0.f };

	C3DSoundOutput* _soundOutput = nullptr;
	OpusDecoder* _dec = nullptr;

	uint32_t pushedBuffers = 0;

	std::queue<ALuint> freeBuffers;
	bool _isPlaying = false;
public:
	CStreamPlayer();
	~CStreamPlayer();

	bool PushOpusBuffer(const void * data, int count) override;

	void SetPosition(float x, float y, float z) override;
	void SetVelocity(float x, float y, float z) override;
	void SetDirection(float x, float y, float z) override;

	void SetMaxDistance(float distance) override;
	void SetMinDistance(float distance) override;
	void SetRolloffFactor(float rolloff) override;
	bool IsPlaying() override;

	bool Update() override;
};

