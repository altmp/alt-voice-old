#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

#include <RtAudio.h>
#include <opus.h>

#include "ISoundInput.h"
#include "CRingBuffer.h"
#include "COpusEncoder.h"
#include "rnnoise.h"
#include "VoiceError.h"

#include "helpers.h"
using namespace helpers;

class CSoundInput: public ISoundInput
{
	static constexpr int NORMALIZE_FRAME_COUNT = 20;

	uint32_t sampleRate;
	uint32_t frameSize;
	uint32_t bitRate;

	std::atomic<float> micGain = 1.f;

	COpusEncoder* enc = nullptr;

	OnVoiceCallback cb = nullptr;
	OnVoiceCallback rawCb = nullptr;
	RingBuffer<Sample>* ringBuffer = nullptr;

	//Record thread flags
	std::atomic<bool> inputActive = false;

	//Temp packets
	BYTE packet[MAX_PACKET_SIZE] = { 0 };
	float floatBuffer[FRAME_SIZE] = { 0 };

	Sample opusInputFrameBuffer[FRAME_SIZE];

	//Denoiser
	std::atomic<bool> noiseSuppressionEnabled = false;
	DenoiseState* denoiseSt = nullptr;

	// Normalize stuff
	std::atomic<bool> normalizationEnabled = true;
	float normalizeMax = 0.f;

	//Device stuff
	std::shared_ptr<RtAudio> inputDevice;
	std::string inputDeviceName;
	int32_t inputDeviceID = -1;
	std::function<void(Sample*, uint32_t size)> inputCallback = nullptr;

private:
	void OnPcmData(Sample* data, uint32_t framesCount);
	void Denoise(Sample* buffer);
	void Normalize(Sample* buffer, size_t frameSize);

	//Device stuff
	static int InputDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData);
	void RestartInputStream();
	AltVoiceError OpenInputStream(const std::function<void(Sample*, uint32_t)>& callback);

public:
	CSoundInput(int _sampleRate, int _framesPerBuffer, int _bitRate);
	~CSoundInput();

	bool EnableInput() override;
	bool DisableInput() override;
	void ChangeMicGain(float gain) override;
	void RegisterCallback(OnVoiceCallback callback) override;
	void RegisterRawCallback(OnVoiceCallback callback) override;
	void SetNoiseSuppressionStatus(bool enabled) override;
	void SetNormalizationEnabled(bool enabled)override;

	std::vector<std::pair<int, std::string>> GetInputDevices() override;
	void SetInputDevice(int id) override;
	int32_t GetInputDevice() override;
};

