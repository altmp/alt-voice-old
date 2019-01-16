Write-Debug "Building opus library"
$env:WinSDKVersion = 10.0.17763.0
msbuild ./vendor/opus/win32/VS2015/opus.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=..\..\..\..\libs\

Write-Debug "Building Portaudio library"
New-Item -ItemType directory -Path ".\temp\portaudio"
Set-Location -Path ".\temp\portaudio"
cmake -G "Visual Studio 15 Win64" ..\..\vendor\portaudio\
Set-Location -Path "..\..\"
msbuild .\temp\portaudio\portaudio_static.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=..\..\libs\

Write-Debug "Building OpenAL library"
New-Item -ItemType directory -Path ".\temp\openal"
Set-Location -Path ".\temp\openal"
cmake -G "Visual Studio 15 Win64" -DLIBTYPE=STATIC ..\..\vendor\openal\
Set-Location -Path "..\..\"
msbuild .\temp\openal\OpenAL.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=..\..\libs\

Write-Debug "Building alt-voice library"
New-Item -ItemType directory -Path ".\temp\alt-voice"
Set-Location -Path ".\temp\alt-voice"
cmake -G "Visual Studio 15 Win64" ..\..\
Set-Location -Path "..\..\"
msbuild .\temp\alt-voice\alt-voice.vcxproj /p:PlatformToolset=v141 /p:Configuration=Release /p:Platform=x64 /p:OutDir=..\..\bin\

Write-Debug "alt-voice built"
