@echo off
echo Building opus library
if not exist "./temp/opus" mkdir "./temp/opus"
cd ./temp/opus
cmake -G "Visual Studio 15 Win64" ../../vendor/opus/
cd ../../
call ".\vendor\opus\win32\genversion.bat" ".\vendor\opus\win32\version.h" PACKAGE_VERSION
msbuild ./temp/opus/opus.vcxproj /p:PlatformToolset=v141 /p:Configuration=%1 /p:Platform=x64 /p:OutDir=../../libs/%1/
if errorlevel 1 (
   echo Opus build error
   exit /b %errorlevel%
)

echo Building OpenAL library
if not exist "./temp/openal" mkdir "./temp/openal"
cd ./temp/openal
cmake -G "Visual Studio 15 Win64" -DLIBTYPE=STATIC ../../vendor/openal/
cd ../../
msbuild ./temp/openal/OpenAL.vcxproj /p:PlatformToolset=v141 /p:Configuration=%1 /p:Platform=x64 /p:OutDir=../../libs/%1/
if errorlevel 1 (
   echo OpenAL build error
   exit /b %errorlevel%
)

echo Building alt-voice library
if not exist "./temp/alt-voice" mkdir "./temp/alt-voice"
cd ./temp/alt-voice
cmake -DCMAKE_BUILD_TYPE=%1 -G "Visual Studio 15 Win64" ../../
cd ../../
msbuild ./temp/alt-voice/alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=%1 /p:Platform=x64 /p:OutDir=../../bin/%1/
if errorlevel 1 (
   echo alt-voice build error
   exit /b %errorlevel%
)

if not exist "./bin/%1" mkdir "./bin/%1"
copy /y .\temp\alt-voice\%1\alt-voice.lib .\bin\%1\alt-voice.lib

echo alt-voice built
