#include "C3DSoundOutput.h"
#include "CStreamPlayer.h"

C3DSoundOutput::C3DSoundOutput(int sampleRate) : _sampleRate(sampleRate)
{
	const ALCchar *name;
	ALCdevice *device;
	ALCcontext *ctx;

	device = alcOpenDevice(NULL);
	if (!device)
		throw std::runtime_error("Could not open a device!");

	ctx = alcCreateContext(device, NULL);
	if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
	{
		if (ctx != NULL)
			alcDestroyContext(ctx);
		alcCloseDevice(device);
		throw std::runtime_error("Could not set a context!");
	}

	name = NULL;
	if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(device) != AL_NO_ERROR)
		name = alcGetString(device, ALC_DEVICE_SPECIFIER);

	int opusErr;
	dec = opus_decoder_create(_sampleRate, 1, &opusErr);
	if (opusErr != OPUS_OK || dec == NULL)
		throw std::runtime_error("Opus decoder create error");
}


C3DSoundOutput::~C3DSoundOutput()
{
	for (IStreamPlayer* p : _streamPlayers)
		delete p;
	_streamPlayers.clear();

	ALCdevice *device;
	ALCcontext *ctx;

	ctx = alcGetCurrentContext();
	if (ctx == NULL)
		return;

	device = alcGetContextsDevice(ctx);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	alcCloseDevice(device);
}

void C3DSoundOutput::SetMyPosition(float x, float y, float z)
{
	listenerPos[0] = x;
	listenerPos[1] = y;
	listenerPos[2] = z;
}

void C3DSoundOutput::SetMyVelocity(float x, float y, float z)
{
	listenerVel[0] = x;
	listenerVel[1] = y;
	listenerVel[2] = z;
}

void C3DSoundOutput::SetMyOrientationFront(float x, float y, float z)
{
	listenerOri[0] = x;
	listenerOri[1] = y;
	listenerOri[2] = z;
}

void C3DSoundOutput::SetMyOrientationUp(float x, float y, float z)
{
	listenerOri[3] = x;
	listenerOri[4] = y;
	listenerOri[5] = z;
}

void C3DSoundOutput::UpdateMe()
{
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
}

IStreamPlayer* C3DSoundOutput::CreateStreamPlayer()
{
	CStreamPlayer* nextStreamPlayer = new CStreamPlayer();
	nextStreamPlayer->_soundOutput = this;
	nextStreamPlayer->srate = _sampleRate;
	nextStreamPlayer->_dec = dec;
	_streamPlayers.push_back(nextStreamPlayer);
	return (IStreamPlayer*)nextStreamPlayer;
}

void C3DSoundOutput::DeleteStreamPlayer(IStreamPlayer * streamPlayer)
{
	_streamPlayers.remove(streamPlayer);
	delete streamPlayer;
}
