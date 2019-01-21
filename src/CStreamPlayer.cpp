#include "CStreamPlayer.h"
#include <iostream>

CStreamPlayer::CStreamPlayer()
{
	alGenBuffers(NUM_BUFFERS, buffers);
	for (uint16_t i = 0; i < NUM_BUFFERS; ++i)
	{
		freeBuffers.push(buffers[i]);
		std::cout << "Buffer [ID: " << buffers[i] << "] allocated" << std::endl;
	}

	if (alGetError() != AL_NO_ERROR)
		throw std::runtime_error("Could not create buffers");
}


CStreamPlayer::~CStreamPlayer()
{
	alDeleteBuffers(NUM_BUFFERS, buffers);
}

bool CStreamPlayer::PushOpusBuffer(const void * data, int count)
{
	static Sample out[OPUS_BUFFER_SIZE];
	int frame_size = opus_decode_float(dec, (const unsigned char*)data, count, out, OPUS_BUFFER_SIZE, 0);
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

	if(hasSource)
		alSourcefv(source, AL_POSITION, currentPos);
}

void CStreamPlayer::SetVelocity(float x, float y, float z)
{
	currentVel[0] = x;
	currentVel[1] = y;
	currentVel[2] = z;

	if (hasSource)
		alSourcefv(source, AL_VELOCITY, currentVel);
}

void CStreamPlayer::SetDirection(float x, float y, float z)
{
	currentDir[0] = x;
	currentDir[1] = y;
	currentDir[2] = z;

	if (hasSource)
		alSourcefv(source, AL_DIRECTION, currentDir);
}

void CStreamPlayer::SetMaxDistance(float distance)
{
	maxDistance = distance;
	if(hasSource)
		alSourcef(source, AL_MAX_DISTANCE, distance);
}

void CStreamPlayer::SetMinDistance(float distance)
{
	minDistance = distance;
	if(hasSource)
		alSourcef(source, AL_REFERENCE_DISTANCE, distance);
}

void CStreamPlayer::SetRolloffFactor(float rolloff)
{
	rolloffFactor = rolloff;
	if(hasSource)
		alSourcef(source, AL_ROLLOFF_FACTOR, rolloff);
}

bool CStreamPlayer::IsPlaying()
{
	return isPlaying;
}

bool CStreamPlayer::Update()
{
	bool sourceJustReceived = false;
	if (!hasSource)
	{
		if (!ringBuffer.BytesToRead())
			return true;

		//TODO: Remove sources from far objects if all busy
		if (!soundOutput->GetSource(source))
			return true;

		UpdateSource(source);
		sourceJustReceived = true;
		isPlaying = false;
	}

	ALint processed, state;

	//Unqueue old buffers
	if (!sourceJustReceived)
	{
		alGetSourcei(source, AL_SOURCE_STATE, &state);
		isPlaying = state == AL_PLAYING;

		alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
		if (alGetError() != AL_NO_ERROR)
			return false;

		if (!ringBuffer.BytesToRead() && !isPlaying)
		{
			while (processed > 0)
			{
				ALuint bufid;
				alSourceUnqueueBuffers(source, 1, &bufid);
				freeBuffers.push(bufid);
				processed--;
			}

			hasSource = false;
			soundOutput->FreeSource(source);
			return true;
		}

		if (ringBuffer.IsHalfFull())
			alSourcef(source, AL_PITCH, 1.05f);
		else
			alSourcef(source, AL_PITCH, 1.f);

		while (processed > 0)
		{
			ALuint bufid;
			alSourceUnqueueBuffers(source, 1, &bufid);
			freeBuffers.push(bufid);
			processed--;
		}
	}
	
	while (ringBuffer.BytesToRead() && freeBuffers.size())
	{
		Sample tempBuffer[4096];
		const ALuint bufferId = freeBuffers.front();
		freeBuffers.pop();

		ALsizei readed = 0;
		int bufferIndex = 0;
		readed = (ALsizei)ringBuffer.Read(tempBuffer, 4096);
		if (readed > 0)
		{
			alBufferData(bufferId, format, tempBuffer, readed * sizeof(Sample), srate);
			alSourceQueueBuffers(source, 1, &bufferId);
		}
		else
			break;

		if (alGetError() != AL_NO_ERROR)
			return false;
	}

	if (!isPlaying && pushedBuffers > MIN_BUFFERS_TO_PLAY)
	{
		std::cout << "Start playing from source [ID: " << source << "]" << std::endl;
		alSourcePlay(source);
		if (alGetError() != AL_NO_ERROR)
			return false;
	}
	return true;
}

//TODO: Refactor
bool CStreamPlayer::UpdateSource(ALuint source)
{
	alSourceRewind(source);
	alSourcei(source, AL_LOOPING, false);
	alSourcei(source, AL_BUFFER, 0);
	alSourcefv(source, AL_POSITION, currentPos);
	alSourcefv(source, AL_VELOCITY, currentVel);
	alSourcefv(source, AL_DIRECTION, currentDir);
	alSourcef(source, AL_MAX_DISTANCE, maxDistance);
	alSourcef(source, AL_REFERENCE_DISTANCE, minDistance);
	alSourcef(source, AL_ROLLOFF_FACTOR, rolloffFactor);

	hasSource = true;
	if (alGetError() != AL_NO_ERROR)
		return false;
	return true;
}
