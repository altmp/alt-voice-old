#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

#include <RtAudio.h>
#include <opus.h>

#include "ISoundOutput.h"
#include "CRingBuffer.h"
#include "VoiceError.h"

#include "helpers.h"
using namespace helpers;

class CSoundOutput : public ISoundOutput
{
	float volume = 1.f;
	OnVoiceCallback cb = nullptr;
	RingBuffer<Sample>* ringBuffer = nullptr;

	std::shared_ptr<RtAudio> outputDevice;
	std::string outputDeviceName;
	int32_t outputDeviceID = -1;

	uint32_t sampleRate;
	uint32_t frameSize;
	uint32_t bitRate;
	std::function<void(helpers::Sample*, uint32_t size)> outputCallback = nullptr;

	static int OutputDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData);
	void RestartOutputStream();
	AltVoiceError OpenOutputStream(const std::function<void(Sample*, uint32_t)>& callback);

	void OnPcmData(Sample* data, uint32_t framesCount);
public:
	CSoundOutput(int sampleRate, int framesPerBuffer, int bitrate);
	~CSoundOutput();

	void ChangeOutputVolume(float volume) override;
	void Write(void* data, size_t size) override;

	std::vector<std::pair<int, std::string>> GetOutputDevices() override;
	void SetOutputDevice(int id) override;
	int32_t GetOutputDevice() override;
};