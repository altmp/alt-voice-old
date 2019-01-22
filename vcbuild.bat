@echo off
echo Building alt-voice library
if not exist "./temp/alt-voice" mkdir "./temp/alt-voice"
cd ./temp/alt-voice
cmake -DCMAKE_BUILD_TYPE=%1 -G "Visual Studio 15 Win64" ../../
cd ../../
msbuild ./temp/alt-voice/alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=%1 /p:Platform=x64
if errorlevel 1 (
   echo alt-voice build error
   exit /b %errorlevel%
)

if not exist "./bin/%1" mkdir "./bin/%1"
if not exist "./lib/%1" mkdir "./lib/%1"
copy /y .\temp\alt-voice\%1\alt-voice.lib .\lib\%1\alt-voice.lib
copy /y .\bin\%1\alt-voice.dll .\lib\%1\alt-voice.dll

msbuild ./temp/alt-voice/3d-two-sources.vcxproj /p:PlatformToolset=v141 /p:Configuration=%1 /p:Platform=x64
if errorlevel 1 (
   echo 3d-two-sources build error
   exit /b %errorlevel%
)
copy /y .\temp\alt-voice\%1\3d-two-sources.exe .\bin\%1\3d-two-sources.exe

echo alt-voice built
