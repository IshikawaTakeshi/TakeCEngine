@echo off
echo プロジェクトファイルを生成しています...
premake5 vs2022
if %errorlevel% neq 0 pause