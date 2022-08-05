#pragma once
#include <vector>
#include <utility>
#include <string>

using OnVoiceCallback = void(*)(const void* buffer, int size, float micLevel);

class ISoundInput
{
public:
	virtual ~ISoundInput() = default;

	virtual bool EnableInput() = 0;
	virtual bool DisableInput() = 0;
	virtual void ChangeMicGain(float gain) = 0;
	virtual void RegisterCallback(OnVoiceCallback callback) = 0;
	virtual void RegisterRawCallback(OnVoiceCallback callback) = 0;
	virtual void SetNoiseSuppressionStatus(bool enabled) = 0;
	virtual void SetNormalizationEnabled(bool enabled) = 0;

	virtual std::vector<std::pair<int, std::string>> GetInputDevices() = 0;
	virtual void SetInputDevice(int id) = 0;
	virtual int32_t GetInputDevice() = 0;
};
