#pragma once

using OnVoiceCallback = void(*)(const void* buffer, int size, float micLevel);

class ISoundInput
{
public:
	virtual ~ISoundInput() = default;

	virtual bool EnableInput() = 0;
	virtual bool DisableInput() = 0;
	virtual void ChangeMicGain(float gain) = 0;
	virtual bool ChangeDevice(char* deviceName) = 0;
	virtual void RegisterCallback(OnVoiceCallback callback) = 0;
};
