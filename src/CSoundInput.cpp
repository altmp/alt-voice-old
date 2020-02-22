#include <chrono>
#include "CSoundInput.h"
#include "CVoiceException.h"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

#define EXIT_ON_ERROR(hres, error)  \
              if (FAILED(hres)) { lastError = error; goto Exit; }
template <class T> void SafeRelease(T * *ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

void CSoundInput::OnVoiceInput()
{
	UINT32 packetLength = 0;
	BYTE* pData;
	DWORD flags;
	HRESULT hr;

	while (threadActive)
	{
		if (inputActive && !isInputReallyActive)
		{
			hr = pAudioClient->Start();
			if (hr == S_OK)
			{
				resamplerInstance = WWMFResamplerInit(&inputFormat, &outputFormat, 60);
				isInputReallyActive = true;
			}
		}

		if (isInputReallyActive)
		{
			pCaptureClient->GetNextPacketSize(&packetLength);
			if (packetLength != 0)
			{
				// Get the available data in the shared buffer.
				pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);
				if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
				{
					int silenceSize = numFramesAvailable * pwfx->nBlockAlign;
					for (int i = 0; i < silenceSize; i += sizeof(silence)) // Tell CopyData to write silence.
					{
						int bufSize = silenceSize > sizeof(silence) ? sizeof(silence) : silenceSize;
						int resampledSize = sizeof(resampledBuffer);
						hr = WWMFResamplerResample(resamplerInstance, silence, bufSize, resampledBuffer, &resampledSize);
						if (!FAILED(hr) && resampledSize > 0)
							OnPcmData((BYTE*)resampledBuffer, resampledSize, resampledSize / sizeof(Sample));
						silenceSize -= bufSize;
					}
				}
				else
				{
					int resampledSize = sizeof(resampledBuffer);
					hr = WWMFResamplerResample(resamplerInstance, pData, pwfx->nBlockAlign * numFramesAvailable, resampledBuffer, &resampledSize);
					if (!FAILED(hr) && resampledSize > 0)
						OnPcmData(resampledBuffer, resampledSize, resampledSize / sizeof(Sample));
				}

				pCaptureClient->ReleaseBuffer(numFramesAvailable);
			}
		}

		if (!inputActive && isInputReallyActive)
		{
			hr = pAudioClient->Stop();
			if (hr == S_OK)
			{
				isInputReallyActive = false;
				int resampledSize = sizeof(resampledBuffer);
				hr = WWMFResamplerDrain(resamplerInstance, resampledBuffer, &resampledSize);
				if (!FAILED(hr) && resampledSize > 0)
					OnPcmData(resampledBuffer, resampledSize, resampledSize / sizeof(Sample));
				WWMFResamplerTerm(resamplerInstance);
				resamplerInstance = 0;
			}
		}

		std::this_thread::sleep_for(sleepTime);
	}
}

//https://forum.juce.com/t/float-to-decibel-conversion/1841
float CSoundInput::LinearToDecibel(float linear)
{
	if (linear != 0.0f)
		return 20.0f * log10(linear);
	else
		return -144.0f;  // effectively minus infinity
}

void CSoundInput::GainPCM(Sample* data, size_t framesCount)
{
	Sample maxFrame = 0;
	for (int i = 0; i < framesCount; ++i)
	{
		Sample s = abs(data[i]);
		if (s > maxFrame)
			maxFrame = s;
	}

	float maxPossibleGain = (MAXSHORT - 10) / (float)maxFrame;
	float gain = min(maxPossibleGain, micGain + 1);

	for (int i = 0; i < framesCount; ++i)
		data[i] *= gain;
}

void CSoundInput::OnPcmData(BYTE* data, size_t size, size_t framesCount)
{
	_ringBuffer->Write((const Sample*)data, framesCount);
	while (_ringBuffer->BytesToRead() >= FRAME_SIZE_OPUS)
	{
		_ringBuffer->Read((Sample*)opusInputFrameBuffer, FRAME_SIZE_OPUS);

		//micGain = 6;
		//GainPCM(opusInputFrameBuffer, FRAME_SIZE_OPUS);
		//if(micGain < 0.99 || micGain > 1.01) GainPCM(opusInputFrameBuffer, FRAME_SIZE_OPUS);

		if (noiseSuppressionEnabled)
			Denoise(opusInputFrameBuffer);

		int16_t micLevel = 0;
		for (int i = 0; i < FRAME_SIZE_OPUS; ++i)
		{
			if (opusInputFrameBuffer[i] > micLevel)
				micLevel = opusInputFrameBuffer[i];
		}

		Normalize(opusInputFrameBuffer, FRAME_SIZE_OPUS);

		if (rawCb) rawCb(opusInputFrameBuffer, FRAME_SIZE_OPUS * sizeof(Sample), (float)micLevel / MAXSHORT);

		int len = opus_encode(enc, opusInputFrameBuffer, FRAME_SIZE_OPUS, packet, MAX_PACKET_SIZE);

		if (len < 0 || len > MAX_PACKET_SIZE) return;
		if (cb) cb(packet, len, (float)micLevel / MAXSHORT);
	}
}

