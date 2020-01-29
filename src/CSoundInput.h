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
#include "rnnoise.h"

#include "WWMFResampler.h"
#include "WWMFResamplerCppIF.h"

#define FRAME_SIZE_OPUS 480
#define MAX_PACKET_SIZE 32768

using Sample = int16_t;

constexpr int RefTimesPerMillisec = 1;
constexpr int RefTimesPerSec = RefTimesPerMillisec * 1000;

class CSoundInput: public ISoundInput
{
	static constexpr int NORMALIZE_FRAME_COUNT = 20;

	void OnVoiceInput();

	WWMFPcmFormatMarshal inputFormat = { 0 };
	WWMFPcmFormatMarshal outputFormat = { 0 };
	int resamplerInstance = 0;
	BYTE resampledBuffer[0xFFFF];

	uint32_t _sampleRate;
	uint32_t _framesPerBuffer;
	uint32_t _bitRate;

	std::atomic<float> micGain = 1.f;

	OpusEncoder* enc = nullptr;

	OnVoiceCallback cb = nullptr;
	OnVoiceCallback rawCb = nullptr;
	RingBuffer<Sample> *_ringBuffer;

	//WASAPI
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	IAudioClient* pAudioClient = NULL;
	IAudioCaptureClient* pCaptureClient = NULL;
	WAVEFORMATEX* pwfx = NULL;

	//WASAPI recording stuff
	UINT32 numFramesAvailable;
	BYTE silence[512] = { 0 };
	UINT32 packetLength = 0;
	BYTE* pData;
	DWORD flags;

	//Record thread flags
	std::atomic<bool> inputActive = false;
	bool isInputReallyActive = false;
	std::atomic<bool> threadActive;

	//Temp packets
	BYTE packet[MAX_PACKET_SIZE] = { 0 };
	float floatBuffer[FRAME_SIZE_OPUS] = { 0 };
	//

	Sample opusInputFrameBuffer[FRAME_SIZE_OPUS];

	std::thread* inputStreamThread = nullptr;
	Sample* transferBuffer = nullptr;

	//WASAPI sleep time
	std::chrono::milliseconds sleepTime;

	std::mutex deviceMutex;

	//Denoiser
	std::atomic<bool> noiseSuppressionEnabled = false;
	DenoiseState* denoiseSt = nullptr;

	// Normalize stuff
	float normalizeMax = 0.f;

private:
	float LinearToDecibel(float linear);
	void GainPCM(Sample* data, size_t framesCount);
	void OnPcmData(BYTE* data, size_t size, size_t framesCount);
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
};

