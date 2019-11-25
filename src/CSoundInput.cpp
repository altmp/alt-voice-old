#include <chrono>
#include "CSoundInput.h"
#include "CVoiceException.h"

void CSoundInput::OnVoiceInput()
{
	static uint32_t catchedFrames = 0;
	static Sample opusFrameBuffer[FRAME_SIZE_OPUS];
	static unsigned char packet[MAX_PACKET_SIZE];
	static bool isBufferCaptured = false;

	while(threadAlive)
	{
		isBufferCaptured = false;
		{
			std::unique_lock<std::mutex> _deviceLock(deviceMutex);
			if (!inputDevice)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
			alcGetIntegerv(inputDevice, ALC_CAPTURE_SAMPLES, 1, (ALCint*)&catchedFrames);
			if (catchedFrames >= _framesPerBuffer)
			{
				alcCaptureSamples(inputDevice, transferBuffer, _framesPerBuffer);
				isBufferCaptured = true;
			}
		}

		if(isBufferCaptured)
		{
			float micLevel = -1.f;
			for (uint32_t i = 0; i < _framesPerBuffer; ++i)
			{
				if (transferBuffer[i] > micLevel)
					micLevel = transferBuffer[i];
			}
			
			{
				std::unique_lock<std::mutex> _micGainLock(micGainMutex);

				Sample highestSample = 0;
				bool highestFound = false;
				for (uint32_t i = 0; i < _framesPerBuffer; ++i)
				{
					if (!highestFound)
					{
						highestFound = true;
						highestSample = abs(transferBuffer[i]);
					}
					else if (abs(transferBuffer[i]) > highestSample)
						highestSample = abs(transferBuffer[i]);
				}
				float highestPossibleMultiplier = (float)1.0f / highestSample;
				if (micGain > highestPossibleMultiplier)
					micGain = highestPossibleMultiplier;

				for (uint32_t i = 0; i < _framesPerBuffer; ++i)
					transferBuffer[i] = transferBuffer[i] * micGain;
			}
			

			_ringBuffer.Write((const Sample*)transferBuffer, _framesPerBuffer);

			while (_ringBuffer.BytesToRead() >= FRAME_SIZE_OPUS)
			{
				_ringBuffer.Read((Sample*)opusFrameBuffer, FRAME_SIZE_OPUS);

				{
					std::unique_lock<std::mutex> _deviceLock(noiseSuppressionMutex);


					if (noiseSuppressionEnabled && denoiseSt)
					{
						for (int i = 0; i < FRAME_SIZE_OPUS; ++i) opusFrameBuffer[i] *= 32768.0;
						rnnoise_process_frame(denoiseSt, opusFrameBuffer, opusFrameBuffer);
						for (int i = 0; i < FRAME_SIZE_OPUS; ++i) opusFrameBuffer[i] /= 32768.0;
					}

				}

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

CSoundInput::CSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate): _sampleRate(sampleRate), _framesPerBuffer(framesPerBuffer), _bitRate(bitrate)
{
	inputDevice = alcCaptureOpenDevice(deviceName, sampleRate, AL_FORMAT_MONO_FLOAT32, framesPerBuffer);

	if (!inputDevice)
		throw CVoiceException(AltVoiceError::DeviceOpenError);

	inputStreamThread = new std::thread(&CSoundInput::OnVoiceInput, this);
	inputStreamThread->detach();

	sleepTime = (framesPerBuffer / (sampleRate / 1000)) / 2;

	int opusErr;
	enc = opus_encoder_create(_sampleRate, 1, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK || enc == NULL)
		throw CVoiceException(AltVoiceError::OpusEncoderCreateError);

	if (opus_encoder_ctl(enc, OPUS_SET_BITRATE(_bitRate)) != OPUS_OK)
		throw CVoiceException(AltVoiceError::OpusBitrateSetError);

	transferBuffer = new Sample[_framesPerBuffer];

	denoiseSt = rnnoise_create(NULL);
	if (!denoiseSt)
		throw CVoiceException(AltVoiceError::DenoiseInitError);
}


CSoundInput::~CSoundInput()
{
	threadAlive = false;
	delete inputStreamThread;

	alcCaptureCloseDevice(inputDevice);
	opus_encoder_destroy(enc);

	rnnoise_destroy(denoiseSt);
}

bool CSoundInput::EnableInput()
{
	if (!inputActive)
	{
		std::unique_lock<std::mutex> _deviceLock(deviceMutex);
		if (inputDevice)
		{
			inputActive = true;
			alcCaptureStart(inputDevice);
			return true;
		}
	}
	return false;
}

bool CSoundInput::DisableInput()
{
	if (inputActive)
	{
		std::unique_lock<std::mutex> _deviceLock(deviceMutex);
		inputActive = false;
		if(inputDevice)
			alcCaptureStop(inputDevice);
		return true;
	}
	return false;
}

void CSoundInput::ChangeMicGain(float gain)
{
	std::unique_lock<std::mutex> _micGainLock(micGainMutex);
	micGain = gain;
}

bool CSoundInput::ChangeDevice(char * deviceName)
{
	std::unique_lock<std::mutex> _deviceLock(deviceMutex);
	alcCaptureCloseDevice(inputDevice);

	inputDevice = alcCaptureOpenDevice(deviceName, _sampleRate, AL_FORMAT_MONO_FLOAT32, _framesPerBuffer);
	if (!inputDevice)
		return false;
	return true;
}

void CSoundInput::RegisterCallback(OnVoiceCallback callback)
{
	cb = callback;
}

void CSoundInput::SetNoiseSuppressionStatus(bool enabled)
{
	std::unique_lock<std::mutex> _deviceLock(noiseSuppressionMutex);
	noiseSuppressionEnabled = enabled;
}
