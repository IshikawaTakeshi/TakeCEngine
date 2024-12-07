#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "Quaternion.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include <string>

class WinApp;
class DirectXCommon;
class SrvManager;
class ImGuiManager {
public:

	ImGuiManager() = default;
	~ImGuiManager() = default;

	void Initialize(WinApp* winApp,DirectXCommon* dxCommon,SrvManager* srvManager);

	void Finalize();

	void Begin();

	void End();

	void Draw();

	static void QuaternionScreenPrintf(const std::string& label, const Quaternion& q);

	static void Vector3ScreenPrintf(const std::string& label, const Vector3& v);

	static void Matrix4x4ScreenPrintf(const std::string& label, const Matrix4x4& m);

private:

	
	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};

