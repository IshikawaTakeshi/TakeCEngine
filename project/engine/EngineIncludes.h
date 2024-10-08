#pragma once
#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "D3DResourceLeakChecker.h"
#include "WinApp.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "TextureManager.h"

#include "Audio.h"
#include "Transform.h"
#include "MatrixMath.h"
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Object3dCommon.h"
#include "Object3d.h"
#include "ModelManager.h"
#include "Triangle.h"
#include "Sphere.h"
#include "Model.h"
#include "Camera.h"
#include "CameraManager.h"
#include "SrvManager.h"
#include "TakeCFrameWork.h"

#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#pragma region imgui
#ifdef _DEBUG
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // DEBUG
#pragma endregion