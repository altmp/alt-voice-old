#include "CStreamPlayer.h"
#include <iostream>

CStreamPlayer::CStreamPlayer()
{
	alGenBuffers(NUM_BUFFERS, buffers);
	for (uint16_t i = 0; i < NUM_BUFFERS; ++i)
		freeBuffers.push(buffers[i]);

	if (alGetError() != AL_NO_ERROR)
		throw std::runtime_error("Could not create buffers");

	alGenSources(1, &source);
	if (alGetError() != AL_NO_ERROR)
		throw std::runtime_error("Could not create source");

	alSourcef(source,	AL_PITCH,		1.f);
	alSourcef(source,	AL_GAIN,		1.f);
	alSourcefv(source,	AL_POSITION,	currentPos);
	alSourcefv(source,	AL_VELOCITY,	currentVel);
	alSourcefv(source,	AL_DIRECTION,	currentDir);
	alSourcei(source,	AL_LOOPING,		false);

	alSourceRewind(source);
	alSourcei(source, AL_BUFFER, 0);

	if (alGetError() != AL_NO_ERROR)
		throw std::runtime_error("Could not set source parameters");
}


CStreamPlayer::~CStreamPlayer()
{
	alDeleteSources(1, &source);
	alDeleteBuffers(NUM_BUFFERS, buffers);
}

bool CStreamPlayer::PushOpusBuffer(const void * data, int count)
{
	static opus_int16 out[8196];
	int frame_size = opus_decode(_dec, (const unsigned char*)data, count, out, 8196, 0);
	if (frame_size < 0)
		return false;

	pushedBuffers++;

	ringBuffer.Write(out, frame_size);
	return true;
}

void CStreamPlayer::SetPosition(float x, float y, float z)
{
	currentPos[0] = x;
	currentPos[1] = y;
	currentPos[2] = z;
	alSourcefv(source, AL_POSITION, currentPos);
}

void CStreamPlayer::SetVelocity(float x, float y, float z)
{
	currentVel[0] = x;
	currentVel[1] = y;
	currentVel[2] = z;
	alSourcefv(source, AL_VELOCITY, currentVel);
}

void CStreamPlayer::SetDirection(float x, float y, float z)
{
	currentDir[0] = x;
	currentDir[1] = y;
	currentDir[2] = z;
	alSourcefv(source, AL_DIRECTION, currentDir);
}

void CStreamPlayer::SetMaxDistance(float distance)
{
	alSourcef(source, AL_MAX_DISTANCE, distance);
}

void CStreamPlayer::SetMinDistance(float distance)
{
	alSourcef(source, AL_REFERENCE_DISTANCE, distance);
}

void CStreamPlayer::SetRolloffFactor(float rolloff)
{
	alSourcef(source, AL_ROLLOFF_FACTOR, rolloff);
}

bool CStreamPlayer::IsPlaying()
{
	return _isPlaying;
}

bool CStreamPlayer::Update()
{
	ALint processed, state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);
	_isPlaying = state == AL_PLAYING;

	alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
	if (alGetError() != AL_NO_ERROR)
		return false;

	if (!ringBuffer.BytesToRead())
		return true;

	if(ringBuffer.IsHalfFull())
		alSourcef(source, AL_PITCH, 1.1f);
	else
		alSourcef(source, AL_PITCH, 1.f);

	bool streamPaused = processed == 0;

	while (processed > 0)
	{
		ALuint bufid;
		alSourceUnqueueBuffers(source, 1, &bufid);
		freeBuffers.push(bufid);
		processed--;
	}

	while(ringBuffer.BytesToRead() && freeBuffers.size())
	{
		static short tempBuffer[480];
		const ALuint bufferId = freeBuffers.front();
		freeBuffers.pop();

		ALsizei readed = 0;
		int bufferIndex = 0;
		readed = (ALsizei)ringBuffer.Read(tempBuffer, 480);
		if (readed > 0)
		{
			alBufferData(bufferId, format, tempBuffer, readed * sizeof(short), srate);
			alSourceQueueBuffers(source, 1, &bufferId);
		}
		else
			break;

		if (alGetError() != AL_NO_ERROR)
			return false;
	}

	if (state != AL_PLAYING && pushedBuffers > MIN_BUFFERS_TO_PLAY)
	{
		std::cout << pushedBuffers << " buffers collected. Start playing" << std::endl;
		alSourcePlay(source);
		if (alGetError() != AL_NO_ERROR)
			return false;
	}

	return true;
}
