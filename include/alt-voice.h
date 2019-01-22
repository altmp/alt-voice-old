#pragma once
#include "IStreamPlayer.h"
#include "I3DSoundOutput.h"
#include "ISoundInput.h"

#define DEFAULT_SOURCE_COUNT 32//Lloks like this number copypasted from Snail

#ifndef ALT_VOICE_API
#if defined(ALT_LIB_STATIC)
#define ALT_VOICE_API
#elif defined(_WIN32)
#if defined(ALT_VOICE_LIB)
#define ALT_VOICE_API __declspec(dllexport)
#else
#define ALT_VOICE_API __declspec(dllimport)
#endif
#else
#define AL_API extern
#endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

ALT_VOICE_API I3DSoundOutput* CreateSoundOutput(int sampleRate, int sourcesCount = DEFAULT_SOURCE_COUNT);
ALT_VOICE_API ISoundInput* CreateSoundInput(int sampleRate, int framesPerBuffer, int bitrate);
ALT_VOICE_API void DestroySoundOutput(I3DSoundOutput* output);
ALT_VOICE_API void DestroySoundInput(ISoundInput* input);

#if defined(__cplusplus)
}
#endif
