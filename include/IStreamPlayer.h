#pragma once

class IStreamPlayer
{
public:
	virtual ~IStreamPlayer() = default;

	virtual bool PushOpusBuffer(const void * data, int count) = 0;

	virtual void SetPosition(float x, float y, float z) = 0;
	virtual void SetVelocity(float x, float y, float z) = 0;
	virtual void SetDirection(float x, float y, float z) = 0;

	virtual void SetMaxDistance(float distance) = 0;
	virtual void SetMinDistance(float distance) = 0;
	virtual void SetRolloffFactor(float rolloff) = 0;
	virtual void SetSpatialSoundState(bool state) = 0;
	virtual void SetExtraGain(float gain) = 0;
	virtual bool IsPlaying() = 0;

	virtual bool Update() = 0;
};
