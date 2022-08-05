#pragma once
#include <cmath>
#include <cstdint>

namespace helpers
{
	using Sample = int16_t;

	static constexpr int FRAME_SIZE = 960;
	static constexpr int  MAX_PACKET_SIZE = 32768;

	static float GetSignalMultiplierForVolume(float volume)
	{	//https://github.com/almoghamdani/audify/blob/master/src/rt_audio.cpp#L422
		// Explained here: https://stackoverflow.com/a/1165188
		return (std::pow<float>(10, volume) - 1) / (10 - 1);
	}

	static void GainPCM(Sample* data, size_t framesCount, float gain)
	{
		for (int i = 0; i < framesCount; ++i)
			data[i] *= gain;
	}
}