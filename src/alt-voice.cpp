#include "alt-voice.h"

#include "CSoundOutput.h"
#include "CSoundInput.h"
#include "CStreamPlayer.h"
#include "COpusEncoder.h"
#include "COpusDecoder.h"
#include "CVoiceException.h"

//ISoundOutput* output = nullptr;
ISoundInput* input = nullptr;

char* GetInputDevicesEnum()
{
	/*char *devName = nullptr;
	if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE)
		devName = (char *)alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	return devName;*/
	return "";
}

char* GetOutputDevicesEnum()
{
	/*char *devName = nullptr;
	if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE)
	{
		if (alcIsExtensionPresent(NULL, "ALC_enumerate_all_EXT") == AL_FALSE)
			devName = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		else
			devName = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	}
	return devName;*/
	return nullptr;
}

char * GetNextDevice(char ** enumerator)
{
	/*if(!enumerator || !*enumerator)
		return nullptr;

	char* deviceName = *enumerator;

	if (deviceName && deviceName != '\0')
	{
		size_t devNameLength = strlen(deviceName);
		*enumerator += (devNameLength + 1);
		return deviceName;
	}
	else
		return nullptr;*/
	return nullptr;
}

//AltVoiceError CreateSoundOutput(char* deviceName, int sampleRate, int sourcesCount, ISoundOutput ** soundOutput)
//{
//	if (!output)
//	{
//		try 
//		{
//			output = new CSoundOutput(deviceName, sampleRate, sourcesCount);
//		}
//		catch (const CVoiceException& e)
//		{
//			return e.GetCode();
//		}
//	}
//	*soundOutput = output;
//	return AltVoiceError::Ok;
//}

AltVoiceError CreateSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate, ISoundInput ** soundInput)
{
	if (!input)
	{
		try
		{
			input = new CSoundInput(deviceName, sampleRate, framesPerBuffer, bitrate);
		}
		catch (const CVoiceException& e)
		{
			return e.GetCode();
		}
	}
	*soundInput = input;
	return AltVoiceError::Ok;
}

//void DestroySoundOutput(ISoundOutput * _output)
//{
//	if (output == _output)
//		delete output;
//	output = nullptr;
//}

void DestroySoundInput(ISoundInput * _input)
{
	if (input == _input)
		delete input;
	input = nullptr;
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

ALT_VOICE_API AltVoiceError CreateOpusEncoder(int sampleRate, int channelsCount, IOpusEncoder** opusEncoder)
{
	try
	{
		IOpusEncoder* encoder = new COpusEncoder(sampleRate, channelsCount);
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
