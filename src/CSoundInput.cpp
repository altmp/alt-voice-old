#include "CSoundInput.h"
#include <thread>
#include <chrono>

void CSoundInput::OnVoiceInput()
{
	static int catchedFrames = 0;
	static Sample opusFrameBuffer[FRAME_SIZE_OPUS];
	static unsigned char packet[MAX_PACKET_SIZE];

	for (;;)
	{
		alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, &catchedFrames);
		if (catchedFrames >= _framesPerBuffer)
		{
			alcCaptureSamples(inputDevice, transferBuffer, _framesPerBuffer);
			float micLevel = -1.f;
			for (int i = 0; i < _framesPerBuffer; ++i)
			{
				if (transferBuffer[i] > micLevel)
					micLevel = transferBuffer[i];
			}
			
			for (int i = 0; i < _framesPerBuffer; ++i)
				transferBuffer[i] = transferBuffer[i] * micGain;

			_ringBuffer.Write((const Sample*)transferBuffer, _framesPerBuffer);

			while (_ringBuffer.BytesToRead() >= FRAME_SIZE_OPUS)
			{
				_ringBuffer.Read((Sample*)opusFrameBuffer, FRAME_SIZE_OPUS);
				int len = opus_encode_float(enc, opusFrameBuffer, FRAME_SIZE_OPUS, packet, MAX_PACKET_SIZE);

				if (len < 0 || len > MAX_PACKET_SIZE)
					return;

				if (cb)
					cb(packet, len, micLevel);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
	}
}

CSoundInput::CSoundInput(int sampleRate, int framesPerBuffer, int bitrate): _sampleRate(sampleRate), _framesPerBuffer(framesPerBuffer), _bitRate(bitrate)
{
	sleepTime = (framesPerBuffer / (sampleRate / 1000)) / 2;

	inputDevice = alcCaptureOpenDevice(NULL, sampleRate, AL_FORMAT_MONO_FLOAT32, framesPerBuffer);

	if (!inputDevice)
		throw std::runtime_error("Capture device open error");

	inputStreamThread = new std::thread(&CSoundInput::OnVoiceInput, this);
	inputStreamThread->detach();

	int opusErr;
	enc = opus_encoder_create(_sampleRate, 1, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK || enc == NULL)
		throw std::runtime_error("Opus encoder create error");

	if (opus_encoder_ctl(enc, OPUS_SET_BITRATE(_bitRate)) != OPUS_OK)
		throw std::runtime_error("Opus set bitrate error");

	transferBuffer = new Sample[_framesPerBuffer];
}


CSoundInput::~CSoundInput()
{
	delete inputStreamThread;

	alcCaptureCloseDevice(inputDevice);
	opus_encoder_destroy(enc);
}

bool CSoundInput::EnableInput()
{
	if (!inputActive)
	{
		inputActive = true;
		alcCaptureStart(inputDevice);
		return true;
	}
	return false;
}

bool CSoundInput::DisableInput()
{
	if (inputActive)
	{
		inputActive = false;
		alcCaptureStop(inputDevice);
		return true;
	}
	return false;
}

void CSoundInput::ChangeMicGain(float gain)
{
	micGain = gain;
}

void CSoundInput::RegisterCallback(OnVoiceCallback callback)
{
	cb = callback;
}
