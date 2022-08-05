#include "CSoundOutput.h"
#include "CVoiceException.h"

void CSoundOutput::OnPcmData(Sample* data, uint32_t framesCount)
{
	if (ringBuffer->BytesToRead() > 0)
	{
		ringBuffer->Read(data, FRAME_SIZE);
		GainPCM(data, framesCount, volume);
	}
	else memset(data, 0, FRAME_SIZE * sizeof(Sample));
}

CSoundOutput::CSoundOutput(int _sampleRate, int _framesPerBuffer, int _bitRate) :
	outputDevice(std::make_shared<RtAudio>(RtAudio::WINDOWS_DS)), sampleRate(_sampleRate), frameSize(_framesPerBuffer), bitRate(_bitRate)
{
	ringBuffer = new RingBuffer<Sample>(sampleRate * sizeof(Sample));
	
	auto ret = OpenOutputStream([this](Sample* data, uint32_t size) { this->OnPcmData(data, size); });
	if (ret != AltVoiceError::Ok)
		throw CVoiceException(ret);
}

CSoundOutput::~CSoundOutput()
{
	delete ringBuffer;
}

void CSoundOutput::ChangeOutputVolume(float _volume)
{
	volume = GetSignalMultiplierForVolume(_volume);
}

void CSoundOutput::Write(void* data, size_t size)
{
	ringBuffer->Write((Sample*)data, size);
}

std::vector<std::pair<int, std::string>> CSoundOutput::GetOutputDevices()
{
	std::vector<std::pair<int, std::string>> devices;
	for (auto i : outputDevice->getDeviceIds())
	{
		auto device = outputDevice->getDeviceInfo(i);
		if (device.outputChannels)
			devices.push_back(std::make_pair(i, device.name));
	}

	return devices;
}

void CSoundOutput::SetOutputDevice(int id)
{
	outputDeviceID = id;
	for (auto i : outputDevice->getDeviceIds())
	{
		auto device = outputDevice->getDeviceInfo(i);
		if (device.name == outputDeviceName)
			outputDeviceID = device.ID;
	}

	if (outputDevice->isStreamRunning() || outputDevice->isStreamOpen())
	{
		RestartOutputStream();
	}
}

int32_t CSoundOutput::GetOutputDevice()
{
	if (outputDeviceID == -1)
	{
		for (auto i : outputDevice->getDeviceIds())
		{
			auto device = outputDevice->getDeviceInfo(i);
			if (device.outputChannels)
			{
				outputDeviceID = device.ID;
				break;
			}
		}
	}
	return outputDeviceID;
}

void CSoundOutput::RestartOutputStream()
{
	if (outputDevice->isStreamRunning())
	{
		outputDevice->stopStream();
	}

	if (outputDevice->isStreamOpen())
	{
		outputDevice->closeStream();
	}

	OpenOutputStream(outputCallback);
}

int CSoundOutput::OutputDataCallback(void* outputBuffer, void* inputBuffer, unsigned int nFrames, double streamTime, RtAudioStreamStatus status, void* userData)
{
	CSoundOutput* mgr = (CSoundOutput*)userData;

	if (nFrames != mgr->frameSize)
		return 0;

	if (mgr->outputCallback)
		mgr->outputCallback((Sample*)outputBuffer, mgr->frameSize);

	return 0;
}

AltVoiceError CSoundOutput::OpenOutputStream(const std::function<void(Sample*, uint32_t)>& callback)
{
	outputCallback = callback;

	if (GetOutputDevice() == -1)
		return AltVoiceError::RtAudioError_MissingDevice;

	try {
		outputDevice->openStream(new RtAudio::StreamParameters{ (uint32_t)GetOutputDevice(), 1, 0 }, nullptr, RTAUDIO_SINT16, sampleRate, &frameSize, OutputDataCallback, this);
	}
	catch (std::exception& ex) {
		return AltVoiceError::RtAudioError_OpenStream;
	}

	if (outputDevice->startStream() != RtAudioErrorType::RTAUDIO_NO_ERROR)
		return AltVoiceError::RtAudioError_StartStream;

	return AltVoiceError::Ok;
}