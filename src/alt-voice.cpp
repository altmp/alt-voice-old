#include "alt-voice.h"

#include "C3DSoundOutput.h"
#include "CSoundInput.h"
#include "CStreamPlayer.h"

I3DSoundOutput* output = nullptr;
ISoundInput* input = nullptr;


I3DSoundOutput * CreateSoundOutput(int sampleRate)
{
	if (!output)
		output = new C3DSoundOutput(sampleRate);
	return output;
}

ISoundInput * CreateSoundInput(int sampleRate, int framesPerBuffer, int bitrate)
{
	if (!input)
		input = new CSoundInput(sampleRate, framesPerBuffer, bitrate);
	return input;
}

void DestroySoundOutput(I3DSoundOutput * _output)
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
