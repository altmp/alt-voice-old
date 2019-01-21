#pragma once
#include <list>
#include <opus.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include "I3DSoundOutput.h"


class CStreamPlayer;

class C3DSoundOutput: public I3DSoundOutput
{
	friend CStreamPlayer;
	OpusDecoder* dec = nullptr;
	std::list<IStreamPlayer *> _streamPlayers;

	uint32_t _sampleRate;

	ALfloat listenerPos[3] = { 0.f, 0.f, 0.f };
	ALfloat listenerVel[3] = { 0.f, 0.f, 0.f };
	ALfloat listenerOri[6] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

	ALuint *sources = nullptr;
	ALuint _sourcesCount;
	std::queue<ALuint> freeSources;
public:
	C3DSoundOutput(int sampleRate, int sourcesCount);
	~C3DSoundOutput();

	void SetMyPosition(float x, float y, float z) override;
	void SetMyVelocity(float x, float y, float z) override;
	void SetMyOrientationFront(float x, float y, float z) override;
	void SetMyOrientationUp(float x, float y, float z) override;
	void UpdateMe() override;

	IStreamPlayer* CreateStreamPlayer() override;
	void DeleteStreamPlayer(IStreamPlayer* streamPlayer) override;

private:
	void FreeSource(ALuint source);
	bool GetSource(ALuint& source);
};

