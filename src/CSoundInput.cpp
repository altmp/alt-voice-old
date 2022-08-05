#include <chrono>
#include <fstream>
#include "CSoundInput.h"
#include "CVoiceException.h"

void CSoundInput::OnPcmData(Sample* data, uint32_t framesCount)
{
	if (!inputActive)
		return;

	ringBuffer->Write((const Sample*)data, framesCount);
	while (ringBuffer->BytesToRead() >= FRAME_SIZE)
	{
		ringBuffer->Read((Sample*)opusInputFrameBuffer, FRAME_SIZE);

		if (noiseSuppressionEnabled)
			Denoise(opusInputFrameBuffer);

		Sample micLevel = 0;
		for (int i = 0; i < FRAME_SIZE; ++i)
		{
			if (opusInputFrameBuffer[i] > micLevel)
				micLevel = opusInputFrameBuffer[i];
		}
		
		if (normalizationEnabled)
			Normalize(opusInputFrameBuffer, FRAME_SIZE);
		
		GainPCM(opusInputFrameBuffer, FRAME_SIZE, micGain);

		if (rawCb) rawCb(opusInputFrameBuffer, FRAME_SIZE * sizeof(Sample), (float)micLevel / MAXSHORT);

		int len = enc->EncodeShort(opusInputFrameBuffer, FRAME_SIZE, packet, MAX_PACKET_SIZE);

		if (len < 0 || len > MAX_PACKET_SIZE) return;
		if (cb) cb(packet, len, (float)micLevel / MAXSHORT);
	}
}

void CSoundInput::Denoise(Sample* buffer)
{
	if (denoiseSt)
	{
		// pcm / 2 is an epic workaround on RNNoise distortion
		for (int i = 0; i < FRAME_SIZE; ++i) floatBuffer[i] = buffer[i] / 2;

		for (int i = 0; i < FRAME_SIZE; i += FRAME_SIZE / sizeof(Sample))
			rnnoise_process_frame(denoiseSt, floatBuffer + i, floatBuffer + i);

		for (int i = 0; i < FRAME_SIZE; ++i) buffer[i] = floatBuffer[i] * 2;
	}
}

void CSoundInput::Normalize(Sample* buffer, size_t frameSize)
{
	Sample maxFrame = 0;
	for (int i = 0; i < frameSize; ++i)
	{
		Sample s = abs(buffer[i]);
		if (s > maxFrame)
			maxFrame = s;
	}

	if (normalizeMax == 0.f || maxFrame > normalizeMax || normalizeMax / maxFrame < 0.5)
		normalizeMax = maxFrame;
	else
		normalizeMax = (normalizeMax * (NORMALIZE_FRAME_COUNT - 1) + maxFrame) / NORMALIZE_FRAME_COUNT;

	if (normalizeMax <= 1.f)
		return;

	float gain = MAXSHORT / normalizeMax / 2;
	gain = std::fmin<float, float>(gain, 10);

	for (int i = 0; i < frameSize; ++i)
		buffer[i] *= gain;
}

CSoundInput::CSoundInput(int _sampleRate, int _framesPerBuffer, int _bitRate) : 
	sampleRate(_sampleRate), frameSize(_framesPerBuffer), bitRate(_bitRate), inputDevice(std::make_shared<RtAudio>(RtAudio::WINDOWS_DS))
{
	ringBuffer = new RingBuffer<Sample>(sampleRate * sizeof(Sample));

	//create input device here -------------------------------------------------------------------------
	auto ret = OpenInputStream([this](Sample* data, uint32_t size) { this->OnPcmData(data, size); });
	if(ret != AltVoiceError::Ok)
		throw CVoiceException(ret);

	try {
		enc = new COpusEncoder(sampleRate, 1, bitRate);
	}
	catch (const CVoiceException& e) {
		throw CVoiceException(e.GetCode());
	}

	denoiseSt = rnnoise_create(NULL);
	if (!denoiseSt)
		throw CVoiceException(AltVoiceError::DenoiseInitError);
}

CSoundInput::~CSoundInput()
{
	delete ringBuffer;

	if (enc)
		delete enc;

	if (denoiseSt)
		rnnoise_destroy(denoiseSt);
}

bool CSoundInput::EnableInput()
{
	if (!inputActive)
	{
		inputActive = true;
		return true;
	}
	return false;
}

bool CSoundInput::DisableInput()
{
	if (inputActive)
	{
		inputActive = false;
		return true;
	}
	return false;
}

void CSoundInput::ChangeMicGain(float gain)
{
	micGain = GetSignalMultiplierForVolume(gain);
}

void CSoundInput::RegisterCallback(OnVoiceCallback callback)
{
	cb = callback;
}

void CSoundInput::RegisterRawCallback(OnVoiceCallback callback)
{
	rawCb = callback;
}

void CSoundInput::SetNoiseSuppressionStatus(bool enabled)
{
	noiseSuppressionEnabled = enabled;
}

void CSoundInput::SetNormalizationEnabled(bool enabled)
{
	normalizationEnabled = enabled;
	normalizeMax = 0.f;
}

std::vector<std::pair<int, std::string>> CSoundInput::GetInputDevices()
{
	std::vector<std::pair<int, std::string>> devices;
	for (auto i : inputDevice->getDeviceIds())
	{
		auto device = inputDevice->getDeviceInfo(i);
		if (device.inputChannels)
			devices.push_back(std::make_pair(i, device.name));
	}

	return devices;
}

void CSoundInput::SetInputDevice(int id)
{
	inputDeviceID = id;
	for (auto i : inputDevice->getDeviceIds())
	{
		auto device = inputDevice->getDeviceInfo(i);
		if (device.name == inputDeviceName)
			inputDeviceID = device.ID;
	}

	if (inputDevice->isStreamRunning() || inputDevice->isStreamOpen())
	{
		RestartInputStream();
	}
}

int32_t CSoundInput::GetInputDevice()
{
	if (inputDeviceID == -1)
	{
		for (auto i : inputDevice->getDeviceIds())
		{
			auto device = inputDevice->getDeviceInfo(i);
			if (device.inputChannels)
			{
				inputDeviceID = device.ID;
				break;
			}
		}
	}
	return inputDeviceID;
}

void CSoundInput::RestartInputStream()
{
	if (inputDevice->isStreamRunning())
	{
		inputDevice->stopStream();
	}

	if (inputDevice->isStreamOpen())
	{
		inputDevice->closeStream();
	}

	OpenInputStream(inputCallback);
}

int CSoundInput::InputDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
	CSoundInput* mgr = (CSoundInput*)userData;

	if (nFrames != mgr->frameSize)
		return 0;

	if (mgr->inputCallback)
		mgr->inputCallback((Sample*)inputBuffer, mgr->frameSize);

	return 0;
}

AltVoiceError CSoundInput::OpenInputStream(const std::function<void(Sample*, uint32_t)>& callback)
{
	inputCallback = callback;

	if (GetInputDevice() == -1)
		return AltVoiceError::RtAudioError_MissingDevice;

	try {
		inputDevice->openStream(nullptr, new RtAudio::StreamParameters{ (uint32_t)GetInputDevice(), 1, 0 }, RTAUDIO_SINT16, sampleRate, &frameSize, InputDataCallback, this);
	}
	catch (std::exception& ex) {
		return AltVoiceError::RtAudioError_OpenStream;
	}

	if (inputDevice->startStream() != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return AltVoiceError::RtAudioError_StartStream;

	return AltVoiceError::Ok;
}