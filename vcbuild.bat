REM @echo off
REM echo Building opus library
REM SET WinSDKVersion=10.0.17763.0
REM msbuild ./vendor/opus/win32/VS2015/opus.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../../../libs/

REM echo Building Portaudio library
REM mkdir temp/portaudio
REM cd temp/portaudio
REM cmake -G "Visual Studio 15 Win64" ../../vendor/portaudio/
REM cd ../../
REM msbuild ./temp/portaudio/portaudio_static.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs/

REM echo Building OpenAL library
REM mkdir temp/openal
REM cd temp/openal
REM cmake -G "Visual Studio 15 Win64" -DLIBTYPE=STATIC ../../vendor/openal/
REM cd ../../
REM msbuild ./temp/openal/OpenAL.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs/

REM echo Building alt-voice library
REM mkdir temp/alt-voice
REM cd temp/alt-voice
REM cmake -G "Visual Studio 15 Win64" ../../
REM cd ../../
REM msbuild ./temp/alt-voice/alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../bin/

REM echo alt-voice built


dir