void CSoundInput::Denoise(Sample* buffer)
{
	if (denoiseSt)
	{
		// pcm / 2 is an epic workaround on RNNoise distortion
		for (int i = 0; i < FRAME_SIZE_OPUS; ++i) floatBuffer[i] = (float)buffer[i] / 2;
		rnnoise_process_frame(denoiseSt, floatBuffer, floatBuffer);
		for (int i = 0; i < FRAME_SIZE_OPUS; ++i) buffer[i] = floatBuffer[i] * 2;
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

	float gain = float(MAXSHORT - 10) / normalizeMax;
	gain = min(gain, 20);

	for (int i = 0; i < frameSize; ++i)
		buffer[i] *= gain;
}

CSoundInput::CSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate): _sampleRate(sampleRate), _framesPerBuffer(framesPerBuffer), _bitRate(bitrate)
{
	AltVoiceError lastError = AltVoiceError::Ok;
	HRESULT hr = S_OK;

	_ringBuffer = new RingBuffer<Sample>(sampleRate * 2);

	static bool _coInitialized = false;
	if (!_coInitialized)
	{
		hr = CoInitialize(NULL);
		EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);
		_coInitialized = true;
	}

	REFERENCE_TIME hnsRequestedDuration = RefTimesPerSec;
	REFERENCE_TIME hnsActualDuration;
	UINT32 bufferFrameCount;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	if (pwfx->wFormatTag == WAVE_FORMAT_PCM || pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{
		inputFormat.sampleFormat = pwfx->wFormatTag == WAVE_FORMAT_PCM ? WWMFBitFormatType::WWMFBitFormatInt : WWMFBitFormatType::WWMFBitFormatFloat;
		inputFormat.sampleRate = pwfx->nSamplesPerSec;
		inputFormat.nChannels = pwfx->nChannels;
		inputFormat.bits = pwfx->wBitsPerSample;
		inputFormat.validBitsPerSample = pwfx->wBitsPerSample;
	}
	else if (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
	{
		WAVEFORMATEXTENSIBLE* pWaveFormatExtensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pwfx);
		if (pWaveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
			inputFormat.sampleFormat = WWMFBitFormatType::WWMFBitFormatInt;
		else if (pWaveFormatExtensible->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
			inputFormat.sampleFormat = WWMFBitFormatType::WWMFBitFormatFloat;
		else
			EXIT_ON_ERROR(-1, AltVoiceError::DeviceOpenError);

		inputFormat.sampleRate = pWaveFormatExtensible->Format.nSamplesPerSec;
		inputFormat.nChannels = pWaveFormatExtensible->Format.nChannels;
		inputFormat.bits = pWaveFormatExtensible->Format.wBitsPerSample;
		inputFormat.validBitsPerSample = pWaveFormatExtensible->Samples.wValidBitsPerSample;
		inputFormat.dwChannelMask = pWaveFormatExtensible->dwChannelMask;
	}
	else
		EXIT_ON_ERROR(-1, AltVoiceError::DeviceOpenError);

	outputFormat.bits = sizeof(Sample) * 8;
	outputFormat.nChannels = 1;
	outputFormat.sampleFormat = WWMFBitFormatType::WWMFBitFormatInt;
	outputFormat.sampleRate = sampleRate;
	outputFormat.validBitsPerSample = outputFormat.bits;

	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);
	EXIT_ON_ERROR(hr, AltVoiceError::DeviceOpenError);

	hnsActualDuration = (double)RefTimesPerSec * bufferFrameCount / pwfx->nSamplesPerSec;
	sleepTime = std::chrono::milliseconds(hnsActualDuration / RefTimesPerMillisec / 8);

	int opusErr;
	enc = opus_encoder_create(_sampleRate, 1, OPUS_APPLICATION_VOIP, &opusErr);
	if (opusErr != OPUS_OK || enc == NULL)
		EXIT_ON_ERROR(-1, AltVoiceError::OpusEncoderCreateError);

	if (opus_encoder_ctl(enc, OPUS_SET_BITRATE(_bitRate)) != OPUS_OK)
		EXIT_ON_ERROR(-1, AltVoiceError::OpusBitrateSetError);

	if (opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_AUTO)) != OPUS_OK)
		EXIT_ON_ERROR(-1, AltVoiceError::OpusSignalSetError);

	opus_encoder_ctl(enc, OPUS_SET_INBAND_FEC(1));
	opus_encoder_ctl(enc, OPUS_SET_PACKET_LOSS_PERC(15));

	transferBuffer = new Sample[_framesPerBuffer];

	denoiseSt = rnnoise_create(NULL);
	if (!denoiseSt)
		EXIT_ON_ERROR(-1, AltVoiceError::DenoiseInitError);

	threadActive = true;
	inputStreamThread = new std::thread(&CSoundInput::OnVoiceInput, this);

Exit:
	if (lastError != AltVoiceError::Ok)
	{
		CoTaskMemFree(pwfx);
		SafeRelease(&pEnumerator);
		SafeRelease(&pDevice);
		SafeRelease(&pAudioClient);
		SafeRelease(&pCaptureClient);

		if (enc)
			opus_encoder_destroy(enc);

		if (denoiseSt)
			rnnoise_destroy(denoiseSt);

		throw CVoiceException(lastError);
	}
}


CSoundInput::~CSoundInput()
{
	threadActive = false;
	inputStreamThread->join();
	delete _ringBuffer;
	delete inputStreamThread;
	delete[] transferBuffer;

	CoTaskMemFree(pwfx);
	SafeRelease(&pEnumerator);
	SafeRelease(&pDevice);
	SafeRelease(&pAudioClient);
	SafeRelease(&pCaptureClient);

	opus_encoder_destroy(enc);

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
	micGain = LinearToDecibel(gain);
}

bool CSoundInput::ChangeDevice(char * deviceName)
{
	/*std::unique_lock<std::mutex> _deviceLock(deviceMutex);
	alcCaptureCloseDevice(inputDevice);

	inputDevice = alcCaptureOpenDevice(deviceName, _sampleRate, AL_FORMAT_MONO_FLOAT32, _framesPerBuffer);
	if (!inputDevice)
		return false;*/
	return true;
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
