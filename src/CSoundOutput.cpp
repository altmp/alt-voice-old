#include "CSoundOutput.h"
#include "CStreamPlayer.h"
#include "CVoiceException.h"

CSoundOutput::CSoundOutput(char* deviceName, int sampleRate, int sourcesCount) : _sampleRate(sampleRate)
{
	device = alcOpenDevice(deviceName);
	if (!device)
		throw CVoiceException(AltVoiceError::DeviceOpenError);

	ctx = alcCreateContext(device, NULL);
	if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
	{
		if (ctx != NULL)
			alcDestroyContext(ctx);
		alcCloseDevice(device);
		throw CVoiceException(AltVoiceError::ContextSetError);
	}

	ALCint maxMonoSources;
	alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &maxMonoSources);
	if (sourcesCount > maxMonoSources)
		sourcesCount = maxMonoSources;

	sources = new ALuint[sourcesCount];
	alGenSources(sourcesCount, sources);
	if (alGetError() != AL_NO_ERROR)
		throw CVoiceException(AltVoiceError::SourcesCreateError);
	_sourcesCount = sourcesCount;

	for (uint8_t i = 0; i < sourcesCount; ++i)
		freeSources.push(sources[i]);

	CStreamPlayer::soundOutput = this;
}


CSoundOutput::~CSoundOutput()
{
	for (IStreamPlayer* p : _streamPlayers)
		delete p;
	_streamPlayers.clear();

	alDeleteSources(_sourcesCount, sources);
	delete[] sources;

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	alcCloseDevice(device);
}

void CSoundOutput::SetMyPosition(float x, float y, float z)
{
	listenerPos[0] = x;
	listenerPos[1] = y;
	listenerPos[2] = z;
}

void CSoundOutput::SetMyVelocity(float x, float y, float z)
{
	listenerVel[0] = x;
	listenerVel[1] = y;
	listenerVel[2] = z;
}

void CSoundOutput::SetMyOrientationFront(float x, float y, float z)
{
	listenerOri[0] = x;
	listenerOri[1] = y;
	listenerOri[2] = z;
}

void CSoundOutput::SetMyOrientationUp(float x, float y, float z)
{
	listenerOri[3] = x;
	listenerOri[4] = y;
	listenerOri[5] = z;
}

void CSoundOutput::SetMyInnerConeAngle(float angle)
{
	innerConeAngle = angle;
}

void CSoundOutput::SetMyOutterConeAngle(float angle)
{
	outerConeAngle = angle;
}

void CSoundOutput::UpdateMe()
{
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alListenerf(AL_CONE_INNER_ANGLE, innerConeAngle);
	alListenerf(AL_CONE_OUTER_ANGLE, outerConeAngle);
}

IStreamPlayer* CSoundOutput::CreateStreamPlayer()
{
	try
	{
		CStreamPlayer* nextStreamPlayer = new CStreamPlayer();
		_streamPlayers.push_back(nextStreamPlayer);
		return (IStreamPlayer*)nextStreamPlayer;
	}
	catch (CVoiceException exception)
	{
		lastError = exception.GetCode();
		return nullptr;
	}
}

void CSoundOutput::DeleteStreamPlayer(IStreamPlayer * streamPlayer)
{
	_streamPlayers.remove(streamPlayer);
	delete streamPlayer;
}

void CSoundOutput::SetBufferingTime(unsigned int timeMS)
{
	bufferingTime = timeMS;
}

void CSoundOutput::SetExtraGain(float gain)
{
	extraGain = gain;
}

AltVoiceError CSoundOutput::ChangeDevice(const char * deviceName)
{
	for (IStreamPlayer* p : _streamPlayers)
		((CStreamPlayer*)p)->DropSource();

	while (freeSources.size())
		freeSources.pop();

	alDeleteSources(_sourcesCount, sources);
	delete[] sources;

	alcMakeContextCurrent(NULL);
	alcDestroyContext(ctx);
	alcCloseDevice(device);

	device = alcOpenDevice(deviceName);
	if (!device)
	{
		lastError = AltVoiceError::DeviceOpenError;
		return AltVoiceError::DeviceOpenError;
	}

	ctx = alcCreateContext(device, NULL);
	if (ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
	{
		if (ctx != NULL)
			alcDestroyContext(ctx);
		alcCloseDevice(device);
		lastError = AltVoiceError::ContextSetError;
		return AltVoiceError::ContextSetError;
	}

	ALCint maxMonoSources;
	alcGetIntegerv(device, ALC_MONO_SOURCES, 1, &maxMonoSources);
	if (_sourcesCount > (ALuint)maxMonoSources)
		_sourcesCount = (ALuint)maxMonoSources;

	sources = new ALuint[_sourcesCount];
	alGenSources(_sourcesCount, sources);
	if (alGetError() != AL_NO_ERROR)
	{
		lastError = AltVoiceError::SourcesCreateError;
		return AltVoiceError::SourcesCreateError;
	}

	for (uint8_t i = 0; i < _sourcesCount; ++i)
		freeSources.push(sources[i]);

	return AltVoiceError::Ok;
}

AltVoiceError CSoundOutput::GetLastError()
{
	return lastError;
}

void CSoundOutput::FreeSource(ALuint source)
{
	freeSources.push(source);
}

bool CSoundOutput::GetSource(ALuint & source)
{
	if(!freeSources.size())
		return false;

	source = freeSources.front();
	freeSources.pop();
	return true;
}
