@echo off
call vcbuild.bat Debug
if errorlevel 1 (
   echo Debug build error
   exit /b %errorlevel%
)
call vcbuild.bat Release
if errorlevel 1 (
   echo Release build error
   exit /b %errorlevel%
)

7z a .\bin\alt-voice.zip lib\*\*.dll lib\*\*.lib include\*.h