#include "C3DSoundOutput.h"
#include "CStreamPlayer.h"

C3DSoundOutput::C3DSoundOutput(int sampleRate, int sourcesCount) : _sampleRate(sampleRate)
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

	/*name = NULL;
	if (alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
		name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
	if (!name || alcGetError(device) != AL_NO_ERROR)
		name = alcGetString(device, ALC_DEVICE_SPECIFIER);*/

	ALCint maxMonoSources;
	alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &maxMonoSources);
	if (sourcesCount > maxMonoSources)
		sourcesCount = maxMonoSources;

	sources = new ALuint[sourcesCount];
	alGenSources(sourcesCount, sources);
	if (alGetError() != AL_NO_ERROR)
		throw std::runtime_error("Could not create source");
	_sourcesCount = sourcesCount;

	for (uint8_t i = 0; i < sourcesCount; ++i)
		freeSources.push(sources[i]);

	CStreamPlayer::soundOutput = this;
}


C3DSoundOutput::~C3DSoundOutput()
{
	for (IStreamPlayer* p : _streamPlayers)
		delete p;
	_streamPlayers.clear();

	alDeleteSources(_sourcesCount, sources);
	delete[_sourcesCount] sources;

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
	_streamPlayers.push_back(nextStreamPlayer);
	return (IStreamPlayer*)nextStreamPlayer;
}

void C3DSoundOutput::DeleteStreamPlayer(IStreamPlayer * streamPlayer)
{
	_streamPlayers.remove(streamPlayer);
	delete streamPlayer;
}

void C3DSoundOutput::SetBufferingTime(unsigned int timeMS)
{
	bufferingTime = timeMS;
}

void C3DSoundOutput::FreeSource(ALuint source)
{
	std::cout << "Free source [ID: " << source << "]" << std::endl;
	freeSources.push(source);
}

bool C3DSoundOutput::GetSource(ALuint & source)
{
	if(!freeSources.size())
		return false;

	source = freeSources.front();
	std::cout << "Requesting source [ID: " << source << "]" << std::endl;
	freeSources.pop();
	return true;
}
