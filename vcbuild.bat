@echo off
echo Building opus library
SET WinSDKVersion=10.0.17763.0
msbuild ./vendor/opus/win32/VS2015/opus.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../../../libs

echo Building Portaudio library
mkdir temp/portaudio
cd temp/portaudio
cmake -G "Visual Studio 15 Win64" ../../vendor/portaudio/
cd ../../
msbuild ./temp/portaudio/portaudio_static.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs

echo Building OpenAL library
mkdir temp/openal
cd temp/openal
cmake -G "Visual Studio 15 Win64" -DLIBTYPE=STATIC ../../vendor/openal/
cd ../../
msbuild ./temp/openal/OpenAL.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs

echo Building alt-voice library
mkdir temp/alt-voice
cd temp/alt-voice
cmake -G "Visual Studio 15 Win64" ../../
cd ../../
msbuild ./temp/alt-voice/alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../bin

echo alt-voice built
