@echo off
echo Building opus library
if not exist "./temp/opus" mkdir "./temp/opus"
cd ./temp/opus
cmake -G "Visual Studio 15 Win64" ../../vendor/opus/
cd ../../
msbuild ./temp/opus/opus.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs/

echo Building OpenAL library
if not exist "./temp/openal" mkdir "./temp/openal"
cd ./temp/openal
cmake -G "Visual Studio 15 Win64" -DLIBTYPE=STATIC ../../vendor/openal/
cd ../../
msbuild ./temp/openal/OpenAL.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../libs/

echo Building alt-voice library
if not exist "./temp/alt-voice" mkdir "./temp/alt-voice"
cd ./temp/alt-voice
cmake -G "Visual Studio 15 Win64" ../../
cd ../../
msbuild ./temp/alt-voice/alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=../../bin/
copy /y .\temp\alt-voice\Release\alt-voice.lib .\bin\alt-voice.lib

echo "alt-voice built"
