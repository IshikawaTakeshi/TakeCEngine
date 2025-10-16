@echo off
setlocal enabledelayedexpansion

REM 生成先を build/ に固定
if not exist build mkdir build

REM Visual Studio 2022 のソリューション/プロジェクト生成
echo Generating Visual Studio 2022 solution...
premake5 vs2022

echo.
echo Generated solution at: build\TakeCEngine.sln
pause