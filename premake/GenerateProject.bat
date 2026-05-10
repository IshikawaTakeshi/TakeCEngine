@echo off
:: このバッチファイルと同じフォルダを作業ディレクトリにする
chdir /d "%~dp0"

echo ================================================
echo  VSフィルタを整理しています...
echo  (premake5 vs2022)
echo ================================================
echo.

:: 同じフォルダにある premake5.exe を直接呼ぶ
premake5.exe --file=premake.lua vs2022

if %errorlevel% neq 0 (
    echo.
    echo [エラー] プロジェクト生成に失敗しました。
    pause
    exit /b 1
)

:: ---------------------------------------------------------------
:: PlatformToolset を v143 -> v145 に修正する
::   premake5 の vs2022 アクションは v143 を出力するが、
::   このプロジェクトは v145 (Microsoft C++ Build Tools) を使用する。
::   ライブラリ名 (assimp-vc143-mtd.lib 等) は変えないよう
::   タグごと完全一致で置換する。
:: ---------------------------------------------------------------
echo [後処理] PlatformToolset を v143 -^> v145 に修正しています...
powershell -NoProfile -Command ^
    "(Get-Content '..\project\DirectXGame.vcxproj') ^
        -replace '<PlatformToolset>v143</PlatformToolset>', '<PlatformToolset>v145</PlatformToolset>' ^
    | Set-Content '..\project\DirectXGame.vcxproj'"

if %errorlevel% neq 0 (
    echo.
    echo [エラー] PlatformToolset の修正に失敗しました。
    pause
    exit /b 1
)

echo.
echo [完了] プロジェクトファイルを生成しました。
echo   出力先: ..\project\DirectXGame.sln
echo.
echo Visual Studio でソリューションを開き直すとフィルタが反映されます。
pause