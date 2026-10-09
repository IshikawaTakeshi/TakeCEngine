@echo off
chcp 65001 >nul
setlocal

set "REPOSITORY_ROOT=%~dp0..\.."
set "TEST_BUILD_DIRECTORY=%REPOSITORY_ROOT%\Generated\obj\Tests\BehaviorTreeComponentTests"
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

cl.exe /nologo /std:c++20 /EHsc /W4 /WX /utf-8 /MT /Gy /O2 /FIWindows.h /DNOMINMAX ^
    /I "%REPOSITORY_ROOT%\project" ^
    /I "%REPOSITORY_ROOT%\project\engine" ^
    /external:W0 ^
    /external:I "%REPOSITORY_ROOT%\project\externals" ^
    /external:I "%REPOSITORY_ROOT%\project\externals\imgui" ^
    /external:I "%REPOSITORY_ROOT%\project\externals\assimp\include" ^
    /external:I "%REPOSITORY_ROOT%\project\packages\Microsoft.AI.DirectML.1.15.4\include" ^
    /external:I "%REPOSITORY_ROOT%\project\packages\Microsoft.Direct3D.DXC.1.9.2602.24\build\native\include" ^
    /external:I "%REPOSITORY_ROOT%\project\packages\Microsoft.ML.OnnxRuntime.DirectML.1.24.4\build\native\include" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\AIControllerComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\AICommandHandlerComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\AIMovementComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\AI\AICommandBuffer.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\BehaviorTreeComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\BehaviorTreeBrain.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\ActionNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\Blackboard.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\SequenceNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\SelectorNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\PlannerSelectorNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\WeightSelectorNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\WaitNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\WaitBlackboardTimeNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\BehaviorNodeRegistry.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\BehaviorTreeAssetValidator.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\BehaviorTreeUtil.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\ComboFactory.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\ConditionNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\ScoreConditionNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\SetBlackboardBoolNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\BehaviorTree\SetBlackboardStringNode.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\Component.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\GameObject.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\GameObjectManager.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\GameObject\TransformComponent.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\MathEnv.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Easing.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Matrix4x4.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\MatrixMath.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Quaternion.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Vector3.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Math\Vector3Math.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Utility\Logger.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Utility\ResourcePath.cpp" ^
    "%REPOSITORY_ROOT%\project\engine\Utility\StringUtility.cpp" ^
    "%REPOSITORY_ROOT%\project\tests\BehaviorTreeComponentTests.cpp" ^
    "%REPOSITORY_ROOT%\project\tests\AICommandBufferTests.cpp" ^
    /Fo:"%TEST_BUILD_DIRECTORY%\\" ^
    /Fe:"%TEST_BUILD_DIRECTORY%\BehaviorTreeComponentTests.exe" /link /OPT:REF user32.lib
if errorlevel 1 exit /b 1

"%TEST_BUILD_DIRECTORY%\BehaviorTreeComponentTests.exe"
if errorlevel 1 exit /b 1

echo BehaviorTreeComponent tests passed.
exit /b 0
