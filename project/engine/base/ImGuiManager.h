#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

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

private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;
};

