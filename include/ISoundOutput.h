#pragma once
#include <vector>
#include <utility>
#include <string>

using OnVoiceCallback = void(*)(const void* buffer, int size, float micLevel);

class ISoundOutput
{
public:
	virtual ~ISoundOutput() = default;

	virtual void ChangeOutputVolume(float volume) = 0;
	virtual void Write(void* data, size_t size) = 0;

	virtual std::vector<std::pair<int, std::string>> GetOutputDevices() = 0;
	virtual void SetOutputDevice(int id) = 0;
	virtual int32_t GetOutputDevice() = 0;
};