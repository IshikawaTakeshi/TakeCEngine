@echo off
setlocal
pushd "%~dp0"

set "TOOLSET=%~1"
if not defined TOOLSET set "TOOLSET=v143"

echo Generating Visual Studio 2022 projects (PlatformToolset=%TOOLSET%)...
"%~dp0premake5.exe" --file="%~dp0premake.lua" vs2022
if errorlevel 1 goto :error

if /I "%TOOLSET%"=="v143" goto :success

for %%F in ("%~dp0..\project\TakeCEngine.vcxproj" "%~dp0..\project\DirectXGame.vcxproj") do (
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
        "$path = '%%~fF'; $content = [IO.File]::ReadAllText($path); $content = $content.Replace('<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>%TOOLSET%</PlatformToolset>'); [IO.File]::WriteAllText($path, $content, [Text.UTF8Encoding]::new($false))"
    if errorlevel 1 goto :error
)

:success
echo Generated: %~dp0..\project\DirectXGame.sln
popd
exit /b 0

:error
echo Project generation failed.
popd
exit /b 1
