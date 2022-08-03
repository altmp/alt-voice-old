#include "CDeviceManager.h"

CDeviceManager g_deviceManager;

std::vector<std::string> CDeviceManager::GetInputDevices()
{
    std::vector<std::string> devices;
    for (auto i : rtAudio->getDeviceIds())
    {
        auto device = rtAudio->getDeviceInfo(i);
        if (device.inputChannels)
            devices.push_back(device.name);
    }

    return devices;
}

std::vector<std::string> CDeviceManager::GetOutputDevices()
{
    std::vector<std::string> devices;
    for (auto i : rtAudio->getDeviceIds())
    {
        auto device = rtAudio->getDeviceInfo(i);
        if (device.outputChannels)
            devices.push_back(device.name);
    }

    return devices;
}

void CDeviceManager::SetInputDevice(const char* name)
{
	inputDeviceName = name;
	for (auto i : rtAudio->getDeviceIds())
	{
		auto device = rtAudio->getDeviceInfo(i);
		if (device.name == inputDeviceName)
			inputDeviceID = device.ID;
	}
}

int32_t CDeviceManager::GetInputDevivce()
{
	if (inputDeviceID == -1)
	{
		for (auto i : rtAudio->getDeviceIds())
		{
			auto device = rtAudio->getDeviceInfo(i);
			if (device.inputChannels)
			{
				inputDeviceID = device.ID;
				break;
			}
		}
	}
	return inputDeviceID;
}

int CDeviceManager::DeviceDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
	CDeviceManager* mgr = (CDeviceManager*)userData;

	if (nFrames != mgr->frameSize)
		return 0;

	if (mgr->inputCallback)
		mgr->inputCallback((int16_t*)inputBuffer, mgr->frameSize);

	return 0;
}

AltVoiceError CDeviceManager::OpenInputStream(unsigned int _sampleRate, unsigned int _frameSize, const std::function<void(int16_t*, uint32_t)>&& callback)
{
	frameSize = _frameSize;
	inputCallback = callback;

	if (GetInputDevivce() == -1)
		return AltVoiceError::RtAudioError_MissingDevice;

	try {
		rtAudio->openStream(nullptr, new RtAudio::StreamParameters{ (uint32_t)GetInputDevivce(), 1, 0 }, RTAUDIO_SINT16, _sampleRate, &frameSize, DeviceDataCallback, this);
	}
	catch (std::exception& ex) {
		return AltVoiceError::RtAudioError_OpenStream;
	}

	if (rtAudio->startStream() != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return AltVoiceError::RtAudioError_StartStream;

	return AltVoiceError::Ok;
}