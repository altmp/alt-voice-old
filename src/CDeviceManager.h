#pragma once
#include <RtAudio.h>
#include "CRingBuffer.h"
#include "CVoiceException.h"

class CDeviceManager
{
	std::shared_ptr<RtAudio> rtAudio;
	std::string inputDeviceName;
	int32_t inputDeviceID = -1;

	uint32_t frameSize;
	std::function<void(int16_t*, uint32_t size)> inputCallback = nullptr;

	static int DeviceDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData);
public:
	CDeviceManager() : 
		rtAudio(std::make_shared<RtAudio>(RtAudio::WINDOWS_DS)) {}

    std::vector<std::string> GetInputDevices();
	std::vector<std::string> GetOutputDevices();

	void SetInputDevice(const char* name);
	int32_t GetInputDevivce();
	AltVoiceError OpenInputStream(unsigned int _sampleRate, unsigned int _frameSize, const std::function<void(int16_t*, uint32_t)>&& callback);
};

extern CDeviceManager g_deviceManager;