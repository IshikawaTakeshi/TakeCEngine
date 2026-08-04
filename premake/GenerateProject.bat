@echo off
setlocal
pushd "%~dp0"

set "TOOLSET=%~1"
if not defined TOOLSET set "TOOLSET=v145"

echo Generating TakeCEngine project (PlatformToolset=%TOOLSET%)...
"%~dp0premake5.exe" --file="%~dp0premake.lua" vs2026
if errorlevel 1 goto :error

if /I "%TOOLSET%"=="v145" goto :success

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$path = '%~dp0..\project\TakeCEngine.vcxproj'; $content = [IO.File]::ReadAllText($path); $content = $content.Replace('<PlatformToolset>v145</PlatformToolset>', '<PlatformToolset>%TOOLSET%</PlatformToolset>'); [IO.File]::WriteAllText($path, $content, [Text.UTF8Encoding]::new($false))"
if errorlevel 1 goto :error

:success
echo Generated: %~dp0..\project\TakeCEngine.sln
popd
exit /b 0

:error
echo Project generation failed.
popd
exit /b 1
