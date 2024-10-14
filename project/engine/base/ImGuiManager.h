#pragma once
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

