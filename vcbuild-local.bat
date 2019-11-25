@echo off
echo Building alt-voice library
if not exist "./temp/alt-voice" mkdir "./temp/alt-voice"
cd ./temp/alt-voice
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=TRUE -G"Visual Studio 16" -A x64 ../../
cd ../../

REM msbuild ./temp/alt-voice/3d-two-sources.vcxproj /p:PlatformToolset=v141 /p:Configuration=Debug /p:Platform=x64
REM if errorlevel 1 (
REM    echo 3d-two-sources build error
REM    exit /b %errorlevel%
REM )
REM copy /y .\temp\alt-voice\Debug\3d-two-sources.exe .\bin\Debug\3d-two-sources.exe

echo alt-voice built
