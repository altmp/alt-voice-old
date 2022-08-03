#pragma once
#include <thread>
#include <mutex>
#include <atomic>

#include <mmdeviceapi.h>
#include <uuids.h>
#include <audioclient.h>
#include <mftransform.h>
#include <mfapi.h>
#include <mferror.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <wmcodecdsp.h>

#pragma comment(lib, "ole32.lib")

#include <opus.h>

#include "ISoundInput.h"
#include "CRingBuffer.h"
#include "COpusEncoder.h"
#include "CDeviceManager.h"
#include "rnnoise.h"

#define FRAME_SIZE_OPUS 960
#define MAX_PACKET_SIZE 32768

using Sample = int16_t;

constexpr int RefTimesPerMillisec = 1;
constexpr int RefTimesPerSec = RefTimesPerMillisec * 1000;

class CSoundInput: public ISoundInput
{
	static constexpr int NORMALIZE_FRAME_COUNT = 20;

	uint32_t _sampleRate;
	uint32_t _framesPerBuffer;
	uint32_t _bitRate;

	std::atomic<float> micGain = 1.f;

	COpusEncoder* enc = nullptr;

	OnVoiceCallback cb = nullptr;
	OnVoiceCallback rawCb = nullptr;
	RingBuffer<Sample> *_ringBuffer;

	//Record thread flags
	std::atomic<bool> inputActive = false;
	bool isInputReallyActive = false;

	//Temp packets
	BYTE packet[MAX_PACKET_SIZE] = { 0 };
	float floatBuffer[FRAME_SIZE_OPUS] = { 0 };

	Sample opusInputFrameBuffer[FRAME_SIZE_OPUS];

	//Denoiser
	std::atomic<bool> noiseSuppressionEnabled = false;
	DenoiseState* denoiseSt = nullptr;

	// Normalize stuff
	bool normalizationEnabled = true;
	float normalizeMax = 0.f;

private:
	float LinearToDecibel(float linear);
	void GainPCM(Sample* data, size_t framesCount);
	void OnPcmData(int16_t* data, uint32_t framesCount);
	void Denoise(Sample* buffer);
	void Normalize(Sample* buffer, size_t frameSize);

public:
	CSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate);
	~CSoundInput();

	bool EnableInput() override;
	bool DisableInput() override;
	void ChangeMicGain(float gain) override;
	bool ChangeDevice(char* deviceName) override;
	void RegisterCallback(OnVoiceCallback callback) override;
	void RegisterRawCallback(OnVoiceCallback callback) override;
	void SetNoiseSuppressionStatus(bool enabled) override;
	void SetNormalizationEnabled(bool enabled)override;
};

