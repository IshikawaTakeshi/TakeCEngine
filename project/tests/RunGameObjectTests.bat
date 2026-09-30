@echo off
setlocal

set "REPOSITORY_ROOT=%~dp0..\.."
set "TEST_BUILD_DIRECTORY=%REPOSITORY_ROOT%\Generated\obj\Tests\GameObjectTests"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo Visual Studio Installer's vswhere.exe was not found.
    exit /b 1
)

for /f "usebackq tokens=*" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VISUAL_STUDIO_DIRECTORY=%%I"
)

if not defined VISUAL_STUDIO_DIRECTORY (
    echo A Visual Studio C++ toolchain was not found.
    exit /b 1
)

call "%VISUAL_STUDIO_DIRECTORY%\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1

if not exist "%TEST_BUILD_DIRECTORY%" mkdir "%TEST_BUILD_DIRECTORY%"

cl.exe /nologo /std:c++20 /EHsc /W4 /WX /utf-8 /MTd ^
    /I "%REPOSITORY_ROOT%\project" ^
    /I "%REPOSITORY_ROOT%\project\engine" ^
    /external:W0 ^
    /external:I "%REPOSITORY_ROOT%\project\externals" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\Component.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\GameObject.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\GameObjectManager.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\TransformComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\MathEnv.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Matrix4x4.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\MatrixMath.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Quaternion.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Vector3.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Vector3Math.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Utility\Logger.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Utility\StringUtility.cpp" ^
    "%REPOSITORY_ROOT%\project\tests\GameObjectTests.cpp" ^
    /Fo:"%TEST_BUILD_DIRECTORY%\\" ^
    /Fe:"%TEST_BUILD_DIRECTORY%\GameObjectTests.exe"
if errorlevel 1 exit /b 1

"%TEST_BUILD_DIRECTORY%\GameObjectTests.exe"
if errorlevel 1 exit /b 1

echo GameObject tests passed.
exit /b 0
