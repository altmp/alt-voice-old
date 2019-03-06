#include "alt-voice.h"

#include "CSoundOutput.h"
#include "CSoundInput.h"
#include "CStreamPlayer.h"
#include "CVoiceException.h"

ISoundOutput* output = nullptr;
ISoundInput* input = nullptr;

ALT_VOICE_API char* GetInputDevicesEnum()
{
	char *devName = nullptr;
	if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE)
		devName = (char *)alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
	return devName;
}

ALT_VOICE_API char* GetOutputDevicesEnum()
{
	char *devName = nullptr;
	if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE)
	{
		if (alcIsExtensionPresent(NULL, "ALC_enumerate_all_EXT") == AL_FALSE)
			devName = (char *)alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		else
			devName = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
	}
	return devName;
}

ALT_VOICE_API char * GetNextDevice(char ** enumerator)
{
	if(!enumerator || !*enumerator)
		return nullptr;

	char* deviceName = *enumerator;

	if (deviceName && deviceName != '\0')
	{
		size_t devNameLength = strlen(deviceName);
		*enumerator += (devNameLength + 1);
		return deviceName;
	}
	else
		return nullptr;
}

AltVoiceError CreateSoundOutput(char* deviceName, int sampleRate, int sourcesCount, ISoundOutput ** soundOutput)
{
	if (!output)
	{
		try 
		{
			output = new CSoundOutput(deviceName, sampleRate, sourcesCount);
		}
		catch (const CVoiceException& e)
		{
			return e.GetCode();
		}
	}
	*soundOutput = output;
	return AltVoiceError::Ok;
}

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

void DestroySoundOutput(ISoundOutput * _output)
{
	if (output == _output)
		delete output;
	output = nullptr;
}

void DestroySoundInput(ISoundInput * _input)
{
	if (input == _input)
		delete input;
	input = nullptr;
}
