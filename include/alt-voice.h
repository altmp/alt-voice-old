#pragma once
#include "IStreamPlayer.h"
#include "ISoundOutput.h"
#include "ISoundInput.h"
#include "IOpusEncoder.h"
#include "IOpusDecoder.h"
#include "VoiceError.h"


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
        #define ALT_VOICE_API extern
    #endif
#endif

#if defined(__cplusplus)
extern "C" {
#endif

ALT_VOICE_API char* GetInputDevicesEnum();
ALT_VOICE_API char* GetOutputDevicesEnum();
ALT_VOICE_API char* GetNextDevice(char** enumerator);
ALT_VOICE_API AltVoiceError CreateSoundOutput(char* deviceName, int sampleRate, int sourcesCount, ISoundOutput** soundOutput);
ALT_VOICE_API AltVoiceError CreateSoundInput(char* deviceName, int sampleRate, int framesPerBuffer, int bitrate, ISoundInput** soundInput);
ALT_VOICE_API void DestroySoundOutput(ISoundOutput* output);
ALT_VOICE_API void DestroySoundInput(ISoundInput* input);
ALT_VOICE_API const char* GetVoiceErrorText(AltVoiceError error);
ALT_VOICE_API AltVoiceError CreateOpusEncoder(int sampleRate, int channelsCount, IOpusEncoder** opusEncoder);
ALT_VOICE_API AltVoiceError CreateOpusDecoder(int sampleRate, int channelsCount, IOpusDecoder** opusDecoder);
ALT_VOICE_API void DestroyOpusEncoder(IOpusEncoder* opusEncoder);
ALT_VOICE_API void DestroyOpusDecoder(IOpusDecoder* opusDecoder);

#if defined(__cplusplus)
}
#endif
