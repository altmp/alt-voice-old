#pragma once
#include "IStreamPlayer.h"

class I3DSoundOutput
{
public:
	virtual ~I3DSoundOutput() = default;

	virtual void SetMyPosition(float x, float y, float z) = 0;
	virtual void SetMyVelocity(float x, float y, float z) = 0;
	virtual void SetMyOrientationFront(float x, float y, float z) = 0;
	virtual void SetMyOrientationUp(float x, float y, float z) = 0;
	virtual void UpdateMe() = 0;

	virtual IStreamPlayer* CreateStreamPlayer() = 0;
	virtual void DeleteStreamPlayer(IStreamPlayer* streamPlayer) = 0;
	virtual void SetBufferingTime(unsigned int timeMS) = 0;
};
