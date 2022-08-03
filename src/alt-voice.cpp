#include "alt-voice.h"

#include "CSoundInput.h"
#include "COpusEncoder.h"
#include "COpusDecoder.h"
#include "CVoiceException.h"
#include "CDeviceManager.h"

std::vector<std::string> GetInputDevicesEnum()
{
	return g_deviceManager.GetInputDevices();
}

std::vector<std::string> GetOutputDevicesEnum()
{
	return g_deviceManager.GetInputDevices();
}

AltVoiceError CreateSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate, ISoundInput ** soundInput)
{
	try
	{
		*soundInput = new CSoundInput(deviceName, sampleRate, framesPerBuffer, bitrate);
	}
	catch (const CVoiceException & e)
	{
		return e.GetCode();
	}

	return AltVoiceError::Ok;
}

void DestroySoundInput(ISoundInput * _input)
{
	delete _input;
}

const char * GetVoiceErrorText(AltVoiceError error)
{
	switch (error)
	{
	case AltVoiceError::Ok:
		return "No error";
	case AltVoiceError::DeviceOpenError:
		return "Device open error";
	case AltVoiceError::ContextSetError:
		return "Context set error";
	case AltVoiceError::SourcesCreateError:
		return "Source create error";
	case AltVoiceError::BufferCreateError_InvalidName:
		return "Buffer create error. Invalid name";
	case AltVoiceError::BufferCreateError_InvalidEnum:
		return "Buffer create error. Invalid enum";
	case AltVoiceError::BufferCreateError_InvalidValue:
		return "Buffer create error. Invalid value";
	case AltVoiceError::BufferCreateError_InvalidOperation:
		return "Buffer create error. Invalid operation";
	case AltVoiceError::BufferCreateError_OutOfMemory:
		return "Buffer create error. Out of memory";
	case AltVoiceError::OpusEncoderCreateError:
		return "Encoder create error";
	case AltVoiceError::OpusDecoderCreateError:
		return "Decoder create error";
	case AltVoiceError::OpusBitrateSetError:
		return "Opus bitrate set error";
	case AltVoiceError::OpusSignalSetError:
		return "Opus signal set error";
	case AltVoiceError::DenoiseInitError:
		return "Denoiser init error";
	}
	return "Unknown error";
}

ALT_VOICE_API AltVoiceError CreateOpusEncoder(int sampleRate, int channelsCount, IOpusEncoder** opusEncoder, int bitRate)
{
	try
	{
		IOpusEncoder* encoder = new COpusEncoder(sampleRate, channelsCount, bitRate);
		*opusEncoder = encoder;
		return AltVoiceError::Ok;
	}
	catch (const CVoiceException &e)
	{
		return e.GetCode();
	}
}

ALT_VOICE_API AltVoiceError CreateOpusDecoder(int sampleRate, int channelsCount, IOpusDecoder** opusDecoder)
{
	try
	{
		IOpusDecoder* decoder = new COpusDecoder(sampleRate, channelsCount);
		*opusDecoder = decoder;
		return AltVoiceError::Ok;
	}
	catch (const CVoiceException & e)
	{
		return e.GetCode();
	}
}

ALT_VOICE_API void DestroyOpusEncoder(IOpusEncoder* opusEncoder)
{
	if (opusEncoder)
		delete opusEncoder;
}

ALT_VOICE_API void DestroyOpusDecoder(IOpusDecoder* opusDecoder)
{
	if (opusDecoder)
		delete opusDecoder;
}
