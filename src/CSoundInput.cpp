#include "CSoundInput.h"
#include <iostream>

int CSoundInput::OnInputCallback(const void * inputBuffer, void * outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo * timeInfo, PaStreamCallbackFlags statusFlags, void * userData)
{
	CSoundInput* input = (CSoundInput*)userData;

	static Sample opusFrameBuffer[FRAME_SIZE_OPUS];
	unsigned char packet[MAX_PACKET_SIZE];

	const Sample* pcm = (const Sample*)inputBuffer;
	float maxVal = -1.f;
	for (unsigned long i = 0; i < framesPerBuffer; ++i)
	{
		if (pcm[i] > maxVal)
			maxVal = pcm[i];
	}

	for (int i = 0; i < framesPerBuffer; ++i)
		((Sample*)inputBuffer)[i] = ((Sample*)inputBuffer)[i] * input->micGain;

	input->_ringBuffer.Write((const Sample*)inputBuffer, framesPerBuffer);

	while (input->_ringBuffer.BytesToRead() >= FRAME_SIZE_OPUS)
	{
		input->_ringBuffer.Read((Sample*)opusFrameBuffer, FRAME_SIZE_OPUS);
		int len = opus_encode_float(input->enc, opusFrameBuffer, FRAME_SIZE_OPUS, packet, MAX_PACKET_SIZE);

		if (len < 0 || len > MAX_PACKET_SIZE)
			return PaStreamCallbackResult::paAbort;

		if (input->cb)
			input->cb(packet, len, maxVal);
	}

	return PaStreamCallbackResult::paContinue;
}

CSoundInput::CSoundInput(int sampleRate, int framesPerBuffer, int bitrate): _sampleRate(sampleRate), _framesPerBuffer(framesPerBuffer), _bitRate(bitrate)
{
	setlocale(LC_ALL, "ru_RU");
	PaError err = Pa_Initialize();
	if (err != paNoError)
		throw std::runtime_error("PortAudio initialization error");

	PaStreamParameters inputParameters;

	PaDeviceIndex device = Pa_GetDefaultInputDevice();

	if (device == paNoDevice)
		throw std::runtime_error("Default input device not found");

	const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(device);
	std::cout << deviceInfo->name << std::endl;

	inputParameters.device = device;
	inputParameters.channelCount = 1;
	inputParameters.sampleFormat = paFloat32;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,
		sampleRate,
		_framesPerBuffer,
		paClipOff | paDitherOff,
		OnInputCallback,
		this);
	if (err != paNoError)
	{
		const char* errorText = Pa_GetErrorText(err);
		throw std::runtime_error(errorText);
	}

	int opusErr;
	enc = opus_encoder_create(_sampleRate, 1, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK || enc == NULL)
		throw std::runtime_error("Opus encoder create error");

	if (opus_encoder_ctl(enc, OPUS_SET_BITRATE(_bitRate)) != OPUS_OK)
		throw std::runtime_error("Opus set bitrate error");
}


CSoundInput::~CSoundInput()
{
	if (stream)
	{
		if(Pa_IsStreamActive(stream))
			Pa_AbortStream(stream);
		Pa_CloseStream(stream);
		stream = nullptr;
	}
	Pa_Terminate();
	opus_encoder_destroy(enc);
}

bool CSoundInput::EnableInput()
{
	int stopped = Pa_IsStreamStopped(stream);
	if (stopped == 1)
	{
		PaError err = Pa_StartStream(stream);
		return err == paNoError;
	}
	return false;
}

bool CSoundInput::DisableInput()
{
	int streamActive = Pa_IsStreamActive(stream);
	if (streamActive == 1)
	{
		PaError err = Pa_StopStream(stream);
		return err == paNoError;
	}
	return false;
}

void CSoundInput::ChangeMicGain(float gain)
{
	micGain = gain;
}

float CSoundInput::GetCPULoad()
{
	return (float)Pa_GetStreamCpuLoad(stream);
}

void CSoundInput::RegisterCallback(OnVoiceCallback callback)
{
	cb = callback;
}
