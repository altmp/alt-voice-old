#pragma once
#include <mutex>
#include <queue>
#include <chrono>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <opus.h>

#include "CRingBuffer.h"
#include "IStreamPlayer.h"
#include "CSoundOutput.h"

#define NUM_BUFFERS 64
#define MIN_BUFFERS_TO_PLAY 10
#define RING_BUFFER_SIZE 262144
#define OPUS_BUFFER_SIZE 8196

using Sample = ALfloat;

class CStreamPlayer: public IStreamPlayer
{
	friend CSoundOutput;
	static CSoundOutput* soundOutput;
	static ALfloat zeroFloatVector[3];

	RingBuffer<Sample, RING_BUFFER_SIZE> ringBuffer;

	ALuint buffers[NUM_BUFFERS];
	ALuint source;
	ALuint buffersFilled = 0;

	ALenum format = AL_FORMAT_MONO_FLOAT32;

	ALfloat pitch = 1.f;
	ALfloat gain = 1.f;
	ALfloat currentPos[3] = { 0.f, 0.f, 0.f };
	ALfloat currentVel[3] = { 0.f, 0.f, 0.f };
	ALfloat currentDir[3] = { 0.f, 0.f, 0.f };
	ALfloat minDistance = 0.f;
	ALfloat maxDistance = 100.f;
	ALfloat rolloffFactor = 1.f;

	OpusDecoder* dec = nullptr;

	std::chrono::time_point<std::chrono::system_clock> lastSourceRequestTime;

	std::queue<ALuint> freeBuffers;
	bool isPlaying = false;
	bool hasSource = false;
	bool sourceUsedOnce = false;

	bool disableSpatial = false;
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
	void SetSpatialSoundState(bool state) override;
	bool IsPlaying() override;

	bool Update() override;

private:
	bool UpdateSource(ALuint source);
	void DropSource();
};

