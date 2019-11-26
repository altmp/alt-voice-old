//#pragma once
//#include <list>
//#include <opus.h>
//#include <AL/al.h>
//#include <AL/alc.h>
//#include <AL/alext.h>
//
//#include "ISoundOutput.h"
//#include "VoiceError.h"
//
//class CStreamPlayer;
//
//class CSoundOutput: public ISoundOutput
//{
//	friend CStreamPlayer;
//	std::list<IStreamPlayer *> _streamPlayers;
//
//	uint32_t _sampleRate;
//
//	ALfloat listenerPos[3] = { 0.f, 0.f, 0.f };
//	ALfloat listenerVel[3] = { 0.f, 0.f, 0.f };
//	ALfloat listenerOri[6] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
//	ALfloat extraGain = 1.f;
//	ALfloat innerConeAngle = 90.f;
//	ALfloat outerConeAngle = 180.f;
//
//	ALuint *sources = nullptr;
//	ALuint _sourcesCount;
//	std::queue<ALuint> freeSources;
//	uint32_t bufferingTime = 0;
//
//	ALCdevice *device;
//	ALCcontext *ctx;
//	AltVoiceError lastError = AltVoiceError::Ok;
//public:
//	CSoundOutput(char* deviceName, int sampleRate, int sourcesCount);
//	~CSoundOutput();
//
//	void SetMyPosition(float x, float y, float z) override;
//	void SetMyVelocity(float x, float y, float z) override;
//	void SetMyOrientationFront(float x, float y, float z) override;
//	void SetMyOrientationUp(float x, float y, float z) override;
//	void UpdateMe() override;
//
//	IStreamPlayer* CreateStreamPlayer() override;
//	void DeleteStreamPlayer(IStreamPlayer* streamPlayer) override;
//	void SetBufferingTime(unsigned int timeMS) override;
//	void SetExtraGain(float gain) override;
//
//	AltVoiceError ChangeDevice(const char* deviceName) override;
//	AltVoiceError GetLastError() override;
//private:
//	void FreeSource(ALuint source);
//	bool GetSource(ALuint& source);
//};
//
